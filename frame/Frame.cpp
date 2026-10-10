// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "Frame.h"
#include "ShaderPrograms.h"
#include "geometry/Geometry.h"
#include "material/Material.h"
#include "spatial_field/SpatialField.h"
#include "volume/Volume.h"
#include "vtkSlangWebGPU.h"
// std
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <optional>

// Context struct for buffer map callback
struct MapContext {
  bool done{false};
  WGPUMapAsyncStatus status;
};
// Free function for buffer map callback
namespace {
void bufferMapCallback(WGPUMapAsyncStatus status, WGPUStringView message,
    void *userdata, void *userdata2) {
  auto *ctx = static_cast<MapContext *>(userdata);
  ctx->status = status;
  ctx->done = true;
}

template <typename Handle, typename ReleaseFn>
void rel(Handle &h, ReleaseFn fn)
{
  if (h) {
    fn(h);
    h = nullptr;
  }
}
} // namespace

namespace anari_webgpu {

Frame::Frame(WebGPUDeviceGlobalState *s) : helium::BaseFrame(s) {}

Frame::~Frame()
{
  wait();
  cleanupComputeResources();
}

bool Frame::isValid() const { return m_valid; }

WebGPUDeviceGlobalState *Frame::deviceState() const
{
  return (WebGPUDeviceGlobalState *)helium::BaseObject::m_state;
}

void Frame::commitParameters()
{
  m_renderer = getParamObject<Renderer>("renderer");
  m_camera = getParamObject<Camera>("camera");
  m_world = getParamObject<World>("world");
  m_colorType = getParam<anari::DataType>("channel.color", ANARI_UNKNOWN);
  m_depthType = getParam<anari::DataType>("channel.depth", ANARI_UNKNOWN);
  m_size = getParam<uint2>("size", uint2(0u, 0u));
}

void Frame::finalize()
{
  if (!m_renderer)
    reportMessage(ANARI_SEVERITY_WARNING, "missing required parameter 'renderer' on frame");
  if (!m_camera)
    reportMessage(ANARI_SEVERITY_WARNING, "missing required parameter 'camera' on frame");
  if (!m_world)
    reportMessage(ANARI_SEVERITY_WARNING, "missing required parameter 'world' on frame");

  const auto numPixels = m_size.x * m_size.y;
  m_perPixelBytes = 4 * (m_colorType == ANARI_FLOAT32_VEC4 ? 4 : 1);
  m_pixelBuffer.resize(numPixels * m_perPixelBytes);
  m_depthBuffer.resize(m_depthType == ANARI_FLOAT32 ? numPixels : 0);
  m_zBuffer.resize(numPixels);

  m_valid = m_renderer && m_camera && m_camera->isValid() && m_world && m_world->isValid();

  cleanupComputeResources();

  rel(m_volumePipeline, wgpuComputePipelineRelease);
  rel(m_volumeShader,   wgpuShaderModuleRelease);
  rel(m_volumeBGL,      wgpuBindGroupLayoutRelease);

  m_gpuResourcesInitialized = false;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

static WGPUBindGroupLayout makeBGL(WGPUDevice device,
    const WGPUBindGroupLayoutEntry *entries, uint32_t count)
{
  WGPUBindGroupLayoutDescriptor desc{};
  desc.nextInChain = nullptr;
  desc.entryCount = count;
  desc.entries = entries;
  return wgpuDeviceCreateBindGroupLayout(device, &desc);
}

static WGPUBuffer uploadUniform(WGPUDevice device, WGPUQueue queue,
    const void *data, uint64_t size)
{
  WGPUBufferDescriptor desc{};
  desc.nextInChain = nullptr;
  desc.size = size;
  desc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
  desc.mappedAtCreation = false;
  WGPUBuffer buf = wgpuDeviceCreateBuffer(device, &desc);
  wgpuQueueWriteBuffer(queue, buf, 0, data, size);
  return buf;
}

static WGPUBindGroup makeBindGroup1(
    WGPUDevice device, WGPUBindGroupLayout bgl, WGPUBuffer buffer, uint64_t size)
{
  WGPUBindGroupEntry e{};
  e.binding = 0;
  e.buffer = buffer;
  e.offset = 0;
  e.size = size;
  WGPUBindGroupDescriptor bgDesc{};
  bgDesc.nextInChain = nullptr;
  bgDesc.layout = bgl;
  bgDesc.entryCount = 1;
  bgDesc.entries = &e;
  return wgpuDeviceCreateBindGroup(device, &bgDesc);
}

static WGPUComputePipeline makePipeline(WGPUDevice device,
    WGPUShaderModule shader, const WGPUBindGroupLayout *bgls, uint32_t bglCount,
    const char *label, const char *entryPoint)
{
  WGPUPipelineLayoutDescriptor plDesc{};
  plDesc.nextInChain = nullptr;
  plDesc.bindGroupLayoutCount = bglCount;
  plDesc.bindGroupLayouts = bgls;
  WGPUPipelineLayout pl = wgpuDeviceCreatePipelineLayout(device, &plDesc);

  WGPUComputePipelineDescriptor cpDesc{};
  cpDesc.nextInChain = nullptr;
  cpDesc.label = WGPUStringView{label, strlen(label)};
  cpDesc.layout = pl;
  cpDesc.compute.module = shader;
  cpDesc.compute.entryPoint = WGPUStringView{entryPoint, strlen(entryPoint)};
  cpDesc.compute.constantCount = 0;
  WGPUComputePipeline pipeline = wgpuDeviceCreateComputePipeline(device, &cpDesc);
  wgpuPipelineLayoutRelease(pl);
  return pipeline;
}

static WGPUComputePipeline makePipeline(WGPUDevice device,
    WGPUShaderModule shader, WGPUBindGroupLayout bgl, const char *label,
    const char *entryPoint)
{
  return makePipeline(device, shader, &bgl, 1, label, entryPoint);
}


static WGPUBindGroupLayoutEntry storageEntry(uint32_t binding, bool readOnly)
{
  WGPUBindGroupLayoutEntry e{};
  e.binding = binding;
  e.visibility = WGPUShaderStage_Compute;
  e.buffer.type = readOnly
      ? WGPUBufferBindingType_ReadOnlyStorage
      : WGPUBufferBindingType_Storage;
  e.buffer.minBindingSize = 0;
  return e;
}

static WGPUBindGroupLayoutEntry uniformEntry(uint32_t binding)
{
  WGPUBindGroupLayoutEntry e{};
  e.binding = binding;
  e.visibility = WGPUShaderStage_Compute;
  e.buffer.type = WGPUBufferBindingType_Uniform;
  e.buffer.minBindingSize = 0;
  return e;
}

static std::optional<vtkslang::HostValue> hostValue(const helium::AnariAny &value)
{
  using vtkslang::ScalarType;
  struct Shape
  {
    ANARIDataType type;
    ScalarType scalar;
    uint32_t width;
    uint32_t runs;
  };
  static const Shape shapes[] = {
      {ANARI_FLOAT32, ScalarType::Float32, 1, 1},
      {ANARI_FLOAT32_VEC2, ScalarType::Float32, 2, 1},
      {ANARI_FLOAT32_VEC3, ScalarType::Float32, 3, 1},
      {ANARI_FLOAT32_VEC4, ScalarType::Float32, 4, 1},
      {ANARI_INT32, ScalarType::Int32, 1, 1},
      {ANARI_INT32_VEC2, ScalarType::Int32, 2, 1},
      {ANARI_INT32_VEC3, ScalarType::Int32, 3, 1},
      {ANARI_INT32_VEC4, ScalarType::Int32, 4, 1},
      {ANARI_UINT32, ScalarType::UInt32, 1, 1},
      {ANARI_UINT32_VEC2, ScalarType::UInt32, 2, 1},
      {ANARI_UINT32_VEC3, ScalarType::UInt32, 3, 1},
      {ANARI_UINT32_VEC4, ScalarType::UInt32, 4, 1},
      {ANARI_FLOAT32_MAT2, ScalarType::Float32, 2, 2},
      {ANARI_FLOAT32_MAT3, ScalarType::Float32, 3, 3},
      {ANARI_FLOAT32_MAT4, ScalarType::Float32, 4, 4},
  };
  for (const Shape &s : shapes)
    if (s.type == value.type())
      return vtkslang::HostValue{s.scalar, s.width, s.runs, value.data()};
  return std::nullopt;
}

void Frame::initComputeResources()
{
  auto *state = deviceState();
  if (!state->wgpuDevice || m_size.x == 0 || m_size.y == 0)
    return;

  WGPUDevice device = state->wgpuDevice;
  uint32_t numPixels = m_size.x * m_size.y;

  {
    WGPUBufferDescriptor desc{};
    desc.nextInChain = nullptr;
    desc.label = WGPUStringView{"anari_fb_color", 14};
    desc.usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopySrc;
    desc.size = numPixels * sizeof(float) * 4;
    desc.mappedAtCreation = false;
    m_colorStorageBuf = wgpuDeviceCreateBuffer(device, &desc);
  }
  {
    WGPUBufferDescriptor desc{};
    desc.nextInChain = nullptr;
    desc.label = WGPUStringView{"anari_fb_depth", 14};
    desc.usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopySrc;
    desc.size = numPixels * sizeof(uint32_t);
    desc.mappedAtCreation = false;
    m_depthStorageBuf = wgpuDeviceCreateBuffer(device, &desc);
  }
  {
    WGPUBufferDescriptor desc{};
    desc.nextInChain = nullptr;
    desc.label = WGPUStringView{"anari_fb_readback", 17};
    desc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead;
    desc.size = numPixels * sizeof(float) * 4;
    desc.mappedAtCreation = false;
    m_readbackBuffer = wgpuDeviceCreateBuffer(device, &desc);
  }

  {
    m_clearShader = createBuiltinShader(device, "clear");
    if (!m_clearShader) {
      reportMessage(ANARI_SEVERITY_ERROR, "Failed to compile clear shader");
      m_gpuResourcesInitialized = false;
      return;
    }

    WGPUBindGroupLayoutEntry entries[3] = {
        storageEntry(0, false), storageEntry(1, false), uniformEntry(2)};
    m_clearBGL = makeBGL(device, entries, 3);
    m_clearPipeline = makePipeline(device, m_clearShader, m_clearBGL,
        "anari_clear_pipeline", "main");
  }

  {
    m_rasterShader = createBuiltinShader(device, "raster");
    if (!m_rasterShader) {
      reportMessage(ANARI_SEVERITY_ERROR, "Failed to compile raster shader");
      m_gpuResourcesInitialized = false;
      return;
    }

    WGPUBindGroupLayoutEntry entries[7] = {
        storageEntry(0, false), storageEntry(1, false), uniformEntry(2),
        storageEntry(3, true), storageEntry(4, true),
        storageEntry(5, true), storageEntry(6, true)};
    m_rasterBGL = makeBGL(device, entries, 7);
    m_rasterPipeline = makePipeline(device, m_rasterShader, m_rasterBGL,
        "anari_raster_pipeline", "main");
    m_rasterDepthPipeline = makePipeline(device, m_rasterShader, m_rasterBGL,
        "anari_raster_depth_pipeline", "depth_main");
  }

  {
    m_pbrShader = createBuiltinShader(device, "pbr");
    if (!m_pbrShader) {
      reportMessage(ANARI_SEVERITY_ERROR, "Failed to compile PBR shader");
      m_gpuResourcesInitialized = false;
      return;
    }

    WGPUBindGroupLayoutEntry entries[7] = {
        storageEntry(0, false), storageEntry(1, false), uniformEntry(2),
        storageEntry(3, true), storageEntry(4, true),
        storageEntry(5, true), storageEntry(6, true)};
    m_pbrBGL = makeBGL(device, entries, 7);
    m_pbrPipeline = makePipeline(device, m_pbrShader, m_pbrBGL,
        "anari_pbr_pipeline", "pbr_main");
    m_pbrDepthPipeline = makePipeline(device, m_pbrShader, m_pbrBGL,
        "anari_pbr_depth_pipeline", "pbr_depth_main");
  }

  {
    m_volumeShader = createBuiltinShader(device, "volume");
    if (!m_volumeShader) {
      reportMessage(ANARI_SEVERITY_ERROR, "Failed to compile volume shader");
      m_gpuResourcesInitialized = false;
      return;
    }

    WGPUBindGroupLayoutEntry volEntries[5] = {
        storageEntry(0, false), storageEntry(1, true), uniformEntry(2),
        storageEntry(3, true), storageEntry(4, true)};
    m_volumeBGL = makeBGL(device, volEntries, 5);
    m_volumePipeline = makePipeline(device, m_volumeShader, m_volumeBGL,
        "anari_volume_pipeline", "vol_main");
  }

  m_gpuResourcesInitialized = true;
}

WGPUShaderModule Frame::createBuiltinShader(WGPUDevice device, const char *name)
{
  vtkslang::Artifact artifact =
      m_slangCompiler.compileProgram(shaderProgram(name));
  if (!artifact) {
    reportMessage(ANARI_SEVERITY_ERROR,
        "vtkSlang failed to compile '%s':\n%s",
        name,
        artifact.diagnostics.c_str());
    return nullptr;
  }
  std::string label = std::string("anari_") + name;
  return vtkslang::createShaderModule(device, artifact, label.c_str());
}

void Frame::reportOnce(ANARIStatusSeverity severity, const std::string &message)
{
  if (m_reportedMessages.insert(message).second)
    reportMessage(severity, "%s", message.c_str());
}

const Frame::SlangPipelines *Frame::slangPipelines(WGPUDevice device,
    const char *shader,
    WGPUBindGroupLayout group0,
    const char *colorEntryPoint,
    const char *depthEntryPoint,
    const SlangOverride &o)
{
  std::string key = std::string(shader) + '\0' + o.source + '\0' + o.materialType
      + '\0' + o.lightingType + '\0' + o.normalType + '\0' + o.outputType + '\0'
      + o.vertexType + '\0' + o.hooksType;
  auto it = m_slangPipelines.find(key);
  if (it != m_slangPipelines.end())
    return it->second.valid ? &it->second : nullptr;
  SlangPipelines &p = m_slangPipelines[key];

  vtkslang::CompileOptions options;
  if (!o.source.empty())
    options.modules.push_back(o.source);
  if (!o.materialType.empty())
    options.specializations.emplace_back("IMaterial", o.materialType);
  if (!o.lightingType.empty())
    options.specializations.emplace_back("ILightingModel", o.lightingType);
  if (!o.normalType.empty())
    options.specializations.emplace_back("INormalModifier", o.normalType);
  if (!o.outputType.empty())
    options.specializations.emplace_back("ISurfaceOutput", o.outputType);
  if (!o.vertexType.empty())
    options.specializations.emplace_back("IVertexModifier", o.vertexType);
  if (!o.hooksType.empty())
    options.specializations.emplace_back("IShaderHooks", o.hooksType);
  vtkslang::Artifact artifact =
      m_slangCompiler.compileProgram(shaderProgram(shader), options);
  if (!artifact) {
    reportOnce(ANARI_SEVERITY_ERROR,
        std::string("WEBGPU_MATERIAL_SLANG: '") + shader
            + "' failed to compile with the material's overrides; using its "
              "default shading:\n"
            + artifact.diagnostics);
    return nullptr;
  }
  p.plan = vtkslang::planBindings(
      artifact, [](const vtkslang::ReflectedBinding &b) { return b.group == 0; });
  std::vector<std::string> refused;
  for (const auto &block : p.plan.blocks)
    if (block.binding != 0)
      refused.push_back(block.name);
  for (const auto &resource : p.plan.resources)
    refused.push_back(resource.name);
  for (const auto &b : p.plan.unsupported)
    refused.push_back(b.name);
  if (!refused.empty()) {
    reportOnce(ANARI_SEVERITY_ERROR,
        "WEBGPU_MATERIAL_SLANG: override parameters must be plain data; '"
            + refused.front() + "' is not supported. Using the default shading.");
    return nullptr;
  }

  const std::string label = std::string("anari_slang_") + shader;
  p.shader = vtkslang::createShaderModule(device, artifact, label.c_str());
  if (!p.shader)
    return nullptr;
  uint32_t groupCount = 1;
  for (const auto &block : p.plan.blocks)
    groupCount = std::max(groupCount, block.group + 1);
  std::vector<WGPUBindGroupLayout> bgls(groupCount, nullptr);
  bgls[0] = group0;
  for (const auto &block : p.plan.blocks) {
    WGPUBindGroupLayoutEntry entry = uniformEntry(0);
    entry.buffer.minBindingSize = block.sizeBytes;
    p.blockLayouts.push_back(makeBGL(device, &entry, 1));
    bgls[block.group] = p.blockLayouts.back();
  }
  for (uint32_t g = 1; g < groupCount; ++g) {
    if (!bgls[g]) {
      p.emptyLayouts.push_back(makeBGL(device, nullptr, 0));
      bgls[g] = p.emptyLayouts.back();
    }
  }
  p.color = makePipeline(
      device, p.shader, bgls.data(), groupCount, label.c_str(), colorEntryPoint);
  p.depth = makePipeline(
      device, p.shader, bgls.data(), groupCount, label.c_str(), depthEntryPoint);
  p.valid = p.color && p.depth;
  return p.valid ? &p : nullptr;
}

Frame::SlangBindGroups Frame::makeSlangBindGroups(WGPUDevice device,
    WGPUQueue queue,
    const SlangPipelines *p,
    const SlangOverride &o)
{
  SlangBindGroups result;
  if (!p)
    return result;
  std::vector<std::string> names;
  for (const auto &parameter : o.parameters)
    names.push_back(parameter.first);
  for (const std::string &name : vtkslang::unmatched(p->plan, names))
    reportOnce(ANARI_SEVERITY_WARNING,
        "WEBGPU_MATERIAL_SLANG: parameter 'slang." + name
            + "' does not match a field of the override types");
  const auto &blocks = p->plan.blocks;
  for (uint32_t g = 1;; ++g) {
    size_t index = blocks.size();
    for (size_t i = 0; i < blocks.size(); ++i)
      index = blocks[i].group == g ? i : index;
    bool any = false;
    for (const auto &b : blocks)
      any |= b.group >= g;
    if (!any)
      break;
    if (index < blocks.size()) {
      std::vector<uint8_t> values = packSlangParameters(blocks[index], o);
      WGPUBuffer buffer = uploadUniform(device, queue, values.data(), values.size());
      result.buffers.push_back(buffer);
      result.groups.emplace_back(
          g, makeBindGroup1(device, p->blockLayouts[index], buffer, values.size()));
    } else {
      size_t emptyIndex = 0;
      for (uint32_t h = 1; h < g; ++h) {
        bool has = false;
        for (const auto &b : blocks)
          has |= b.group == h;
        emptyIndex += has ? 0 : 1;
      }
      WGPUBindGroupDescriptor desc{};
      desc.layout = p->emptyLayouts[emptyIndex];
      result.groups.emplace_back(g, wgpuDeviceCreateBindGroup(device, &desc));
    }
  }
  return result;
}

std::vector<uint8_t> Frame::packSlangParameters(
    const vtkslang::BindingPlan::Block &block, const SlangOverride &o)
{
  std::vector<uint8_t> data = vtkslang::blockBytes(block);
  for (const auto &[name, value] : o.parameters) {
    const vtkslang::UniformField *field = vtkslang::findField(block, name);
    if (!field)
      continue;
    const std::string prefix = "WEBGPU_MATERIAL_SLANG: parameter 'slang." + name + "' ";
    const std::optional<vtkslang::HostValue> host = hostValue(value);
    if (!host) {
      reportOnce(ANARI_SEVERITY_WARNING, prefix + "is not plain data");
      continue;
    }
    const std::string types = std::string("(") + anari::toString(value.type())
        + ") does not fit its field (" + field->type + ")";
    switch (vtkslang::pack(*field, *host, data)) {
    case vtkslang::PackStatus::Packed:
      break;
    case vtkslang::PackStatus::ScalarMismatch:
      reportOnce(ANARI_SEVERITY_WARNING, prefix + types + ": the scalar types differ");
      break;
    default:
      reportOnce(ANARI_SEVERITY_WARNING, prefix + types);
      break;
    }
  }
  return data;
}

void Frame::cleanupComputeResources()
{
  rel(m_readbackBuffer,  wgpuBufferRelease);
  rel(m_colorStorageBuf, wgpuBufferRelease);
  rel(m_depthStorageBuf, wgpuBufferRelease);

  rel(m_clearPipeline, wgpuComputePipelineRelease);
  rel(m_clearShader,   wgpuShaderModuleRelease);
  rel(m_clearBGL,      wgpuBindGroupLayoutRelease);

  rel(m_rasterPipeline,      wgpuComputePipelineRelease);
  rel(m_rasterDepthPipeline, wgpuComputePipelineRelease);
  rel(m_rasterShader,        wgpuShaderModuleRelease);
  rel(m_rasterBGL,           wgpuBindGroupLayoutRelease);

  rel(m_pbrPipeline,      wgpuComputePipelineRelease);
  rel(m_pbrDepthPipeline, wgpuComputePipelineRelease);
  rel(m_pbrShader,        wgpuShaderModuleRelease);
  rel(m_pbrBGL,           wgpuBindGroupLayoutRelease);

  for (auto &entry : m_slangPipelines) {
    SlangPipelines &p = entry.second;
    rel(p.color,         wgpuComputePipelineRelease);
    rel(p.depth,         wgpuComputePipelineRelease);
    rel(p.shader,        wgpuShaderModuleRelease);
    for (WGPUBindGroupLayout &layout : p.blockLayouts)
      rel(layout, wgpuBindGroupLayoutRelease);
    for (WGPUBindGroupLayout &layout : p.emptyLayouts)
      rel(layout, wgpuBindGroupLayoutRelease);
  }
  m_slangPipelines.clear();

  m_gpuResourcesInitialized = false;
}

bool Frame::getProperty(const std::string_view &name,
    ANARIDataType type, void *ptr, uint64_t size, uint32_t flags)
{
  if (type == ANARI_FLOAT32 && name == "duration") {
    helium::writeToVoidP(ptr, m_duration);
    return true;
  }
  return 0;
}

// ---------------------------------------------------------------------------

static WGPUBuffer uploadStorage(WGPUDevice device, WGPUQueue queue,
    const void *data, uint64_t size, const char *label)
{
  WGPUBufferDescriptor desc{};
  desc.nextInChain = nullptr;
  desc.label = WGPUStringView{label, strlen(label)};
  desc.usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst;
  desc.size = size;
  desc.mappedAtCreation = false;
  WGPUBuffer buf = wgpuDeviceCreateBuffer(device, &desc);
  wgpuQueueWriteBuffer(queue, buf, 0, data, size);
  return buf;
}

static WGPUBindGroup makeBindGroup7(WGPUDevice device, WGPUBindGroupLayout bgl,
    WGPUBuffer b0, uint64_t s0,
    WGPUBuffer b1, uint64_t s1,
    WGPUBuffer b2, uint64_t s2,
    WGPUBuffer b3, uint64_t s3,
    WGPUBuffer b4, uint64_t s4,
    WGPUBuffer b5, uint64_t s5,
    WGPUBuffer b6, uint64_t s6)
{
  WGPUBindGroupEntry e[7]{};
  auto setE = [&](int i, WGPUBuffer buf, uint64_t sz){
    e[i].binding = (uint32_t)i; e[i].buffer = buf;
    e[i].offset = 0; e[i].size = sz;
  };
  setE(0,b0,s0); setE(1,b1,s1); setE(2,b2,s2); setE(3,b3,s3);
  setE(4,b4,s4); setE(5,b5,s5); setE(6,b6,s6);

  WGPUBindGroupDescriptor bgDesc{};
  bgDesc.nextInChain = nullptr;
  bgDesc.layout = bgl;
  bgDesc.entryCount = 7;
  bgDesc.entries = e;
  return wgpuDeviceCreateBindGroup(device, &bgDesc);
}

// ---------------------------------------------------------------------------

void Frame::renderFrame()
{
  auto start = std::chrono::steady_clock::now();
  auto *state = deviceState();
  state->commitBuffer.flush();

  if (!m_valid || !m_world || m_size.x == 0 || m_size.y == 0)
    goto done;

  if (state->wgpuDevice) {
    if (!m_gpuResourcesInitialized)
      initComputeResources();

    if (m_gpuResourcesInitialized) {
      WGPUDevice device = state->wgpuDevice;
      WGPUQueue  queue  = state->wgpuQueue;
      uint32_t numPixels = m_size.x * m_size.y;

      auto bgc = m_renderer
          ? m_renderer->background()
          : float4(0.f, 0.f, 0.f, 1.f);

      WGPUCommandEncoderDescriptor ceDesc{};
      ceDesc.nextInChain = nullptr;
      ceDesc.label = WGPUStringView{"anari_cmd_encoder", 17};
      WGPUCommandEncoder encoder =
          wgpuDeviceCreateCommandEncoder(device, &ceDesc);

      {
        struct ClearUniforms {
          float bgColor[4]; uint32_t width, height, _pad[2];
        } cp;
        cp.bgColor[0] = bgc.x; cp.bgColor[1] = bgc.y;
        cp.bgColor[2] = bgc.z; cp.bgColor[3] = bgc.w;
        cp.width = m_size.x; cp.height = m_size.y;

        WGPUBuffer ub = uploadUniform(device, queue, &cp, sizeof(cp));

        WGPUBindGroupEntry be[3]{};
        be[0] = {nullptr,0,m_colorStorageBuf,0,numPixels*16ULL};
        be[1] = {nullptr,1,m_depthStorageBuf,0,numPixels*4ULL};
        be[2] = {nullptr,2,ub,0,sizeof(cp)};

        WGPUBindGroupDescriptor bgd{};
        bgd.nextInChain = nullptr;
        bgd.layout = m_clearBGL;
        bgd.entryCount = 3;
        bgd.entries = be;
        WGPUBindGroup bg = wgpuDeviceCreateBindGroup(device, &bgd);

        WGPUComputePassDescriptor passDesc{};
        passDesc.nextInChain = nullptr;
        passDesc.label = WGPUStringView{"anari_clear_pass", 16};
        passDesc.timestampWrites = nullptr;
        WGPUComputePassEncoder pass =
            wgpuCommandEncoderBeginComputePass(encoder, &passDesc);
        wgpuComputePassEncoderSetPipeline(pass, m_clearPipeline);
        wgpuComputePassEncoderSetBindGroup(pass, 0, bg, 0, nullptr);
        wgpuComputePassEncoderDispatchWorkgroups(
            pass, (numPixels + 255) / 256, 1, 1);
        wgpuComputePassEncoderEnd(pass);
        wgpuComputePassEncoderRelease(pass);
        wgpuBindGroupRelease(bg);
        wgpuBufferRelease(ub);
      }

      {
        auto *cam = dynamic_cast<PerspectiveCamera *>(m_camera.ptr);
        float aspect = float(m_size.x) / float(m_size.y);
        float fovy = cam ? cam->fovy() : float(M_PI / 3.0);
        float3 eye = cam ? cam->position() : float3(0,0,5);
        float3 dir = cam ? cam->direction() : float3(0,0,-1);
        float3 up  = cam ? cam->up() : float3(0,1,0);

        float3 f = linalg::normalize(dir);
        float3 s = linalg::normalize(linalg::cross(f, up));
        float3 u = linalg::cross(s, f);

        mat4 view;
        view[0] = {s.x, u.x, -f.x, 0.f};
        view[1] = {s.y, u.y, -f.y, 0.f};
        view[2] = {s.z, u.z, -f.z, 0.f};
        view[3] = {-linalg::dot(s,eye), -linalg::dot(u,eye), linalg::dot(f,eye), 1.f};

        float th = std::tan(fovy / 2.f), near = 0.1f, far = 1000.f;
        mat4 proj;
        proj[0] = {1.f/(aspect*th), 0, 0, 0};
        proj[1] = {0, 1.f/th, 0, 0};
        proj[2] = {0, 0, far/(near-far), -1.f};
        proj[3] = {0, 0, (far*near)/(near-far), 0};
        mat4 vp = linalg::mul(proj, view);

        float3 lightDir{0.f, -1.f, -1.f};
        float3 lightColor{1.f, 1.f, 1.f};
        float  lightIrr = 1.f;
        float  ambientRad = m_renderer ? m_renderer->ambientRadiance() : 0.3f;
        for (auto *inst : m_world->instances()) {
          if (!inst || !inst->group()) continue;
          for (auto *light : inst->group()->lights()) {
            auto *dl = dynamic_cast<DirectionalLight *>(light);
            if (dl) {
              lightDir   = dl->direction();
              lightColor = dl->color();
              lightIrr   = dl->irradiance();
            }
          }
        }

        for (auto *inst : m_world->instances()) {
          if (!inst || !inst->group()) continue;
          mat4 model = inst->xfm();
          mat4 mvp   = linalg::mul(vp, model);

          for (auto *surf : inst->group()->surfaces()) {
            if (!surf || !surf->isValid()) continue;
            auto *geom = dynamic_cast<const TriangleGeometry *>(surf->geometry());
            if (!geom || !geom->isValid()) continue;

            uint32_t numVerts = geom->numVertices();
            uint32_t numTris  = geom->numTriangles();
            const float    *pos  = geom->vertexPositions();
            const float    *norm = geom->vertexNormals();
            const float    *col  = geom->vertexColors();
            const uint32_t *idx  = geom->indices();

            std::vector<uint32_t> indexData;
            if (idx) {
              indexData.assign(idx, idx + numTris * 3);
            } else {
              indexData.resize(numTris * 3);
              for (uint32_t i = 0; i < numTris * 3; i++) indexData[i] = i;
            }

            float dummyNorm[3] = {0.f, 1.f, 0.f};
            float dummyCol[4]  = {1.f, 1.f, 1.f, 1.f};

            uint64_t posSize  = numVerts * 3 * sizeof(float);
            uint64_t normSize = norm ? numVerts * 3 * sizeof(float) : sizeof(dummyNorm);
            uint64_t colSize  = col  ? numVerts * 4 * sizeof(float) : sizeof(dummyCol);
            uint64_t idxSize  = indexData.size() * sizeof(uint32_t);

            WGPUBuffer posBuf  = uploadStorage(device, queue, pos, posSize, "pos");
            WGPUBuffer normBuf = uploadStorage(device, queue, norm ? norm : dummyNorm, normSize, "norm");
            WGPUBuffer colBuf  = uploadStorage(device, queue, col  ? col  : dummyCol,  colSize,  "col");
            WGPUBuffer idxBuf  = uploadStorage(device, queue, indexData.data(), idxSize, "idx");

            auto *pbrMat  = dynamic_cast<const PhysicallyBasedMaterial *>(surf->material());
            auto *matteMat = dynamic_cast<const MatteMaterial *>(surf->material());

            if (pbrMat) {
              struct PBRUniforms {
                float mvp[16];
                float baseColor[4];
                float emissive[4];
                float metallicRoughness[4];
                float lightDir[3]; float ambientRadiance;
                float lightColor[3]; float lightIrradiance;
                uint32_t width, height, numTriangles, hasNormals;
                uint32_t hasColors; uint32_t alphaMode; float alphaCutoff; uint32_t _pad;
              } pu;

              for (int c=0;c<4;c++) for (int r=0;r<4;r++) pu.mvp[c*4+r]=mvp[c][r];
              float4 bc = pbrMat->baseColor();
              pu.baseColor[0]=bc.x; pu.baseColor[1]=bc.y; pu.baseColor[2]=bc.z; pu.baseColor[3]=bc.w;
              float3 em = pbrMat->emissive();
              pu.emissive[0]=em.x; pu.emissive[1]=em.y; pu.emissive[2]=em.z;
              pu.emissive[3]=pbrMat->emissiveStrength();
              pu.metallicRoughness[0]=pbrMat->metallic();
              pu.metallicRoughness[1]=pbrMat->roughness();
              pu.metallicRoughness[2]=pu.metallicRoughness[3]=0.f;
              pu.lightDir[0]=lightDir.x; pu.lightDir[1]=lightDir.y; pu.lightDir[2]=lightDir.z;
              pu.ambientRadiance=ambientRad;
              pu.lightColor[0]=lightColor.x; pu.lightColor[1]=lightColor.y; pu.lightColor[2]=lightColor.z;
              pu.lightIrradiance=lightIrr;
              pu.width=m_size.x; pu.height=m_size.y;
              pu.numTriangles=numTris;
              pu.hasNormals=norm?1:0; pu.hasColors=col?1:0;
              pu.alphaMode   = static_cast<uint32_t>(pbrMat->alphaMode());
              pu.alphaCutoff = pbrMat->alphaCutoff();

              WGPUBuffer ub = uploadUniform(device, queue, &pu, sizeof(pu));

              WGPUBindGroup bg = makeBindGroup7(device, m_pbrBGL,
                  m_colorStorageBuf, numPixels*16ULL,
                  m_depthStorageBuf, numPixels*4ULL,
                  ub, sizeof(pu),
                  posBuf, posSize, normBuf, normSize, colBuf, colSize, idxBuf, idxSize);

              const SlangPipelines *sp = pbrMat->slangOverride().active()
                  ? slangPipelines(device, "pbr", m_pbrBGL, "pbr_main", "pbr_depth_main", pbrMat->slangOverride())
                  : nullptr;
              WGPUComputePipeline depthPipeline = sp ? sp->depth : m_pbrDepthPipeline;
              WGPUComputePipeline colorPipeline = sp ? sp->color : m_pbrPipeline;
              SlangBindGroups slangGroups = sp
                  ? makeSlangBindGroups(device, queue, sp, pbrMat->slangOverride())
                  : SlangBindGroups{};

              WGPUComputePassDescriptor passDesc{};
              passDesc.nextInChain = nullptr;
              passDesc.label = WGPUStringView{"anari_pbr_pass", 14};
              passDesc.timestampWrites = nullptr;
              {
                WGPUComputePassEncoder dpass =
                    wgpuCommandEncoderBeginComputePass(encoder, &passDesc);
                wgpuComputePassEncoderSetPipeline(dpass, depthPipeline);
                wgpuComputePassEncoderSetBindGroup(dpass, 0, bg, 0, nullptr);
                for (const auto &group : slangGroups.groups)
                  wgpuComputePassEncoderSetBindGroup(dpass, group.first, group.second, 0, nullptr);
                wgpuComputePassEncoderDispatchWorkgroups(
                    dpass, (numTris + 255) / 256, 1, 1);
                wgpuComputePassEncoderEnd(dpass);
                wgpuComputePassEncoderRelease(dpass);
              }
              WGPUComputePassEncoder pass =
                  wgpuCommandEncoderBeginComputePass(encoder, &passDesc);
              wgpuComputePassEncoderSetPipeline(pass, colorPipeline);
              wgpuComputePassEncoderSetBindGroup(pass, 0, bg, 0, nullptr);
              for (const auto &group : slangGroups.groups)
                wgpuComputePassEncoderSetBindGroup(pass, group.first, group.second, 0, nullptr);
              wgpuComputePassEncoderDispatchWorkgroups(
                  pass, (numTris + 255) / 256, 1, 1);
              wgpuComputePassEncoderEnd(pass);
              wgpuComputePassEncoderRelease(pass);
              wgpuBindGroupRelease(bg);
              wgpuBufferRelease(ub);
              for (const auto &group : slangGroups.groups)
                wgpuBindGroupRelease(group.second);
              for (WGPUBuffer buffer : slangGroups.buffers)
                wgpuBufferRelease(buffer);

            } else {
              float4 matColor = matteMat
                  ? matteMat->color() : float4(0.8f,0.8f,0.8f,1.f);

              struct RasterUniforms {
                float mvp[16];
                float materialColor[4];
                float lightDir[3]; float ambientRadiance;
                uint32_t width, height, numTriangles, hasNormals;
                uint32_t hasColors; uint32_t _pad[3];
              } ru;

              for (int c=0;c<4;c++) for (int r=0;r<4;r++) ru.mvp[c*4+r]=mvp[c][r];
              ru.materialColor[0]=matColor.x; ru.materialColor[1]=matColor.y;
              ru.materialColor[2]=matColor.z; ru.materialColor[3]=matColor.w;
              ru.lightDir[0]=lightDir.x; ru.lightDir[1]=lightDir.y; ru.lightDir[2]=lightDir.z;
              ru.ambientRadiance=ambientRad;
              ru.width=m_size.x; ru.height=m_size.y;
              ru.numTriangles=numTris;
              ru.hasNormals=norm?1:0; ru.hasColors=col?1:0;

              WGPUBuffer ub = uploadUniform(device, queue, &ru, sizeof(ru));

              WGPUBindGroup bg = makeBindGroup7(device, m_rasterBGL,
                  m_colorStorageBuf, numPixels*16ULL,
                  m_depthStorageBuf, numPixels*4ULL,
                  ub, sizeof(ru),
                  posBuf, posSize, normBuf, normSize, colBuf, colSize, idxBuf, idxSize);

              const SlangPipelines *sp = matteMat && matteMat->slangOverride().active()
                  ? slangPipelines(device, "raster", m_rasterBGL, "main", "depth_main", matteMat->slangOverride())
                  : nullptr;
              WGPUComputePipeline depthPipeline = sp ? sp->depth : m_rasterDepthPipeline;
              WGPUComputePipeline colorPipeline = sp ? sp->color : m_rasterPipeline;
              SlangBindGroups slangGroups = sp
                  ? makeSlangBindGroups(device, queue, sp, matteMat->slangOverride())
                  : SlangBindGroups{};

              WGPUComputePassDescriptor passDesc{};
              passDesc.nextInChain = nullptr;
              passDesc.label = WGPUStringView{"anari_raster_pass", 17};
              passDesc.timestampWrites = nullptr;
              {
                WGPUComputePassEncoder dpass =
                    wgpuCommandEncoderBeginComputePass(encoder, &passDesc);
                wgpuComputePassEncoderSetPipeline(dpass, depthPipeline);
                wgpuComputePassEncoderSetBindGroup(dpass, 0, bg, 0, nullptr);
                for (const auto &group : slangGroups.groups)
                  wgpuComputePassEncoderSetBindGroup(dpass, group.first, group.second, 0, nullptr);
                wgpuComputePassEncoderDispatchWorkgroups(
                    dpass, (numTris + 255) / 256, 1, 1);
                wgpuComputePassEncoderEnd(dpass);
                wgpuComputePassEncoderRelease(dpass);
              }
              WGPUComputePassEncoder pass =
                  wgpuCommandEncoderBeginComputePass(encoder, &passDesc);
              wgpuComputePassEncoderSetPipeline(pass, colorPipeline);
              wgpuComputePassEncoderSetBindGroup(pass, 0, bg, 0, nullptr);
              for (const auto &group : slangGroups.groups)
                wgpuComputePassEncoderSetBindGroup(pass, group.first, group.second, 0, nullptr);
              wgpuComputePassEncoderDispatchWorkgroups(
                  pass, (numTris + 255) / 256, 1, 1);
              wgpuComputePassEncoderEnd(pass);
              wgpuComputePassEncoderRelease(pass);
              wgpuBindGroupRelease(bg);
              wgpuBufferRelease(ub);
              for (const auto &group : slangGroups.groups)
                wgpuBindGroupRelease(group.second);
              for (WGPUBuffer buffer : slangGroups.buffers)
                wgpuBufferRelease(buffer);
            }

            wgpuBufferRelease(posBuf);
            wgpuBufferRelease(normBuf);
            wgpuBufferRelease(colBuf);
            wgpuBufferRelease(idxBuf);
          }
        }
      }

      for (auto *inst : m_world->instances()) {
        if (!inst || !inst->group()) continue;
        for (auto *vol : inst->group()->volumes()) {
          auto *tfVol = dynamic_cast<const TransferFunction1DVolume *>(vol);
          if (!tfVol || !tfVol->isValid()) continue;
          const StructuredRegularField *field = tfVol->field();
          if (!field || !field->isValid()) continue;

          float3 vMin = field->worldMin();
          float3 vMax = field->worldMax();
          uint3  dims = field->dims();
          uint64_t numVoxels = field->numVoxels();

          const uint32_t TF_SIZE = 256;
          std::vector<float4> tfTable = tfVol->buildTFTable(TF_SIZE);

          float3 sp = field->spacing();
          float stepSize = 0.5f * std::sqrt(sp.x*sp.x + sp.y*sp.y + sp.z*sp.z);

          WGPUBuffer voxBuf = uploadStorage(device, queue,
              field->rawData(), numVoxels * sizeof(float), "anari_vol_data");
          WGPUBuffer tfBuf  = uploadStorage(device, queue,
              tfTable.data(), TF_SIZE * sizeof(float4), "anari_vol_tf");

          auto *cam2 = dynamic_cast<PerspectiveCamera *>(m_camera.ptr);
          float aspect2 = float(m_size.x) / float(m_size.y);
          float fovy2 = cam2 ? cam2->fovy() : float(M_PI/3.0);
          float3 eye2 = cam2 ? cam2->position() : float3(0,0,5);
          float3 dir2 = cam2 ? cam2->direction() : float3(0,0,-1);
          float3 up2  = cam2 ? cam2->up() : float3(0,1,0);

          float3 ff = linalg::normalize(dir2);
          float3 ss = linalg::normalize(linalg::cross(ff, up2));
          float3 uu = linalg::cross(ss, ff);
          mat4 view2;
          view2[0] = {ss.x, uu.x, -ff.x, 0.f};
          view2[1] = {ss.y, uu.y, -ff.y, 0.f};
          view2[2] = {ss.z, uu.z, -ff.z, 0.f};
          view2[3] = {-linalg::dot(ss,eye2),-linalg::dot(uu,eye2),linalg::dot(ff,eye2),1.f};
          mat4 viewInv = linalg::inverse(view2);

          float th2 = std::tan(fovy2/2.f), near2=0.1f, far2=1000.f;
          mat4 proj2;
          proj2[0]={1.f/(aspect2*th2),0,0,0};
          proj2[1]={0,1.f/th2,0,0};
          proj2[2]={0,0,far2/(near2-far2),-1.f};
          proj2[3]={0,0,(far2*near2)/(near2-far2),0};
          mat4 projInv = linalg::inverse(proj2);

          struct VolumeUniforms {
            float viewInv[16];
            float projInv[16];
            float volumeMin[3]; float stepSize;
            float volumeMax[3]; float densityScale;
            uint32_t dims[3];   uint32_t tfSize;
            float valueRangeMin, valueRangeMax;
            uint32_t width, height;
          } vu;

          for (int c=0;c<4;c++) for (int r=0;r<4;r++) {
            vu.viewInv[c*4+r] = viewInv[c][r];
            vu.projInv[c*4+r] = projInv[c][r];
          }
          vu.volumeMin[0]=vMin.x; vu.volumeMin[1]=vMin.y; vu.volumeMin[2]=vMin.z;
          vu.stepSize = stepSize;
          vu.volumeMax[0]=vMax.x; vu.volumeMax[1]=vMax.y; vu.volumeMax[2]=vMax.z;
          vu.densityScale = tfVol->densityScale();
          vu.dims[0]=dims.x; vu.dims[1]=dims.y; vu.dims[2]=dims.z;
          vu.tfSize = TF_SIZE;
          float2 vr = tfVol->valueRange();
          vu.valueRangeMin=vr.x; vu.valueRangeMax=vr.y;
          vu.width=m_size.x; vu.height=m_size.y;

          WGPUBuffer ub = uploadUniform(device, queue, &vu, sizeof(vu));

          WGPUBindGroupEntry be5[5]{};
          be5[0] = {nullptr,0,m_colorStorageBuf,0,numPixels*16ULL};
          be5[1] = {nullptr,1,m_depthStorageBuf,0,numPixels*4ULL};
          be5[2] = {nullptr,2,ub,0,sizeof(vu)};
          be5[3] = {nullptr,3,voxBuf,0,numVoxels*sizeof(float)};
          be5[4] = {nullptr,4,tfBuf, 0,TF_SIZE*sizeof(float4)};

          WGPUBindGroupDescriptor bgd5{};
          bgd5.nextInChain = nullptr;
          bgd5.layout = m_volumeBGL;
          bgd5.entryCount = 5;
          bgd5.entries = be5;
          WGPUBindGroup bg5 = wgpuDeviceCreateBindGroup(device, &bgd5);

          WGPUComputePassDescriptor passDesc{};
          passDesc.nextInChain = nullptr;
          passDesc.label = WGPUStringView{"anari_volume_pass", 17};
          passDesc.timestampWrites = nullptr;
          WGPUComputePassEncoder pass =
              wgpuCommandEncoderBeginComputePass(encoder, &passDesc);
          wgpuComputePassEncoderSetPipeline(pass, m_volumePipeline);
          wgpuComputePassEncoderSetBindGroup(pass, 0, bg5, 0, nullptr);
          wgpuComputePassEncoderDispatchWorkgroups(
              pass, (numPixels + 255) / 256, 1, 1);
          wgpuComputePassEncoderEnd(pass);
          wgpuComputePassEncoderRelease(pass);

          wgpuBindGroupRelease(bg5);
          wgpuBufferRelease(ub);
          wgpuBufferRelease(voxBuf);
          wgpuBufferRelease(tfBuf);
        }
      }

      wgpuCommandEncoderCopyBufferToBuffer(encoder,
          m_colorStorageBuf, 0, m_readbackBuffer, 0,
          numPixels * sizeof(float) * 4);

      WGPUCommandBufferDescriptor cbDesc{};
      cbDesc.nextInChain = nullptr;
      cbDesc.label = WGPUStringView{"anari_cmd_buf", 13};
      WGPUCommandBuffer cmdBuf = wgpuCommandEncoderFinish(encoder, &cbDesc);
      wgpuQueueSubmit(queue, 1, &cmdBuf);
      wgpuCommandBufferRelease(cmdBuf);
      wgpuCommandEncoderRelease(encoder);

      MapContext mapCtx;
      WGPUBufferMapCallbackInfo mci{};
      mci.nextInChain = nullptr;
      mci.mode = WGPUCallbackMode_AllowProcessEvents;
      mci.callback = bufferMapCallback;
      mci.userdata1 = &mapCtx;
      uint64_t rbSize = numPixels * sizeof(float) * 4;
      wgpuBufferMapAsync(m_readbackBuffer, WGPUMapMode_Read, 0, rbSize, mci);
      while (!mapCtx.done) {
#if !defined(__EMSCRIPTEN__)
        wgpuInstanceProcessEvents(state->wgpuInstance);
#else
        break;
#endif
      }
      if (mapCtx.done && mapCtx.status == WGPUMapAsyncStatus_Success) {
        const float *mapped = (const float *)wgpuBufferGetConstMappedRange(
            m_readbackBuffer, 0, rbSize);
        if (mapped) {
          for (uint32_t y = 0; y < m_size.y; y++) {
            for (uint32_t x = 0; x < m_size.x; x++) {
              uint32_t i = y * m_size.x + x;
              float4 c{mapped[i*4], mapped[i*4+1], mapped[i*4+2], mapped[i*4+3]};
              writeSample(x, y, PixelSample(c));
            }
          }
        }
        wgpuBufferUnmap(m_readbackBuffer);
      }

    } else {
      auto bgc = m_renderer ? m_renderer->background() : float4(0,0,0,1);
      for (uint32_t y = 0; y < m_size.y; y++)
        for (uint32_t x = 0; x < m_size.x; x++)
          writeSample(x, y, PixelSample(bgc));
      softwareRasterize();
    }
  } else {
    auto bgc = m_renderer ? m_renderer->background() : float4(0,0,0,1);
    for (uint32_t y = 0; y < m_size.y; y++)
      for (uint32_t x = 0; x < m_size.x; x++)
        writeSample(x, y, PixelSample(bgc));
    softwareRasterize();
  }

done:
  auto end = std::chrono::steady_clock::now();
  m_duration = std::chrono::duration<float>(end - start).count();
}

// ---------------------------------------------------------------------------
// Software rasterizer fallback
// ---------------------------------------------------------------------------

void Frame::softwareRasterize()
{
  if (!m_world || !m_camera || m_size.x == 0 || m_size.y == 0
      || m_zBuffer.size() < size_t(m_size.x) * size_t(m_size.y))
    return;

  auto *cam = dynamic_cast<PerspectiveCamera *>(m_camera.ptr);
  if (!cam)
    return;

  float aspect = float(m_size.x) / float(m_size.y);
  float fovy = cam->fovy();
  float3 eye = cam->position(), dir = cam->direction(), up = cam->up();

  float3 f = linalg::normalize(dir);
  float3 s = linalg::normalize(linalg::cross(f, up));
  float3 u = linalg::cross(s, f);

  mat4 view;
  view[0] = {s.x, u.x, -f.x, 0.f};
  view[1] = {s.y, u.y, -f.y, 0.f};
  view[2] = {s.z, u.z, -f.z, 0.f};
  view[3] = {-linalg::dot(s,eye), -linalg::dot(u,eye), linalg::dot(f,eye), 1.f};

  float th = std::tan(fovy/2.f), near = 0.1f, far = 1000.f;
  mat4 proj;
  proj[0] = {1.f/(aspect*th), 0, 0, 0};
  proj[1] = {0, 1.f/th, 0, 0};
  proj[2] = {0, 0, far/(near-far), -1.f};
  proj[3] = {0, 0, (far*near)/(near-far), 0};
  mat4 vp = linalg::mul(proj, view);

  float3 lightDir = linalg::normalize(float3(0.f,-1.f,-1.f));
  float3 lightColor{1.f,1.f,1.f};
  float lightIrr = 1.f;
  float ambientRad = m_renderer ? m_renderer->ambientRadiance() : 0.3f;

  for (auto *inst : m_world->instances()) {
    if (!inst || !inst->group()) continue;
    for (auto *light : inst->group()->lights()) {
      auto *dl = dynamic_cast<DirectionalLight *>(light);
      if (dl) {
        lightDir   = linalg::normalize(dl->direction());
        lightColor = dl->color();
        lightIrr   = dl->irradiance();
      }
    }
  }

  std::fill(m_zBuffer.begin(), m_zBuffer.end(),
      std::numeric_limits<float>::max());

  for (auto *inst : m_world->instances()) {
    if (!inst || !inst->group()) continue;
    mat4 model = inst->xfm();
    mat4 mvp   = linalg::mul(vp, model);

    for (auto *surf : inst->group()->surfaces()) {
      if (!surf || !surf->isValid()) continue;
      auto *geom = dynamic_cast<const TriangleGeometry *>(surf->geometry());
      if (!geom || !geom->isValid()) continue;

      const float    *positions = geom->vertexPositions();
      const float    *normals   = geom->vertexNormals();
      const float    *colors    = geom->vertexColors();
      const float    *texcoords = geom->vertexTexCoords();
      const uint32_t *indices   = geom->indices();
      uint32_t numTris          = geom->numTriangles();

      auto *pbrMat  = dynamic_cast<const PhysicallyBasedMaterial *>(surf->material());
      auto *matteMat = dynamic_cast<const MatteMaterial *>(surf->material());
      if (surf->material() && surf->material()->slangOverride().active()) {
        reportOnce(ANARI_SEVERITY_WARNING,
            "WEBGPU_MATERIAL_SLANG overrides need the GPU path; software "
            "rendering uses the default shading");
      }

      for (uint32_t t = 0; t < numTris; t++) {
        uint32_t i0, i1, i2;
        if (indices) { i0=indices[t*3]; i1=indices[t*3+1]; i2=indices[t*3+2]; }
        else         { i0=t*3; i1=t*3+1; i2=t*3+2; }

        float3 p0{positions[i0*3], positions[i0*3+1], positions[i0*3+2]};
        float3 p1{positions[i1*3], positions[i1*3+1], positions[i1*3+2]};
        float3 p2{positions[i2*3], positions[i2*3+1], positions[i2*3+2]};

        float3 n0, n1, n2;
        if (normals) {
          n0 = {normals[i0*3], normals[i0*3+1], normals[i0*3+2]};
          n1 = {normals[i1*3], normals[i1*3+1], normals[i1*3+2]};
          n2 = {normals[i2*3], normals[i2*3+1], normals[i2*3+2]};
        } else {
          n0 = n1 = n2 = linalg::normalize(linalg::cross(p1-p0, p2-p0));
        }

        float2 uv0{0,0}, uv1{0,0}, uv2{0,0};
        if (texcoords) {
          uv0={texcoords[i0*2], texcoords[i0*2+1]};
          uv1={texcoords[i1*2], texcoords[i1*2+1]};
          uv2={texcoords[i2*2], texcoords[i2*2+1]};
        }

        float4 c0, c1, c2;
        if (pbrMat) {
          float4 bc = pbrMat->baseColor();
          c0=c1=c2=bc;
          if (colors) {
            float4 vc0{colors[i0*4],colors[i0*4+1],colors[i0*4+2],colors[i0*4+3]};
            float4 vc1{colors[i1*4],colors[i1*4+1],colors[i1*4+2],colors[i1*4+3]};
            float4 vc2{colors[i2*4],colors[i2*4+1],colors[i2*4+2],colors[i2*4+3]};
            c0=bc*vc0; c1=bc*vc1; c2=bc*vc2;
          }
        } else {
          float4 mc = matteMat ? matteMat->color() : float4(0.8f,0.8f,0.8f,1.f);
          c0=c1=c2=mc;
          if (colors) {
            float4 vc0{colors[i0*4],colors[i0*4+1],colors[i0*4+2],colors[i0*4+3]};
            float4 vc1{colors[i1*4],colors[i1*4+1],colors[i1*4+2],colors[i1*4+3]};
            float4 vc2{colors[i2*4],colors[i2*4+1],colors[i2*4+2],colors[i2*4+3]};
            c0=mc*vc0; c1=mc*vc1; c2=mc*vc2;
          }
        }

        auto transform = [&](float3 p) {
          return linalg::mul(mvp, float4(p.x,p.y,p.z,1.f));
        };
        float4 clip0=transform(p0), clip1=transform(p1), clip2=transform(p2);
        if (clip0.w<=0||clip1.w<=0||clip2.w<=0) continue;

        float3 ndc0{clip0.x/clip0.w, clip0.y/clip0.w, clip0.z/clip0.w};
        float3 ndc1{clip1.x/clip1.w, clip1.y/clip1.w, clip1.z/clip1.w};
        float3 ndc2{clip2.x/clip2.w, clip2.y/clip2.w, clip2.z/clip2.w};

        auto toScr = [&](float3 ndc) {
          return float3{(ndc.x*0.5f+0.5f)*m_size.x,
              (1.f-(ndc.y*0.5f+0.5f))*m_size.y, ndc.z};
        };
        float3 scr0=toScr(ndc0), scr1=toScr(ndc1), scr2=toScr(ndc2);

        float minX=std::max(0.f,std::min({scr0.x,scr1.x,scr2.x}));
        float maxX=std::min(float(m_size.x-1),std::max({scr0.x,scr1.x,scr2.x}));
        float minY=std::max(0.f,std::min({scr0.y,scr1.y,scr2.y}));
        float maxY=std::min(float(m_size.y-1),std::max({scr0.y,scr1.y,scr2.y}));

        auto edge=[](float3 a,float3 b,float2 c){
          return (c.x-a.x)*(b.y-a.y)-(c.y-a.y)*(b.x-a.x);
        };
        float area=edge(scr0,scr1,float2(scr2.x,scr2.y));
        if (std::abs(area)<1e-6f) continue;

        for (int py=int(minY); py<=int(maxY); py++) {
          for (int px=int(minX); px<=int(maxX); px++) {
            float2 pp{px+0.5f,py+0.5f};
            float w0=edge(scr1,scr2,pp)/area;
            float w1=edge(scr2,scr0,pp)/area;
            float w2=edge(scr0,scr1,pp)/area;
            if (w0<0||w1<0||w2<0) continue;

            float depth = w0*scr0.z + w1*scr1.z + w2*scr2.z;
            uint32_t idx = py*m_size.x + px;
            if (depth >= m_zBuffer[idx]) continue;
            m_zBuffer[idx] = depth;
            if (!m_depthBuffer.empty())
              m_depthBuffer[idx] = depth;

            float2 uv{uv0.x*w0+uv1.x*w1+uv2.x*w2, uv0.y*w0+uv1.y*w1+uv2.y*w2};
            float3 N = linalg::normalize(n0*w0 + n1*w1 + n2*w2);
            float3 L = linalg::normalize(-lightDir);

            float4 fragColor;
            if (pbrMat) {
              float4 bc = pbrMat->evalBaseColor(uv);
              if (colors) {
                float4 vc = c0*w0+c1*w1+c2*w2;
                bc = float4(bc.x*vc.x, bc.y*vc.y, bc.z*vc.z, bc.w*vc.w);
              }
              float2 mr = pbrMat->evalMetallicRoughness(uv);
              float metallic  = mr.x;
              float roughness = mr.y;
              float alpha = roughness * roughness;

              float3 baseRGB{bc.x, bc.y, bc.z};
              float3 V = linalg::normalize(float3(-ndc0.x*w0-ndc1.x*w1-ndc2.x*w2,
                                                  -ndc0.y*w0-ndc1.y*w1-ndc2.y*w2, 1.f));
              float3 H = linalg::normalize(V + L);

              float NdotL = std::max(linalg::dot(N,L), 0.f);
              float NdotV = std::max(linalg::dot(N,V), 0.0001f);
              float NdotH = std::max(linalg::dot(N,H), 0.f);
              float HdotV = std::max(linalg::dot(H,V), 0.f);

              float a2 = alpha*alpha;
              float denom = NdotH*NdotH*(a2-1.f)+1.f;
              float D = a2 / (float(M_PI)*denom*denom + 0.0001f);

              auto gSGGX = [&](float NdotX) {
                float r = roughness+1.f, k=(r*r)/8.f;
                return NdotX / (NdotX*(1.f-k)+k+0.0001f);
              };
              float G = gSGGX(NdotV)*gSGGX(NdotL);

              auto lerp3 = [](float3 a,float3 b,float t){
                return float3(a.x+(b.x-a.x)*t, a.y+(b.y-a.y)*t, a.z+(b.z-a.z)*t);
              };
              float3 F0 = lerp3(float3(0.04f,0.04f,0.04f), baseRGB, metallic);
              float fc = 1.f - HdotV;
              float fc5 = fc*fc*fc*fc*fc;
              float3 F{F0.x+(1.f-F0.x)*fc5, F0.y+(1.f-F0.y)*fc5, F0.z+(1.f-F0.z)*fc5};

              float3 spec{(D*G*F.x)/(4.f*NdotV*NdotL+0.0001f),
                          (D*G*F.y)/(4.f*NdotV*NdotL+0.0001f),
                          (D*G*F.z)/(4.f*NdotV*NdotL+0.0001f)};
              float3 kD{(1.f-F.x)*(1.f-metallic),
                        (1.f-F.y)*(1.f-metallic),
                        (1.f-F.z)*(1.f-metallic)};
              float3 diff{kD.x*baseRGB.x/float(M_PI),
                          kD.y*baseRGB.y/float(M_PI),
                          kD.z*baseRGB.z/float(M_PI)};

              float3 irr{lightIrr*lightColor.x, lightIrr*lightColor.y, lightIrr*lightColor.z};
              float3 Lo{(diff.x+spec.x)*irr.x*NdotL,
                        (diff.y+spec.y)*irr.y*NdotL,
                        (diff.z+spec.z)*irr.z*NdotL};
              Lo = float3(Lo.x+ambientRad*baseRGB.x,
                          Lo.y+ambientRad*baseRGB.y,
                          Lo.z+ambientRad*baseRGB.z);

              auto rh = [](float c){ return c/(c+1.f); };
              auto gm = [](float c){ return std::pow(std::max(c,0.f), 1.f/2.2f); };
              float3 tonemapped{gm(rh(Lo.x)), gm(rh(Lo.y)), gm(rh(Lo.z))};

              float3 em = pbrMat->evalEmissive(uv);
              fragColor = float4(tonemapped.x + em.x, tonemapped.y + em.y,
                                 tonemapped.z + em.z, bc.w);

              float op = pbrMat->evalOpacity(uv, bc.w);
              if (op <= 0.f) continue;
              fragColor.w = op;

            } else {
              float4 col4 = matteMat ? matteMat->evalColor(uv) : (c0*w0+c1*w1+c2*w2);
              if (colors) col4 = col4 * (c0*w0+c1*w1+c2*w2);
              float NdotL = std::max(linalg::dot(N,L), 0.f);
              float lighting = std::min(NdotL*lightIrr + ambientRad, 1.f);
              fragColor = float4(col4.x*lighting, col4.y*lighting, col4.z*lighting, col4.w);

              if (matteMat) {
                float op = matteMat->evalOpacity(uv, col4.w);
                if (op<=0.f) continue;
                fragColor.w = op;
              }
            }

            writeSample(px, py, PixelSample(fragColor, depth));
          }
        }
      }
    }
  }
}

void Frame::softwareRaymarchVolume()
{
  if (!m_world || !m_camera || m_size.x == 0 || m_size.y == 0)
    return;

  auto *cam = dynamic_cast<PerspectiveCamera *>(m_camera.ptr);
  if (!cam)
    return;

  float aspect = float(m_size.x) / float(m_size.y);
  float fovy   = cam->fovy();
  float3 eye = cam->position(), dir = cam->direction(), up = cam->up();

  float3 f = linalg::normalize(dir);
  float3 s = linalg::normalize(linalg::cross(f, up));
  float3 u = linalg::cross(s, f);

  mat4 view;
  view[0]={s.x,u.x,-f.x,0.f}; view[1]={s.y,u.y,-f.y,0.f};
  view[2]={s.z,u.z,-f.z,0.f};
  view[3]={-linalg::dot(s,eye),-linalg::dot(u,eye),linalg::dot(f,eye),1.f};
  mat4 viewInv = linalg::inverse(view);

  float th=std::tan(fovy/2.f), near=0.1f, far=1000.f;
  mat4 proj;
  proj[0]={1.f/(aspect*th),0,0,0}; proj[1]={0,1.f/th,0,0};
  proj[2]={0,0,far/(near-far),-1.f}; proj[3]={0,0,(far*near)/(near-far),0};
  mat4 projInv = linalg::inverse(proj);

  auto unproject = [&](int px, int py) -> float3 {
    float ndcX = (px + 0.5f) / float(m_size.x) * 2.f - 1.f;
    float ndcY = -((py + 0.5f) / float(m_size.y) * 2.f - 1.f);
    float4 clip{ndcX, ndcY, -1.f, 1.f};
    float4 vp4 = linalg::mul(projInv, clip);
    float3 vd = linalg::normalize(float3(vp4.x/vp4.w, vp4.y/vp4.w, -1.f));
    float4 wd4 = linalg::mul(viewInv, float4(vd.x, vd.y, vd.z, 0.f));
    return linalg::normalize(float3(wd4.x, wd4.y, wd4.z));
  };

  auto intersectAABB = [](float3 ro, float3 rd, float3 bmin, float3 bmax) -> float2 {
    float3 inv{1.f/rd.x, 1.f/rd.y, 1.f/rd.z};
    float3 t0{(bmin.x-ro.x)*inv.x, (bmin.y-ro.y)*inv.y, (bmin.z-ro.z)*inv.z};
    float3 t1{(bmax.x-ro.x)*inv.x, (bmax.y-ro.y)*inv.y, (bmax.z-ro.z)*inv.z};
    float tMin = std::max({std::min(t0.x,t1.x), std::min(t0.y,t1.y), std::min(t0.z,t1.z)});
    float tMax = std::min({std::max(t0.x,t1.x), std::max(t0.y,t1.y), std::max(t0.z,t1.z)});
    return float2{tMin, tMax};
  };

  for (auto *inst : m_world->instances()) {
    if (!inst || !inst->group()) continue;
    for (auto *vol : inst->group()->volumes()) {
      auto *tfVol = dynamic_cast<const TransferFunction1DVolume *>(vol);
      if (!tfVol || !tfVol->isValid()) continue;
      const StructuredRegularField *field = tfVol->field();
      if (!field) continue;

      float3 vMin = field->worldMin(), vMax = field->worldMax();
      float3 sp = field->spacing();
      float stepSize = 0.5f*std::sqrt(sp.x*sp.x+sp.y*sp.y+sp.z*sp.z);
      float ds = tfVol->densityScale();
      float2 vr = tfVol->valueRange();

      for (uint32_t py = 0; py < m_size.y; py++) {
        for (uint32_t px = 0; px < m_size.x; px++) {
          float3 rd = unproject((int)px, (int)py);
          float2 tHit = intersectAABB(eye, rd, vMin, vMax);
          if (tHit.x > tHit.y || tHit.y < 0.f) continue;
          float tStart = std::max(tHit.x, 0.f);
          float tEnd   = tHit.y;

          float3 accColor{0,0,0};
          float accAlpha = 0.f;

          for (float t = tStart; t <= tEnd; t += stepSize) {
            float3 pos = float3(eye.x+t*rd.x, eye.y+t*rd.y, eye.z+t*rd.z);
            float scalar = field->sampleWorld(pos);
            float norm = (scalar - vr.x) / (vr.y - vr.x + 1e-6f);
            float4 sample = tfVol->transferFunction(std::max(0.f,std::min(1.f,norm)));
            float sa = std::max(0.f,std::min(1.f, sample.w * ds * stepSize));

            accColor = float3(
                accColor.x + (1.f-accAlpha)*sample.x*sa,
                accColor.y + (1.f-accAlpha)*sample.y*sa,
                accColor.z + (1.f-accAlpha)*sample.z*sa);
            accAlpha += (1.f-accAlpha)*sa;
            if (accAlpha >= 0.99f) break;
          }

          if (accAlpha < 0.001f) continue;

          uint32_t idx = py*m_size.x + px;
          const auto *cpx = m_pixelBuffer.data() + idx*m_perPixelBytes;
          float4 geomCol{0,0,0,0};
          if (m_colorType == ANARI_FLOAT32_VEC4) {
            std::memcpy(&geomCol, cpx, sizeof(float4));
          } else {
            geomCol = float4(cpx[0]/255.f, cpx[1]/255.f, cpx[2]/255.f, cpx[3]/255.f);
          }
          float4 finalCol{
              accColor.x + geomCol.x*(1.f-accAlpha),
              accColor.y + geomCol.y*(1.f-accAlpha),
              accColor.z + geomCol.z*(1.f-accAlpha),
              std::min(geomCol.w + accAlpha, 1.f)};
          writeSample((int)px, (int)py, PixelSample(finalCol));
        }
      }
    }
  }
}

void *Frame::map(std::string_view channel,
    uint32_t *width, uint32_t *height, ANARIDataType *pixelType)
{
  wait();
  *width = m_size.x; *height = m_size.y;
  if (channel == "channel.color") {
    *pixelType = m_colorType;
    return m_pixelBuffer.data();
  } else if (channel == "channel.depth" && !m_depthBuffer.empty()) {
    *pixelType = ANARI_FLOAT32;
    return m_depthBuffer.data();
  }
  *width = 0; *height = 0; *pixelType = ANARI_UNKNOWN;
  return nullptr;
}

void Frame::unmap(std::string_view) {}
int Frame::frameReady(ANARIWaitMask m)
{
  if (m == ANARI_NO_WAIT) return ready();
  wait(); return 1;
}
void Frame::discard() {}
bool Frame::ready() const { return true; }
void Frame::wait() const {}

void Frame::writeSample(int x, int y, const PixelSample &s)
{
  const auto idx = y * m_size.x + x;
  auto *color = m_pixelBuffer.data() + (idx * m_perPixelBytes);
  switch (m_colorType) {
  case ANARI_UFIXED8_VEC4: {
    auto c = helium::cvt_color_to_uint32(s.color);
    std::memcpy(color, &c, sizeof(c)); break;
  }
  case ANARI_UFIXED8_RGBA_SRGB: {
    auto c = helium::cvt_color_to_uint32_srgb(s.color);
    std::memcpy(color, &c, sizeof(c)); break;
  }
  case ANARI_FLOAT32_VEC4:
    std::memcpy(color, &s.color, sizeof(s.color)); break;
  default: break;
  }
  if (!m_depthBuffer.empty())
    m_depthBuffer[idx] = s.depth;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Frame *);

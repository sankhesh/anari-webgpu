// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "Frame.h"
#include "ShaderPrograms.h"
#include "geometry/Geometry.h"
#include "material/Material.h"
#include "spatial_field/SpatialField.h"
#include "volume/Volume.h"
#include "SlangKitWebGPU.h"
// std
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

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

constexpr float kNear = 0.1f;
constexpr float kFar = 1000.f;
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
  waitOnOutstandingWorkIfNeeded();
  m_renderer = getParamObject<Renderer>("renderer");
  m_camera = getParamObject<Camera>("camera");
  m_world = getParamObject<World>("world");
  m_colorType = getParam<anari::DataType>("channel.color", ANARI_UNKNOWN);
  m_depthType = getParam<anari::DataType>("channel.depth", ANARI_UNKNOWN);
  m_size = getParam<uint2>("size", uint2(0u, 0u));
}

void Frame::finalize()
{
  waitOnOutstandingWorkIfNeeded();
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
  rel(m_volumeClassifyPipeline, wgpuComputePipelineRelease);
  rel(m_volumeShader,   wgpuShaderModuleRelease);
  rel(m_volumeBGL,      wgpuBindGroupLayoutRelease);
  rel(m_volumeClassifyBGL, wgpuBindGroupLayoutRelease);

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

static std::optional<slangkit::HostValue> hostValue(const helium::AnariAny &value)
{
  using slangkit::ScalarType;
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
      return slangkit::HostValue{s.scalar, s.width, s.runs, value.data()};
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
    desc.size = numPixels * sizeof(float) * 5;
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

    WGPUBindGroupLayoutEntry entries[8] = {
        storageEntry(0, false), storageEntry(1, false), uniformEntry(2),
        storageEntry(3, true), storageEntry(4, true),
        storageEntry(5, true), storageEntry(6, true), storageEntry(7, true)};
    m_rasterBGL = makeBGL(device, entries, 8);
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

    WGPUBindGroupLayoutEntry entries[8] = {
        storageEntry(0, false), storageEntry(1, false), uniformEntry(2),
        storageEntry(3, true), storageEntry(4, true),
        storageEntry(5, true), storageEntry(6, true), storageEntry(7, true)};
    m_pbrBGL = makeBGL(device, entries, 8);
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

    WGPUBindGroupLayoutEntry volEntries[9] = {storageEntry(0, false),
        storageEntry(1, false), uniformEntry(2), storageEntry(3, true),
        storageEntry(4, true), storageEntry(5, true), storageEntry(6, true),
        storageEntry(7, true), storageEntry(8, true)};
    m_volumeBGL = makeBGL(device, volEntries, 9);
    m_volumePipeline = makePipeline(device, m_volumeShader, m_volumeBGL,
        "anari_volume_pipeline", "vol_main");

    WGPUBindGroupLayoutEntry classifyEntries[4] = {uniformEntry(2),
        storageEntry(7, true), storageEntry(9, true), storageEntry(10, false)};
    m_volumeClassifyBGL = makeBGL(device, classifyEntries, 4);
    m_volumeClassifyPipeline = makePipeline(device, m_volumeShader,
        m_volumeClassifyBGL, "anari_volume_classify_pipeline", "vol_classify");
  }

  m_gpuResourcesInitialized = true;
}

WGPUShaderModule Frame::createBuiltinShader(WGPUDevice device, const char *name)
{
  slangkit::Artifact artifact =
      m_slangCompiler.compileProgram(shaderProgram(name));
  if (!artifact) {
    reportMessage(ANARI_SEVERITY_ERROR,
        "SlangKit failed to compile '%s':\n%s",
        name,
        artifact.diagnostics.c_str());
    return nullptr;
  }
  std::string label = std::string("anari_") + name;
  return slangkit::createShaderModule(device, artifact, label.c_str());
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

  slangkit::CompileOptions options;
  options.label = std::string("anari_slang_") + shader;
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
  slangkit::Artifact artifact =
      m_slangCompiler.compileProgram(shaderProgram(shader), options);
  if (!artifact) {
    reportOnce(ANARI_SEVERITY_ERROR,
        std::string("WEBGPU_MATERIAL_SLANG: '") + shader
            + "' failed to compile with the material's overrides; using its "
              "default shading:\n"
            + artifact.diagnostics);
    return nullptr;
  }
  p.plan = slangkit::planBindings(
      artifact, [](const slangkit::ReflectedBinding &b) { return b.group == 0; });
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
  p.shader = slangkit::createShaderModule(device, artifact, label.c_str());
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
  for (const std::string &name : slangkit::unmatched(p->plan, names))
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
    const slangkit::BindingPlan::Block &block, const SlangOverride &o)
{
  std::vector<uint8_t> data = slangkit::blockBytes(block);
  for (const auto &[name, value] : o.parameters) {
    const slangkit::UniformField *field = slangkit::findField(block, name);
    if (!field)
      continue;
    const std::string prefix = "WEBGPU_MATERIAL_SLANG: parameter 'slang." + name + "' ";
    const std::optional<slangkit::HostValue> host = hostValue(value);
    if (!host) {
      reportOnce(ANARI_SEVERITY_WARNING, prefix + "is not plain data");
      continue;
    }
    const std::string types = std::string("(") + anari::toString(value.type())
        + ") does not fit its field (" + field->type + ")";
    switch (slangkit::pack(*field, *host, data)) {
    case slangkit::PackStatus::Packed:
      break;
    case slangkit::PackStatus::ScalarMismatch:
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
    if (flags & ANARI_WAIT)
      wait();
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

static WGPUBindGroup makeBindGroup(WGPUDevice device, WGPUBindGroupLayout bgl,
    std::initializer_list<std::pair<WGPUBuffer, uint64_t>> buffers)
{
  std::vector<WGPUBindGroupEntry> e(buffers.size());
  uint32_t i = 0;
  for (const auto &b : buffers) {
    e[i].binding = i; e[i].buffer = b.first;
    e[i].offset = 0; e[i].size = b.second;
    i++;
  }

  WGPUBindGroupDescriptor bgDesc{};
  bgDesc.nextInChain = nullptr;
  bgDesc.layout = bgl;
  bgDesc.entryCount = e.size();
  bgDesc.entries = e.data();
  return wgpuDeviceCreateBindGroup(device, &bgDesc);
}

struct BufferBinding
{
  uint32_t binding;
  WGPUBuffer buffer;
  uint64_t size;
};

static WGPUBindGroup makeBindGroup(WGPUDevice device, WGPUBindGroupLayout bgl,
    std::initializer_list<BufferBinding> buffers)
{
  std::vector<WGPUBindGroupEntry> e(buffers.size());
  uint32_t i = 0;
  for (const auto &b : buffers) {
    e[i].binding = b.binding; e[i].buffer = b.buffer;
    e[i].offset = 0; e[i].size = b.size;
    i++;
  }

  WGPUBindGroupDescriptor bgDesc{};
  bgDesc.nextInChain = nullptr;
  bgDesc.layout = bgl;
  bgDesc.entryCount = e.size();
  bgDesc.entries = e.data();
  return wgpuDeviceCreateBindGroup(device, &bgDesc);
}

struct VolumeUniforms
{
  float inverseViewProjection[16];
  float viewProjection[16];
  float worldToTexture[16];
  float dims[4];
  float spacing[4];
  float cellDims[4];
  float eye[4];
  float valueMin, valueScale, baseStep, unitDistance;
  uint32_t width, height, colorCount, opacityCount;
  uint32_t maxSteps, nearest, _pad, cellCount;
};

static mat4 volumeTextureToWorld(
    const StructuredRegularField &field, const mat4 &xfm)
{
  const float3 lower = field.worldMin();
  const float3 extent = field.worldMax() - lower;
  auto nonzero = [](float e) { return e != 0.f ? e : 1e-6f; };
  mat4 m = linalg::identity;
  m[0][0] = nonzero(extent.x);
  m[1][1] = nonzero(extent.y);
  m[2][2] = nonzero(extent.z);
  m[3] = float4(lower.x, lower.y, lower.z, 1.f);
  return linalg::mul(xfm, m);
}

static uint32_t volumeMaxSteps(const mat4 &textureToWorld, float baseStep)
{
  float3 lo(std::numeric_limits<float>::max());
  float3 hi(std::numeric_limits<float>::lowest());
  for (int c = 0; c < 8; c++) {
    const float4 p = linalg::mul(textureToWorld,
        float4(float(c & 1), float((c >> 1) & 1), float((c >> 2) & 1), 1.f));
    lo = linalg::min(lo, p.xyz());
    hi = linalg::max(hi, p.xyz());
  }
  const float steps = 2.f * std::ceil(linalg::length(hi - lo) / baseStep) + 2.f;
  return uint32_t(std::min(steps, float(1u << 20)));
}

static void copyMatrix(float *dst, const mat4 &m)
{
  for (int c = 0; c < 4; c++)
    for (int r = 0; r < 4; r++)
      dst[c * 4 + r] = m[c][r];
}

struct Image2DUniforms
{
  float inTransform[16];
  float inOffset[4];
  float outTransform[16];
  float outOffset[4];
  uint32_t enabled, width, height, linearFilter, wrap1, wrap2, _pad[2];
};

static Image2DUniforms image2DUniforms(const Image2DSampler *s)
{
  Image2DUniforms u{};
  const Image2DSampler::Params p = s ? s->params() : Image2DSampler::Params{};
  for (int c = 0; c < 4; c++) {
    for (int r = 0; r < 4; r++) {
      u.inTransform[c*4+r] = p.inTransform[c][r];
      u.outTransform[c*4+r] = p.outTransform[c][r];
    }
    u.inOffset[c] = p.inOffset[c];
    u.outOffset[c] = p.outOffset[c];
  }
  u.enabled = s ? 1 : 0;
  u.width = s ? s->width() : 1;
  u.height = s ? s->height() : 1;
  u.linearFilter = p.linearFilter ? 1 : 0;
  u.wrap1 = static_cast<uint32_t>(p.wrap1);
  u.wrap2 = static_cast<uint32_t>(p.wrap2);
  return u;
}

// ---------------------------------------------------------------------------

void Frame::renderFrame()
{
  auto *state = deviceState();
  wait();
  refInc(helium::RefType::INTERNAL);
  state->taskQueue.enqueue([state]() { state->commitBuffer.flush(); });
  m_future = state->taskQueue.enqueue([this, state]() {
    state->renderingSemaphore.frameStart();
    render();
    state->renderingSemaphore.frameEnd();
  });
}

void Frame::render()
{
  auto start = std::chrono::steady_clock::now();
  auto *state = deviceState();

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
        mat4 vp = cam ? linalg::mul(cam->projectionMatrix(aspect, kNear, kFar),
                            cam->viewMatrix())
                      : mat4(linalg::identity);

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
            const uint32_t *idx  = geom->indices();
            const auto *material = surf->material();
            bool perPrimitive = false;
            std::vector<float4> colors;
            if (material && !material->colorAttribute().empty())
              colors = geom->attribute(material->colorAttribute(), perPrimitive);
            const float    *col  = colors.empty() ? nullptr : &colors[0].x;
            const Image2DSampler *image = material ? material->colorSampler() : nullptr;
            std::vector<float4> texels = image ? image->texels()
                                               : std::vector<float4>(1, float4(1.f, 1.f, 1.f, 1.f));

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
            uint64_t colSize  = col  ? colors.size() * sizeof(float4) : sizeof(dummyCol);
            uint64_t idxSize  = indexData.size() * sizeof(uint32_t);

            WGPUBuffer posBuf  = uploadStorage(device, queue, pos, posSize, "pos");
            WGPUBuffer normBuf = uploadStorage(device, queue, norm ? norm : dummyNorm, normSize, "norm");
            WGPUBuffer colBuf  = uploadStorage(device, queue, col  ? col  : dummyCol,  colSize,  "col");
            WGPUBuffer idxBuf  = uploadStorage(device, queue, indexData.data(), idxSize, "idx");
            uint64_t texSize   = texels.size() * sizeof(float4);
            WGPUBuffer texBuf  = uploadStorage(device, queue, texels.data(), texSize, "tex");

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
                Image2DUniforms image;
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
              pu.hasNormals=norm?1:0; pu.hasColors=col?(perPrimitive?2:1):0;
              pu.alphaMode   = static_cast<uint32_t>(pbrMat->alphaMode());
              pu.alphaCutoff = pbrMat->alphaCutoff();
              pu.image       = image2DUniforms(image);

              WGPUBuffer ub = uploadUniform(device, queue, &pu, sizeof(pu));

              WGPUBindGroup bg = makeBindGroup(device, m_pbrBGL,
                  {{m_colorStorageBuf, numPixels*16ULL}, {m_depthStorageBuf, numPixels*4ULL},
                   {ub, sizeof(pu)}, {posBuf, posSize}, {normBuf, normSize},
                   {colBuf, colSize}, {idxBuf, idxSize}, {texBuf, texSize}});

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
                uint32_t hasColors, alphaMode; float alphaCutoff; uint32_t _pad;
                Image2DUniforms image;
              } ru;

              for (int c=0;c<4;c++) for (int r=0;r<4;r++) ru.mvp[c*4+r]=mvp[c][r];
              ru.materialColor[0]=matColor.x; ru.materialColor[1]=matColor.y;
              ru.materialColor[2]=matColor.z; ru.materialColor[3]=matColor.w;
              ru.lightDir[0]=lightDir.x; ru.lightDir[1]=lightDir.y; ru.lightDir[2]=lightDir.z;
              ru.ambientRadiance=ambientRad;
              ru.width=m_size.x; ru.height=m_size.y;
              ru.numTriangles=numTris;
              ru.hasNormals=norm?1:0; ru.hasColors=col?(perPrimitive?2:1):0;
              ru.alphaMode = matteMat ? static_cast<uint32_t>(matteMat->alphaMode()) : 0;
              ru.alphaCutoff = matteMat ? matteMat->alphaCutoff() : 0.5f;
              ru.image = image2DUniforms(image);

              WGPUBuffer ub = uploadUniform(device, queue, &ru, sizeof(ru));

              WGPUBindGroup bg = makeBindGroup(device, m_rasterBGL,
                  {{m_colorStorageBuf, numPixels*16ULL}, {m_depthStorageBuf, numPixels*4ULL},
                   {ub, sizeof(ru)}, {posBuf, posSize}, {normBuf, normSize},
                   {colBuf, colSize}, {idxBuf, idxSize}, {texBuf, texSize}});

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
            wgpuBufferRelease(texBuf);
          }
        }
      }

      auto *volCam = dynamic_cast<PerspectiveCamera *>(m_camera.ptr);
      const float volAspect = float(m_size.x) / float(m_size.y);
      const mat4 volVP = volCam
          ? linalg::mul(volCam->projectionMatrix(volAspect, kNear, kFar),
                volCam->viewMatrix())
          : mat4(linalg::identity);
      const float samplingRate =
          m_renderer ? m_renderer->volumeSamplingRate() : 0.5f;
      const uint64_t depthBytes = numPixels * sizeof(uint32_t);
      WGPUBuffer surfaceDepth = nullptr;

      for (auto *inst : m_world->instances()) {
        if (!inst || !inst->group() || !volCam) continue;
        for (auto *vol : inst->group()->volumes()) {
          auto *tfVol = dynamic_cast<const TransferFunction1DVolume *>(vol);
          if (!tfVol || !tfVol->isValid()) continue;
          const StructuredRegularField &field = *tfVol->field();

          if (!surfaceDepth) {
            WGPUBufferDescriptor desc{};
            desc.label = WGPUStringView{"anari_vol_surface_depth", 23};
            desc.usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst;
            desc.size = depthBytes;
            surfaceDepth = wgpuDeviceCreateBuffer(device, &desc);
            wgpuCommandEncoderCopyBufferToBuffer(
                encoder, m_depthStorageBuf, 0, surfaceDepth, 0, depthBytes);
          }

          const mat4 textureToWorld = volumeTextureToWorld(field, inst->xfm());
          const float baseStep = field.stepSize() / samplingRate;
          const uint3 dims = field.dims();
          const float3 spacing = field.spacing();
          const uint3 cellDims = field.cellDims();
          const float2 range = tfVol->valueRange();
          const auto &values = field.values();
          const auto &colors = tfVol->colors();
          const auto &opacities = tfVol->opacities();
          const auto &ranges = field.cellRanges();
          const std::vector<float2> extremes = tfVol->opacityExtremes();

          VolumeUniforms vu{};
          copyMatrix(vu.inverseViewProjection, linalg::inverse(volVP));
          copyMatrix(vu.viewProjection, volVP);
          copyMatrix(vu.worldToTexture, linalg::inverse(textureToWorld));
          vu.dims[0] = float(dims.x); vu.dims[1] = float(dims.y);
          vu.dims[2] = float(dims.z);
          vu.spacing[0] = spacing.x; vu.spacing[1] = spacing.y;
          vu.spacing[2] = spacing.z;
          vu.cellDims[0] = float(cellDims.x); vu.cellDims[1] = float(cellDims.y);
          vu.cellDims[2] = float(cellDims.z);
          vu.cellDims[3] = float(StructuredRegularField::kCellSize);
          const float3 eye = volCam->position();
          vu.eye[0] = eye.x; vu.eye[1] = eye.y; vu.eye[2] = eye.z;
          vu.valueMin = range.x;
          vu.valueScale = range.y != range.x ? 1.f / (range.y - range.x) : 0.f;
          vu.baseStep = baseStep;
          vu.unitDistance = tfVol->unitDistance();
          vu.width = m_size.x;
          vu.height = m_size.y;
          vu.colorCount = uint32_t(colors.size());
          vu.opacityCount = uint32_t(opacities.size());
          vu.maxSteps = volumeMaxSteps(textureToWorld, baseStep);
          vu.nearest = field.nearestFilter() ? 1u : 0u;
          vu.cellCount = uint32_t(ranges.size());

          const uint64_t voxelBytes = values.size() * sizeof(float);
          const uint64_t colorBytes = colors.size() * sizeof(float4);
          const uint64_t opacityBytes = opacities.size() * sizeof(float);
          const uint64_t cellBytes = ranges.size() * sizeof(float2);
          const uint64_t extremesBytes = extremes.size() * sizeof(float2);
          WGPUBuffer ub = uploadUniform(device, queue, &vu, sizeof(vu));
          WGPUBuffer voxBuf = uploadStorage(
              device, queue, values.data(), voxelBytes, "anari_vol_data");
          WGPUBuffer colBuf = uploadStorage(
              device, queue, colors.data(), colorBytes, "anari_vol_color");
          WGPUBuffer opaBuf = uploadStorage(device, queue, opacities.data(),
              opacityBytes, "anari_vol_opacity");
          WGPUBuffer rangeBuf = uploadStorage(
              device, queue, ranges.data(), cellBytes, "anari_vol_ranges");
          WGPUBuffer extBuf = uploadStorage(device, queue, extremes.data(),
              extremesBytes, "anari_vol_extremes");
          WGPUBuffer cellBuf = nullptr;
          {
            WGPUBufferDescriptor desc{};
            desc.label = WGPUStringView{"anari_vol_cells", 15};
            desc.usage = WGPUBufferUsage_Storage;
            desc.size = cellBytes;
            cellBuf = wgpuDeviceCreateBuffer(device, &desc);
          }

          WGPUBindGroup classifyBG = makeBindGroup(device, m_volumeClassifyBGL,
              {{2, ub, sizeof(vu)}, {7, rangeBuf, cellBytes},
                  {9, extBuf, extremesBytes}, {10, cellBuf, cellBytes}});
          WGPUBindGroup marchBG = makeBindGroup(device, m_volumeBGL,
              {{m_colorStorageBuf, numPixels * 16ULL},
                  {m_depthStorageBuf, depthBytes}, {ub, sizeof(vu)},
                  {voxBuf, voxelBytes}, {colBuf, colorBytes},
                  {opaBuf, opacityBytes}, {cellBuf, cellBytes},
                  {rangeBuf, cellBytes}, {surfaceDepth, depthBytes}});

          WGPUComputePassDescriptor passDesc{};
          passDesc.label = WGPUStringView{"anari_volume_pass", 17};
          WGPUComputePassEncoder pass =
              wgpuCommandEncoderBeginComputePass(encoder, &passDesc);
          const uint32_t rows = (vu.cellCount + 65535u) / 65536u;
          const uint32_t columns = std::min((vu.cellCount + 63u) / 64u, 1024u);
          wgpuComputePassEncoderSetPipeline(pass, m_volumeClassifyPipeline);
          wgpuComputePassEncoderSetBindGroup(pass, 0, classifyBG, 0, nullptr);
          wgpuComputePassEncoderDispatchWorkgroups(pass, columns, rows, 1);
          wgpuComputePassEncoderSetPipeline(pass, m_volumePipeline);
          wgpuComputePassEncoderSetBindGroup(pass, 0, marchBG, 0, nullptr);
          wgpuComputePassEncoderDispatchWorkgroups(
              pass, (numPixels + 255) / 256, 1, 1);
          wgpuComputePassEncoderEnd(pass);
          wgpuComputePassEncoderRelease(pass);

          wgpuBindGroupRelease(classifyBG);
          wgpuBindGroupRelease(marchBG);
          for (WGPUBuffer b : {ub, voxBuf, colBuf, opaBuf, rangeBuf, extBuf, cellBuf})
            wgpuBufferRelease(b);
        }
      }
      if (surfaceDepth)
        wgpuBufferRelease(surfaceDepth);

      const uint64_t colorBytes = numPixels * sizeof(float) * 4;
      const bool readDepth = !m_depthBuffer.empty();
      wgpuCommandEncoderCopyBufferToBuffer(encoder,
          m_colorStorageBuf, 0, m_readbackBuffer, 0, colorBytes);
      if (readDepth)
        wgpuCommandEncoderCopyBufferToBuffer(encoder, m_depthStorageBuf, 0,
            m_readbackBuffer, colorBytes, numPixels * sizeof(float));

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
      uint64_t rbSize = colorBytes + (readDepth ? numPixels * sizeof(float) : 0);
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
          const float *depth = readDepth ? mapped + numPixels * 4 : nullptr;
          for (uint32_t y = 0; y < m_size.y; y++) {
            for (uint32_t x = 0; x < m_size.x; x++) {
              uint32_t i = y * m_size.x + x;
              float4 c{mapped[i*4], mapped[i*4+1], mapped[i*4+2], mapped[i*4+3]};
              if (depth && depth[i] < 1.f)
                writeSample(x, y, PixelSample(c, depth[i]));
              else
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
      softwareRaymarchVolume();
    }
  } else {
    auto bgc = m_renderer ? m_renderer->background() : float4(0,0,0,1);
    for (uint32_t y = 0; y < m_size.y; y++)
      for (uint32_t x = 0; x < m_size.x; x++)
        writeSample(x, y, PixelSample(bgc));
    softwareRasterize();
    softwareRaymarchVolume();
  }

  depthToDistance();

done:
  auto end = std::chrono::steady_clock::now();
  m_duration = std::chrono::duration<float>(end - start).count();
}

void Frame::depthToDistance()
{
  auto *cam = dynamic_cast<PerspectiveCamera *>(m_camera.ptr);
  if (m_depthBuffer.empty() || !cam)
    return;

  const float aspect = float(m_size.x) / float(m_size.y);
  const mat4 projInv =
      linalg::inverse(cam->projectionMatrix(aspect, kNear, kFar));
  for (uint32_t y = 0; y < m_size.y; y++) {
    for (uint32_t x = 0; x < m_size.x; x++) {
      float &d = m_depthBuffer[y * m_size.x + x];
      if (d == std::numeric_limits<float>::max())
        continue;
      const float4 p = linalg::mul(projInv,
          float4((x + 0.5f) / m_size.x * 2.f - 1.f,
              (y + 0.5f) / m_size.y * 2.f - 1.f,
              d,
              1.f));
      d = linalg::length(p.xyz() / p.w);
    }
  }
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
  mat4 vp = linalg::mul(
      cam->projectionMatrix(aspect, kNear, kFar), cam->viewMatrix());

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
      const auto *material = surf->material();
      bool perPrimitive = false;
      std::vector<float4> colors;
      if (material && !material->colorAttribute().empty())
        colors = geom->attribute(material->colorAttribute(), perPrimitive);
      const Image2DSampler *image = material ? material->colorSampler() : nullptr;
      bool perPrimitiveUV = false;
      const std::vector<float4> texcoords = geom->attribute("attribute0", perPrimitiveUV);
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

        const float4 &t0 = texcoords[perPrimitiveUV ? t : i0];
        const float4 &t1 = texcoords[perPrimitiveUV ? t : i1];
        const float4 &t2 = texcoords[perPrimitiveUV ? t : i2];
        float2 uv0{t0.x, t0.y}, uv1{t1.x, t1.y}, uv2{t2.x, t2.y};

        float4 c0{1.f, 1.f, 1.f, 1.f}, c1 = c0, c2 = c0;
        if (!colors.empty()) {
          c0 = colors[perPrimitive ? t : i0];
          c1 = colors[perPrimitive ? t : i1];
          c2 = colors[perPrimitive ? t : i2];
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
          const bool swap = a.x>b.x || (a.x==b.x && a.y>b.y);
          if (swap) std::swap(a,b);
          const float e = (c.x-a.x)*(b.y-a.y)-(c.y-a.y)*(b.x-a.x);
          return swap ? -e : e;
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
              m_depthBuffer[outputIndex(px, py)] = depth;

            float2 uv{uv0.x*w0+uv1.x*w1+uv2.x*w2, uv0.y*w0+uv1.y*w1+uv2.y*w2};
            float3 N = linalg::normalize(n0*w0 + n1*w1 + n2*w2);
            float3 L = linalg::normalize(-lightDir);

            float4 attr = c0*w0 + c1*w1 + c2*w2;
            if (image)
              attr = image->eval(attr);

            float4 fragColor;
            if (pbrMat) {
              float4 bc = pbrMat->baseColor() * attr;
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

              float3 em = pbrMat->evalEmissive(uv);
              fragColor = float4(Lo.x + em.x, Lo.y + em.y, Lo.z + em.z, bc.w);

              float op = pbrMat->evalOpacity(uv, bc.w);
              if (op <= 0.f) continue;
              fragColor.w = op;

            } else {
              float4 col4 = (matteMat ? matteMat->color() : float4(0.8f,0.8f,0.8f,1.f)) * attr;
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
  auto *cam = dynamic_cast<PerspectiveCamera *>(m_camera.ptr);
  if (!m_world || !cam || m_size.x == 0 || m_size.y == 0
      || m_zBuffer.size() < size_t(m_size.x) * size_t(m_size.y))
    return;

  const float aspect = float(m_size.x) / float(m_size.y);
  const mat4 vp = linalg::mul(
      cam->projectionMatrix(aspect, kNear, kFar), cam->viewMatrix());
  const mat4 vpInv = linalg::inverse(vp);
  const float samplingRate =
      m_renderer ? m_renderer->volumeSamplingRate() : 0.5f;
  auto unproject = [&](float2 ndc, float depth) {
    const float4 p = linalg::mul(vpInv, float4(ndc.x, ndc.y, depth, 1.f));
    return p.xyz() / p.w;
  };
  auto readColor = [&](uint32_t idx) {
    const auto *c = m_pixelBuffer.data() + idx * m_perPixelBytes;
    if (m_colorType == ANARI_FLOAT32_VEC4) {
      float4 v;
      std::memcpy(&v, c, sizeof(v));
      return v;
    }
    float4 v(c[0] / 255.f, c[1] / 255.f, c[2] / 255.f, c[3] / 255.f);
    if (m_colorType == ANARI_UFIXED8_RGBA_SRGB) {
      using helium::ToneMapMode;
      v = float4(helium::toneMap<ToneMapMode::FROM_SRGB>(v.x),
          helium::toneMap<ToneMapMode::FROM_SRGB>(v.y),
          helium::toneMap<ToneMapMode::FROM_SRGB>(v.z),
          v.w);
    }
    return v;
  };

  for (auto *inst : m_world->instances()) {
    if (!inst || !inst->group()) continue;
    for (auto *vol : inst->group()->volumes()) {
      auto *tfVol = dynamic_cast<const TransferFunction1DVolume *>(vol);
      if (!tfVol || !tfVol->isValid()) continue;
      const StructuredRegularField &field = *tfVol->field();

      const mat4 worldToTexture =
          linalg::inverse(volumeTextureToWorld(field, inst->xfm()));
      const float step = field.stepSize() / samplingRate;
      const float unitDistance = tfVol->unitDistance();
      const uint3 dims = field.dims();
      const float3 extent(float(std::max(dims.x, 2u) - 1),
          float(std::max(dims.y, 2u) - 1),
          float(std::max(dims.z, 2u) - 1));

      for (uint32_t py = 0; py < m_size.y; py++) {
        for (uint32_t px = 0; px < m_size.x; px++) {
          const float2 ndc((px + 0.5f) / m_size.x * 2.f - 1.f,
              1.f - (py + 0.5f) / m_size.y * 2.f);
          const float3 origin = cam->position();
          const float3 farPoint = unproject(ndc, 1.f);
          const float3 dir = linalg::normalize(farPoint - origin);
          float tLimit = linalg::length(farPoint - origin);
          const float surface = m_zBuffer[py * m_size.x + px];
          if (surface < 1.f)
            tLimit = std::min(
                tLimit, linalg::dot(unproject(ndc, surface) - origin, dir));

          const float3 o =
              linalg::mul(worldToTexture, float4(origin, 1.f)).xyz();
          const float3 d = linalg::mul(worldToTexture, float4(dir, 0.f)).xyz();
          const float3 inv = 1.f / d;
          const float3 t0 = -o * inv, t1 = (1.f - o) * inv;
          const float3 lo = linalg::min(t0, t1), hi = linalg::max(t0, t1);
          const float tNear = std::max({lo.x, lo.y, lo.z, 0.f});
          const float tFar = std::min({hi.x, hi.y, hi.z, tLimit});
          if (!(tNear < tFar))
            continue;

          float3 color(0.f, 0.f, 0.f);
          float alpha = 0.f;
          for (float t = tNear + 0.5f * step; t < tFar && alpha < 0.99f;
               t += step) {
            const float3 tc = linalg::clamp(o + t * d, float3(0.f), float3(1.f));
            const float n = tfVol->normalized(field.sampleVoxel(tc * extent));
            float opacity = tfVol->opacityOf(n);
            if (opacity <= 0.f)
              continue;
            opacity = opacity >= 1.f
                ? 1.f
                : 1.f - std::pow(1.f - opacity, step / unitDistance);
            const float4 c = tfVol->colorOf(n);
            color += (1.f - alpha) * opacity * float3(c.x, c.y, c.z);
            alpha += (1.f - alpha) * opacity;
          }

          const float4 entry =
              linalg::mul(vp, float4(origin + tNear * dir, 1.f));
          const uint32_t idx = outputIndex(px, py);
          float depth = std::numeric_limits<float>::max();
          if (!m_depthBuffer.empty())
            depth = std::min(
                m_depthBuffer[idx], std::clamp(entry.z / entry.w, 0.f, 1.f));

          color = linalg::min(color, float3(1.f));
          alpha = std::min(alpha, 1.f);
          const float4 behind = readColor(idx);
          writeSample(int(px),
              int(py),
              PixelSample(float4(color.x + behind.x * (1.f - alpha),
                              color.y + behind.y * (1.f - alpha),
                              color.z + behind.z * (1.f - alpha),
                              alpha + behind.w * (1.f - alpha)),
                  depth));
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

bool Frame::ready() const
{
  return !m_future.valid() || helium::tasking::isReady(m_future);
}

void Frame::wait()
{
  if (m_future.valid()) {
    m_future.get();
    refDec(helium::RefType::INTERNAL);
  }
}

void Frame::waitOnOutstandingWorkIfNeeded()
{
  if (!deviceState()->taskQueue.onWorkerThread())
    wait();
}

uint32_t Frame::outputIndex(int x, int y) const
{
  return (m_size.y - 1 - y) * m_size.x + x;
}

void Frame::writeSample(int x, int y, const PixelSample &s)
{
  const auto idx = outputIndex(x, y);
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

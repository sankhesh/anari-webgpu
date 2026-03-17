// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "Frame.h"
#include "geometry/Geometry.h"
#include "material/Material.h"
// std
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>

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
} // namespace

namespace anari_webgpu {

// ---------------------------------------------------------------------------
// Compute shaders (WGSL)
// ---------------------------------------------------------------------------

// Clear shader: fills the color and depth storage buffers with background
// color and max depth. Dispatched as ceil(numPixels / 256) workgroups.
static const char *s_clearShaderSource = R"(
struct ClearUniforms {
  bgColor : vec4<f32>,
  width   : u32,
  height  : u32,
};

@group(0) @binding(0) var<storage, read_write> colorBuf : array<vec4<f32>>;
@group(0) @binding(1) var<storage, read_write> depthBuf : array<f32>;
@group(0) @binding(2) var<uniform> params : ClearUniforms;

@compute @workgroup_size(256)
fn main(@builtin(global_invocation_id) gid : vec3<u32>) {
  let idx = gid.x;
  let numPixels = params.width * params.height;
  if (idx >= numPixels) { return; }
  colorBuf[idx] = params.bgColor;
  depthBuf[idx] = 1.0;  // far plane in NDC
}
)";

// Rasterization compute shader: each workgroup processes one triangle.
// Threads within the workgroup cooperatively rasterize the triangle's
// bounding box using atomic depth testing for correct ordering.
//
// The framebuffer is a plain storage buffer — no render attachments needed.
// This makes the output directly accessible to other compute shaders for
// compositing, post-processing, or readback, without texture barriers.
static const char *s_rasterShaderSource = R"(
struct RasterUniforms {
  mvp            : mat4x4<f32>,
  materialColor  : vec4<f32>,
  lightDir       : vec3<f32>,
  ambientRadiance: f32,
  width          : u32,
  height         : u32,
  numTriangles   : u32,
  hasNormals     : u32,
  hasColors      : u32,
  _pad           : vec3<u32>,
};

@group(0) @binding(0) var<storage, read_write> colorBuf   : array<vec4<f32>>;
@group(0) @binding(1) var<storage, read_write> depthBufI  : array<atomic<u32>>;
@group(0) @binding(2) var<uniform>             params     : RasterUniforms;
@group(0) @binding(3) var<storage, read>       positions  : array<f32>;
@group(0) @binding(4) var<storage, read>       normals    : array<f32>;
@group(0) @binding(5) var<storage, read>       colors     : array<f32>;
@group(0) @binding(6) var<storage, read>       indices    : array<u32>;

// Encode float depth [0,1] as u32 for atomicMin comparison
fn depthToU32(d : f32) -> u32 {
  return bitcast<u32>(clamp(d, 0.0, 1.0));
}

fn getPosition(idx : u32) -> vec3<f32> {
  return vec3<f32>(positions[idx * 3u], positions[idx * 3u + 1u], positions[idx * 3u + 2u]);
}

fn getNormal(idx : u32) -> vec3<f32> {
  return vec3<f32>(normals[idx * 3u], normals[idx * 3u + 1u], normals[idx * 3u + 2u]);
}

fn getColor(idx : u32) -> vec4<f32> {
  return vec4<f32>(colors[idx * 4u], colors[idx * 4u + 1u], colors[idx * 4u + 2u], colors[idx * 4u + 3u]);
}

fn transformPoint(p : vec3<f32>) -> vec4<f32> {
  return params.mvp * vec4<f32>(p, 1.0);
}

fn edgeFunction(a : vec2<f32>, b : vec2<f32>, c : vec2<f32>) -> f32 {
  return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

@compute @workgroup_size(256)
fn main(@builtin(global_invocation_id) gid : vec3<u32>) {
  // Each thread handles a unique (triangle, pixel-row-tile) pair.
  // We dispatch enough threads: ceil(numTriangles * height / 256)
  // to cover all triangles. Each invocation processes one triangle.
  let triIdx = gid.x;
  if (triIdx >= params.numTriangles) { return; }

  // Fetch vertex indices
  let i0 = indices[triIdx * 3u];
  let i1 = indices[triIdx * 3u + 1u];
  let i2 = indices[triIdx * 3u + 2u];

  let p0 = getPosition(i0);
  let p1 = getPosition(i1);
  let p2 = getPosition(i2);

  // Transform to clip space
  let clip0 = transformPoint(p0);
  let clip1 = transformPoint(p1);
  let clip2 = transformPoint(p2);

  // Cull behind-camera triangles
  if (clip0.w <= 0.0 || clip1.w <= 0.0 || clip2.w <= 0.0) { return; }

  // Perspective divide → NDC
  let ndc0 = clip0.xyz / clip0.w;
  let ndc1 = clip1.xyz / clip1.w;
  let ndc2 = clip2.xyz / clip2.w;

  // NDC to screen space
  let w = f32(params.width);
  let h = f32(params.height);
  let scr0 = vec3<f32>((ndc0.x * 0.5 + 0.5) * w, (1.0 - (ndc0.y * 0.5 + 0.5)) * h, ndc0.z);
  let scr1 = vec3<f32>((ndc1.x * 0.5 + 0.5) * w, (1.0 - (ndc1.y * 0.5 + 0.5)) * h, ndc1.z);
  let scr2 = vec3<f32>((ndc2.x * 0.5 + 0.5) * w, (1.0 - (ndc2.y * 0.5 + 0.5)) * h, ndc2.z);

  // Bounding box
  let minX = max(0.0, min(min(scr0.x, scr1.x), scr2.x));
  let maxX = min(w - 1.0, max(max(scr0.x, scr1.x), scr2.x));
  let minY = max(0.0, min(min(scr0.y, scr1.y), scr2.y));
  let maxY = min(h - 1.0, max(max(scr0.y, scr1.y), scr2.y));

  let area = edgeFunction(scr0.xy, scr1.xy, scr2.xy);
  if (abs(area) < 0.000001) { return; }

  // Compute face normal for lighting
  var faceNormal : vec3<f32>;
  if (params.hasNormals != 0u) {
    let n0 = getNormal(i0);
    let n1 = getNormal(i1);
    let n2 = getNormal(i2);
    faceNormal = normalize((n0 + n1 + n2) / 3.0);
  } else {
    faceNormal = normalize(cross(p1 - p0, p2 - p0));
  }

  let NdotL = max(dot(faceNormal, normalize(-params.lightDir)), 0.0);
  let lighting = min(NdotL + params.ambientRadiance, 1.0);

  // Vertex colors
  var c0 = params.materialColor;
  var c1 = params.materialColor;
  var c2 = params.materialColor;
  if (params.hasColors != 0u) {
    c0 = getColor(i0) * params.materialColor;
    c1 = getColor(i1) * params.materialColor;
    c2 = getColor(i2) * params.materialColor;
  }

  // Rasterize: iterate over bounding box pixels
  for (var py = i32(minY); py <= i32(maxY); py++) {
    for (var px = i32(minX); px <= i32(maxX); px++) {
      let p = vec2<f32>(f32(px) + 0.5, f32(py) + 0.5);
      let w0 = edgeFunction(scr1.xy, scr2.xy, p) / area;
      let w1 = edgeFunction(scr2.xy, scr0.xy, p) / area;
      let w2 = edgeFunction(scr0.xy, scr1.xy, p) / area;

      if (w0 >= 0.0 && w1 >= 0.0 && w2 >= 0.0) {
        let depth = w0 * scr0.z + w1 * scr1.z + w2 * scr2.z;
        let depthU = depthToU32(depth);
        let pixIdx = u32(py) * params.width + u32(px);

        // Atomic depth test: only write if this fragment is closer
        let prev = atomicMin(&depthBufI[pixIdx], depthU);
        if (depthU <= prev) {
          let color = c0 * w0 + c1 * w1 + c2 * w2;
          colorBuf[pixIdx] = vec4<f32>(color.rgb * lighting, color.a);
        }
      }
    }
  }
}
)";

Frame::Frame(WebGPUDeviceGlobalState *s) : helium::BaseFrame(s) {}

Frame::~Frame()
{
  wait();
  cleanupComputeResources();
}

bool Frame::isValid() const
{
  return m_valid;
}

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
  if (!m_renderer) {
    reportMessage(ANARI_SEVERITY_WARNING,
        "missing required parameter 'renderer' on frame");
  }

  if (!m_camera) {
    reportMessage(
        ANARI_SEVERITY_WARNING, "missing required parameter 'camera' on frame");
  }

  if (!m_world) {
    reportMessage(
        ANARI_SEVERITY_WARNING, "missing required parameter 'world' on frame");
  }

  const auto numPixels = m_size.x * m_size.y;

  m_perPixelBytes = 4 * (m_colorType == ANARI_FLOAT32_VEC4 ? 4 : 1);
  m_pixelBuffer.resize(numPixels * m_perPixelBytes);

  m_depthBuffer.resize(m_depthType == ANARI_FLOAT32 ? numPixels : 0);

  m_valid = m_renderer && m_camera && m_camera->isValid() && m_world
      && m_world->isValid();

  // Recreate GPU resources at the new size
  cleanupComputeResources();
  m_gpuResourcesInitialized = false;
}

// ---------------------------------------------------------------------------
// Compute resource initialization
// ---------------------------------------------------------------------------

void Frame::initComputeResources()
{
  auto *state = deviceState();
  if (!state->wgpuDevice || m_size.x == 0 || m_size.y == 0)
    return;

  WGPUDevice device = state->wgpuDevice;
  uint32_t numPixels = m_size.x * m_size.y;

  // ---- Storage buffers for framebuffer (sharable with external code) ----
  {
    WGPUBufferDescriptor desc{};
    desc.nextInChain = nullptr;
    desc.label = WGPUStringView{
        "anari_fb_color", strlen("anari_fb_color")};
    desc.usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopySrc;
    desc.size = numPixels * sizeof(float) * 4; // RGBA32F
    desc.mappedAtCreation = false;
    m_colorStorageBuf = wgpuDeviceCreateBuffer(device, &desc);
  }
  {
    WGPUBufferDescriptor desc{};
    desc.nextInChain = nullptr;
    desc.label = WGPUStringView{
        "anari_fb_depth", strlen("anari_fb_depth")};
    desc.usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopySrc;
    desc.size = numPixels * sizeof(uint32_t); // atomic<u32> for depth
    desc.mappedAtCreation = false;
    m_depthStorageBuf = wgpuDeviceCreateBuffer(device, &desc);
  }

  // ---- Readback buffer (CPU-side) ----
  {
    WGPUBufferDescriptor desc{};
    desc.nextInChain = nullptr;
    desc.label = WGPUStringView{
        "anari_fb_readback", strlen("anari_fb_readback")};
    desc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead;
    desc.size = numPixels * sizeof(float) * 4;
    desc.mappedAtCreation = false;
    m_readbackBuffer = wgpuDeviceCreateBuffer(device, &desc);
  }

  // ---- Clear compute pipeline ----
  {
    WGPUShaderSourceWGSL wgslSource{};
    wgslSource.chain.sType = WGPUSType_ShaderSourceWGSL;
    wgslSource.chain.next = nullptr;
    wgslSource.code = WGPUStringView{
        s_clearShaderSource, strlen(s_clearShaderSource)};

    WGPUShaderModuleDescriptor smDesc{};
    smDesc.nextInChain = &wgslSource.chain;
    smDesc.label = WGPUStringView{
        "anari_clear_shader", strlen("anari_clear_shader")};
    m_clearShader = wgpuDeviceCreateShaderModule(device, &smDesc);

    // Bind group layout: [0] colorBuf storage RW, [1] depthBuf storage RW,
    //                     [2] uniforms
    WGPUBindGroupLayoutEntry entries[3] = {};
    entries[0].binding = 0;
    entries[0].visibility = WGPUShaderStage_Compute;
    entries[0].buffer.type = WGPUBufferBindingType_Storage;
    entries[0].buffer.minBindingSize = 0;

    entries[1].binding = 1;
    entries[1].visibility = WGPUShaderStage_Compute;
    entries[1].buffer.type = WGPUBufferBindingType_Storage;
    entries[1].buffer.minBindingSize = 0;

    entries[2].binding = 2;
    entries[2].visibility = WGPUShaderStage_Compute;
    entries[2].buffer.type = WGPUBufferBindingType_Uniform;
    entries[2].buffer.minBindingSize = 0;

    WGPUBindGroupLayoutDescriptor bglDesc{};
    bglDesc.nextInChain = nullptr;
    bglDesc.entryCount = 3;
    bglDesc.entries = entries;
    m_clearBGL = wgpuDeviceCreateBindGroupLayout(device, &bglDesc);

    WGPUPipelineLayoutDescriptor plDesc{};
    plDesc.nextInChain = nullptr;
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = &m_clearBGL;
    WGPUPipelineLayout pipelineLayout =
        wgpuDeviceCreatePipelineLayout(device, &plDesc);

    WGPUComputePipelineDescriptor cpDesc{};
    cpDesc.nextInChain = nullptr;
    cpDesc.label = WGPUStringView{
        "anari_clear_pipeline", strlen("anari_clear_pipeline")};
    cpDesc.layout = pipelineLayout;
    cpDesc.compute.module = m_clearShader;
    cpDesc.compute.entryPoint = WGPUStringView{"main", 4};
    cpDesc.compute.constantCount = 0;
    m_clearPipeline = wgpuDeviceCreateComputePipeline(device, &cpDesc);

    wgpuPipelineLayoutRelease(pipelineLayout);
  }

  // ---- Rasterize compute pipeline ----
  {
    WGPUShaderSourceWGSL wgslSource{};
    wgslSource.chain.sType = WGPUSType_ShaderSourceWGSL;
    wgslSource.chain.next = nullptr;
    wgslSource.code = WGPUStringView{
        s_rasterShaderSource, strlen(s_rasterShaderSource)};

    WGPUShaderModuleDescriptor smDesc{};
    smDesc.nextInChain = &wgslSource.chain;
    smDesc.label = WGPUStringView{
        "anari_raster_shader", strlen("anari_raster_shader")};
    m_rasterShader = wgpuDeviceCreateShaderModule(device, &smDesc);

    // Bind group layout:
    // [0] colorBuf    storage RW
    // [1] depthBufI   storage RW (atomic<u32>)
    // [2] uniforms    uniform
    // [3] positions   storage RO
    // [4] normals     storage RO
    // [5] colors      storage RO
    // [6] indices     storage RO
    // [7] colorBufW   storage RW (alias of [0] — for WGSL dual-binding)
    WGPUBindGroupLayoutEntry entries[7] = {};
    entries[0].binding = 0;
    entries[0].visibility = WGPUShaderStage_Compute;
    entries[0].buffer.type = WGPUBufferBindingType_Storage;
    entries[0].buffer.minBindingSize = 0;

    entries[1].binding = 1;
    entries[1].visibility = WGPUShaderStage_Compute;
    entries[1].buffer.type = WGPUBufferBindingType_Storage;
    entries[1].buffer.minBindingSize = 0;

    entries[2].binding = 2;
    entries[2].visibility = WGPUShaderStage_Compute;
    entries[2].buffer.type = WGPUBufferBindingType_Uniform;
    entries[2].buffer.minBindingSize = 0;

    entries[3].binding = 3;
    entries[3].visibility = WGPUShaderStage_Compute;
    entries[3].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
    entries[3].buffer.minBindingSize = 0;

    entries[4].binding = 4;
    entries[4].visibility = WGPUShaderStage_Compute;
    entries[4].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
    entries[4].buffer.minBindingSize = 0;

    entries[5].binding = 5;
    entries[5].visibility = WGPUShaderStage_Compute;
    entries[5].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
    entries[5].buffer.minBindingSize = 0;

    entries[6].binding = 6;
    entries[6].visibility = WGPUShaderStage_Compute;
    entries[6].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
    entries[6].buffer.minBindingSize = 0;

    WGPUBindGroupLayoutDescriptor bglDesc{};
    bglDesc.nextInChain = nullptr;
    bglDesc.entryCount = 7;
    bglDesc.entries = entries;
    m_rasterBGL = wgpuDeviceCreateBindGroupLayout(device, &bglDesc);

    WGPUPipelineLayoutDescriptor plDesc{};
    plDesc.nextInChain = nullptr;
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = &m_rasterBGL;
    WGPUPipelineLayout pipelineLayout =
        wgpuDeviceCreatePipelineLayout(device, &plDesc);

    WGPUComputePipelineDescriptor cpDesc{};
    cpDesc.nextInChain = nullptr;
    cpDesc.label = WGPUStringView{
        "anari_raster_pipeline", strlen("anari_raster_pipeline")};
    cpDesc.layout = pipelineLayout;
    cpDesc.compute.module = m_rasterShader;
    cpDesc.compute.entryPoint = WGPUStringView{"main", 4};
    cpDesc.compute.constantCount = 0;
    m_rasterPipeline = wgpuDeviceCreateComputePipeline(device, &cpDesc);

    wgpuPipelineLayoutRelease(pipelineLayout);
  }

  m_gpuResourcesInitialized = true;
}

void Frame::cleanupComputeResources()
{
  auto release = [](auto &handle) {
    if (handle) {
      // All WebGPU handle types have a matching wgpu*Release function,
      // but we use the generic release pattern here.
    }
  };

  if (m_readbackBuffer) {
    wgpuBufferRelease(m_readbackBuffer);
    m_readbackBuffer = nullptr;
  }
  if (m_colorStorageBuf) {
    wgpuBufferRelease(m_colorStorageBuf);
    m_colorStorageBuf = nullptr;
  }
  if (m_depthStorageBuf) {
    wgpuBufferRelease(m_depthStorageBuf);
    m_depthStorageBuf = nullptr;
  }

  if (m_clearPipeline) {
    wgpuComputePipelineRelease(m_clearPipeline);
    m_clearPipeline = nullptr;
  }
  if (m_clearShader) {
    wgpuShaderModuleRelease(m_clearShader);
    m_clearShader = nullptr;
  }
  if (m_clearBGL) {
    wgpuBindGroupLayoutRelease(m_clearBGL);
    m_clearBGL = nullptr;
  }

  if (m_rasterPipeline) {
    wgpuComputePipelineRelease(m_rasterPipeline);
    m_rasterPipeline = nullptr;
  }
  if (m_rasterShader) {
    wgpuShaderModuleRelease(m_rasterShader);
    m_rasterShader = nullptr;
  }
  if (m_rasterBGL) {
    wgpuBindGroupLayoutRelease(m_rasterBGL);
    m_rasterBGL = nullptr;
  }

  m_gpuResourcesInitialized = false;
}

bool Frame::getProperty(const std::string_view &name,
    ANARIDataType type,
    void *ptr,
    uint64_t size,
    uint32_t flags)
{
  if (type == ANARI_FLOAT32 && name == "duration") {
    helium::writeToVoidP(ptr, m_duration);
    return true;
  }

  return 0;
}

// ---------------------------------------------------------------------------
// renderFrame — compute-based GPU path
// ---------------------------------------------------------------------------

void Frame::renderFrame()
{
  auto start = std::chrono::steady_clock::now();

  auto *state = deviceState();
  state->commitBuffer.flush();

  if (!m_valid || !m_world)
    goto done;

  // If WebGPU device is available, use compute-based rendering
  if (state->wgpuDevice) {
    if (!m_gpuResourcesInitialized)
      initComputeResources();

    if (m_gpuResourcesInitialized) {
      WGPUDevice device = state->wgpuDevice;
      WGPUQueue queue = state->wgpuQueue;
      uint32_t numPixels = m_size.x * m_size.y;

      auto bgc =
          m_renderer ? m_renderer->background() : float4(0.f, 0.f, 0.f, 1.f);

      WGPUCommandEncoderDescriptor ceDesc{};
      ceDesc.nextInChain = nullptr;
      ceDesc.label = WGPUStringView{
          "anari_cmd_encoder", strlen("anari_cmd_encoder")};
      WGPUCommandEncoder encoder =
          wgpuDeviceCreateCommandEncoder(device, &ceDesc);

      // ==== Pass 1: Clear framebuffer via compute ====
      {
        struct ClearUniforms {
          float bgColor[4];
          uint32_t width;
          uint32_t height;
          uint32_t _pad[2];
        } clearParams;
        clearParams.bgColor[0] = bgc.x;
        clearParams.bgColor[1] = bgc.y;
        clearParams.bgColor[2] = bgc.z;
        clearParams.bgColor[3] = bgc.w;
        clearParams.width = m_size.x;
        clearParams.height = m_size.y;

        WGPUBufferDescriptor ubDesc{};
        ubDesc.nextInChain = nullptr;
        ubDesc.size = sizeof(ClearUniforms);
        ubDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
        ubDesc.mappedAtCreation = false;
        WGPUBuffer clearUniformBuf =
            wgpuDeviceCreateBuffer(device, &ubDesc);
        wgpuQueueWriteBuffer(
            queue, clearUniformBuf, 0, &clearParams, sizeof(ClearUniforms));

        WGPUBindGroupEntry bgEntries[3] = {};
        bgEntries[0].binding = 0;
        bgEntries[0].buffer = m_colorStorageBuf;
        bgEntries[0].offset = 0;
        bgEntries[0].size = numPixels * sizeof(float) * 4;

        bgEntries[1].binding = 1;
        bgEntries[1].buffer = m_depthStorageBuf;
        bgEntries[1].offset = 0;
        bgEntries[1].size = numPixels * sizeof(uint32_t);

        bgEntries[2].binding = 2;
        bgEntries[2].buffer = clearUniformBuf;
        bgEntries[2].offset = 0;
        bgEntries[2].size = sizeof(ClearUniforms);

        WGPUBindGroupDescriptor bgDesc{};
        bgDesc.nextInChain = nullptr;
        bgDesc.layout = m_clearBGL;
        bgDesc.entryCount = 3;
        bgDesc.entries = bgEntries;
        WGPUBindGroup clearBG =
            wgpuDeviceCreateBindGroup(device, &bgDesc);

        WGPUComputePassDescriptor cpDesc{};
        cpDesc.nextInChain = nullptr;
        cpDesc.label = WGPUStringView{
            "anari_clear_pass", strlen("anari_clear_pass")};
        cpDesc.timestampWrites = nullptr;
        WGPUComputePassEncoder clearPass =
            wgpuCommandEncoderBeginComputePass(encoder, &cpDesc);
        wgpuComputePassEncoderSetPipeline(clearPass, m_clearPipeline);
        wgpuComputePassEncoderSetBindGroup(clearPass, 0, clearBG, 0, nullptr);
        uint32_t clearWorkgroups = (numPixels + 255) / 256;
        wgpuComputePassEncoderDispatchWorkgroups(
            clearPass, clearWorkgroups, 1, 1);
        wgpuComputePassEncoderEnd(clearPass);
        wgpuComputePassEncoderRelease(clearPass);

        wgpuBindGroupRelease(clearBG);
        wgpuBufferRelease(clearUniformBuf);
      }

      // ==== Pass 2: Rasterize each surface via compute ====
      {
        auto *cam = dynamic_cast<PerspectiveCamera *>(m_camera.ptr);
        float aspect = (float)m_size.x / (float)m_size.y;
        float fovy = cam ? cam->fovy() : (float)(M_PI / 3.0);
        float3 eye = cam ? cam->position() : float3(0, 0, 0);
        float3 dir = cam ? cam->direction() : float3(0, 0, -1);
        float3 up = cam ? cam->up() : float3(0, 1, 0);

        // View matrix
        float3 f = linalg::normalize(dir);
        float3 s = linalg::normalize(linalg::cross(f, up));
        float3 u = linalg::cross(s, f);

        mat4 view;
        view[0] = {s.x, u.x, -f.x, 0.f};
        view[1] = {s.y, u.y, -f.y, 0.f};
        view[2] = {s.z, u.z, -f.z, 0.f};
        view[3] = {-linalg::dot(s, eye),
            -linalg::dot(u, eye),
            linalg::dot(f, eye),
            1.f};

        // Projection matrix
        float tanHalfFovy = std::tan(fovy / 2.f);
        float near = 0.1f;
        float far = 1000.f;
        mat4 proj;
        proj[0] = {1.f / (aspect * tanHalfFovy), 0, 0, 0};
        proj[1] = {0, 1.f / tanHalfFovy, 0, 0};
        proj[2] = {0, 0, far / (near - far), -1.f};
        proj[3] = {0, 0, (far * near) / (near - far), 0};

        mat4 vp = linalg::mul(proj, view);

        // Gather light direction
        float3 lightDir = float3(0.f, -1.f, -1.f);
        float ambientRad = m_renderer ? m_renderer->ambientRadiance() : 0.3f;
        for (auto *inst : m_world->instances()) {
          if (!inst || !inst->group())
            continue;
          for (auto *light : inst->group()->lights()) {
            auto *dl = dynamic_cast<DirectionalLight *>(light);
            if (dl)
              lightDir = dl->direction();
          }
        }

        for (auto *inst : m_world->instances()) {
          if (!inst || !inst->group())
            continue;
          mat4 model = inst->xfm();
          mat4 finalMVP = linalg::mul(vp, model);

          for (auto *surf : inst->group()->surfaces()) {
            if (!surf || !surf->isValid())
              continue;
            auto *geom =
                dynamic_cast<const TriangleGeometry *>(surf->geometry());
            auto *mat =
                dynamic_cast<const MatteMaterial *>(surf->material());
            if (!geom || !geom->isValid())
              continue;

            float4 matColor =
                mat ? mat->color() : float4(0.8f, 0.8f, 0.8f, 1.f);

            uint32_t numVerts = geom->numVertices();
            uint32_t numTris = geom->numTriangles();
            const float *positions = geom->vertexPositions();
            const float *normalsPtr = geom->vertexNormals();
            const float *colorsPtr = geom->vertexColors();
            const uint32_t *indicesPtr = geom->indices();

            // Build index buffer (generate sequential if none provided)
            std::vector<uint32_t> indexData;
            if (indicesPtr) {
              indexData.assign(indicesPtr, indicesPtr + numTris * 3);
            } else {
              indexData.resize(numTris * 3);
              for (uint32_t i = 0; i < numTris * 3; i++)
                indexData[i] = i;
            }

            // Provide dummy normal/color data if not present (1 float minimum)
            float dummyNormal[3] = {0.f, 0.f, 1.f};
            float dummyColor[4] = {1.f, 1.f, 1.f, 1.f};

            // Upload geometry data as storage buffers
            auto makeStorageBuf = [&](const void *data, uint64_t size,
                                      const char *label) -> WGPUBuffer {
              WGPUBufferDescriptor desc{};
              desc.nextInChain = nullptr;
              desc.label = WGPUStringView{label, strlen(label)};
              desc.usage =
                  WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst;
              desc.size = size;
              desc.mappedAtCreation = false;
              WGPUBuffer buf = wgpuDeviceCreateBuffer(device, &desc);
              wgpuQueueWriteBuffer(queue, buf, 0, data, size);
              return buf;
            };

            uint64_t posSize = numVerts * 3 * sizeof(float);
            WGPUBuffer posBuf =
                makeStorageBuf(positions, posSize, "anari_positions");

            uint64_t normSize = normalsPtr
                ? numVerts * 3 * sizeof(float)
                : sizeof(dummyNormal);
            WGPUBuffer normBuf = makeStorageBuf(
                normalsPtr ? normalsPtr : dummyNormal,
                normSize, "anari_normals");

            uint64_t colSize = colorsPtr
                ? numVerts * 4 * sizeof(float)
                : sizeof(dummyColor);
            WGPUBuffer colBuf = makeStorageBuf(
                colorsPtr ? colorsPtr : dummyColor,
                colSize, "anari_colors");

            uint64_t idxSize = indexData.size() * sizeof(uint32_t);
            WGPUBuffer idxBuf = makeStorageBuf(
                indexData.data(), idxSize, "anari_indices");

            // Uniform buffer for rasterization params
            struct RasterUniforms {
              float mvp[16];
              float materialColor[4];
              float lightDir[3];
              float ambientRadiance;
              uint32_t width;
              uint32_t height;
              uint32_t numTriangles;
              uint32_t hasNormals;
              uint32_t hasColors;
              uint32_t _pad[3];
            } rasterParams;

            for (int c = 0; c < 4; c++)
              for (int r = 0; r < 4; r++)
                rasterParams.mvp[r * 4 + c] = finalMVP[c][r];

            rasterParams.materialColor[0] = matColor.x;
            rasterParams.materialColor[1] = matColor.y;
            rasterParams.materialColor[2] = matColor.z;
            rasterParams.materialColor[3] = matColor.w;
            rasterParams.lightDir[0] = lightDir.x;
            rasterParams.lightDir[1] = lightDir.y;
            rasterParams.lightDir[2] = lightDir.z;
            rasterParams.ambientRadiance = ambientRad;
            rasterParams.width = m_size.x;
            rasterParams.height = m_size.y;
            rasterParams.numTriangles = numTris;
            rasterParams.hasNormals = normalsPtr ? 1 : 0;
            rasterParams.hasColors = colorsPtr ? 1 : 0;

            WGPUBufferDescriptor ubDesc{};
            ubDesc.nextInChain = nullptr;
            ubDesc.size = sizeof(RasterUniforms);
            ubDesc.usage =
                WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
            ubDesc.mappedAtCreation = false;
            WGPUBuffer rasterUniformBuf =
                wgpuDeviceCreateBuffer(device, &ubDesc);
            wgpuQueueWriteBuffer(queue, rasterUniformBuf, 0,
                &rasterParams, sizeof(RasterUniforms));

            // Bind group
            WGPUBindGroupEntry entries[7] = {};
            entries[0].binding = 0;
            entries[0].buffer = m_colorStorageBuf;
            entries[0].offset = 0;
            entries[0].size = numPixels * sizeof(float) * 4;

            entries[1].binding = 1;
            entries[1].buffer = m_depthStorageBuf;
            entries[1].offset = 0;
            entries[1].size = numPixels * sizeof(uint32_t);

            entries[2].binding = 2;
            entries[2].buffer = rasterUniformBuf;
            entries[2].offset = 0;
            entries[2].size = sizeof(RasterUniforms);

            entries[3].binding = 3;
            entries[3].buffer = posBuf;
            entries[3].offset = 0;
            entries[3].size = posSize;

            entries[4].binding = 4;
            entries[4].buffer = normBuf;
            entries[4].offset = 0;
            entries[4].size = normSize;

            entries[5].binding = 5;
            entries[5].buffer = colBuf;
            entries[5].offset = 0;
            entries[5].size = colSize;

            entries[6].binding = 6;
            entries[6].buffer = idxBuf;
            entries[6].offset = 0;
            entries[6].size = idxSize;

            WGPUBindGroupDescriptor bgDesc{};
            bgDesc.nextInChain = nullptr;
            bgDesc.layout = m_rasterBGL;
            bgDesc.entryCount = 7;
            bgDesc.entries = entries;
            WGPUBindGroup rasterBG =
                wgpuDeviceCreateBindGroup(device, &bgDesc);

            // Dispatch: one thread per triangle
            WGPUComputePassDescriptor passDesc{};
            passDesc.nextInChain = nullptr;
            passDesc.label = WGPUStringView{
                "anari_raster_pass", strlen("anari_raster_pass")};
            passDesc.timestampWrites = nullptr;
            WGPUComputePassEncoder rasterPass =
                wgpuCommandEncoderBeginComputePass(encoder, &passDesc);
            wgpuComputePassEncoderSetPipeline(
                rasterPass, m_rasterPipeline);
            wgpuComputePassEncoderSetBindGroup(
                rasterPass, 0, rasterBG, 0, nullptr);
            uint32_t rasterWorkgroups = (numTris + 255) / 256;
            wgpuComputePassEncoderDispatchWorkgroups(
                rasterPass, rasterWorkgroups, 1, 1);
            wgpuComputePassEncoderEnd(rasterPass);
            wgpuComputePassEncoderRelease(rasterPass);

            // Cleanup per-surface GPU resources
            wgpuBindGroupRelease(rasterBG);
            wgpuBufferRelease(rasterUniformBuf);
            wgpuBufferRelease(posBuf);
            wgpuBufferRelease(normBuf);
            wgpuBufferRelease(colBuf);
            wgpuBufferRelease(idxBuf);
          }
        }
      }

      // ==== Copy color storage buffer to readback buffer ====
      wgpuCommandEncoderCopyBufferToBuffer(encoder,
          m_colorStorageBuf, 0,
          m_readbackBuffer, 0,
          numPixels * sizeof(float) * 4);

      WGPUCommandBufferDescriptor cbDesc{};
      cbDesc.nextInChain = nullptr;
      cbDesc.label = WGPUStringView{
          "anari_cmd_buffer", strlen("anari_cmd_buffer")};
      WGPUCommandBuffer cmdBuf =
          wgpuCommandEncoderFinish(encoder, &cbDesc);
      wgpuQueueSubmit(queue, 1, &cmdBuf);
      wgpuCommandBufferRelease(cmdBuf);
      wgpuCommandEncoderRelease(encoder);

      // ==== Map readback buffer and copy to CPU pixel buffer ====
      MapContext mapCtx;
      WGPUBufferMapCallbackInfo mapCallbackInfo{};
      mapCallbackInfo.nextInChain = nullptr;
      mapCallbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
      mapCallbackInfo.callback = bufferMapCallback;
      mapCallbackInfo.userdata1 = &mapCtx;
      mapCallbackInfo.userdata2 = nullptr;
      uint64_t readbackSize = numPixels * sizeof(float) * 4;
      wgpuBufferMapAsync(m_readbackBuffer, WGPUMapMode_Read,
          0, readbackSize, mapCallbackInfo);

      while (!mapCtx.done) {
#if defined(__EMSCRIPTEN__)
        break;
#else
        wgpuInstanceProcessEvents(state->wgpuInstance);
#endif
      }

      if (mapCtx.done && mapCtx.status == WGPUMapAsyncStatus_Success) {
        const float *mapped = (const float *)wgpuBufferGetConstMappedRange(
            m_readbackBuffer, 0, readbackSize);
        if (mapped) {
          for (uint32_t y = 0; y < m_size.y; y++) {
            for (uint32_t x = 0; x < m_size.x; x++) {
              uint32_t idx = y * m_size.x + x;
              float4 c = {mapped[idx * 4 + 0],
                  mapped[idx * 4 + 1],
                  mapped[idx * 4 + 2],
                  mapped[idx * 4 + 3]};
              writeSample(x, y, PixelSample(c));
            }
          }
        }
        wgpuBufferUnmap(m_readbackBuffer);
      }
    } else {
      softwareRasterize();
    }
  } else {
    // Fill background for software path
    auto bgc =
        m_renderer ? m_renderer->background() : float4(0.f, 0.f, 0.f, 1.f);
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
  if (!m_world || !m_camera)
    return;

  auto *cam = dynamic_cast<PerspectiveCamera *>(m_camera.ptr);
  if (!cam)
    return;

  float aspect = (float)m_size.x / (float)m_size.y;
  float fovy = cam->fovy();
  float3 eye = cam->position();
  float3 dir = cam->direction();
  float3 up = cam->up();

  // Build view matrix
  float3 f = linalg::normalize(dir);
  float3 s = linalg::normalize(linalg::cross(f, up));
  float3 u = linalg::cross(s, f);

  mat4 view;
  view[0] = {s.x, u.x, -f.x, 0.f};
  view[1] = {s.y, u.y, -f.y, 0.f};
  view[2] = {s.z, u.z, -f.z, 0.f};
  view[3] = {
      -linalg::dot(s, eye), -linalg::dot(u, eye), linalg::dot(f, eye), 1.f};

  // Build projection matrix
  float tanHalfFovy = std::tan(fovy / 2.f);
  float near = 0.1f;
  float far = 1000.f;
  mat4 proj;
  proj[0] = {1.f / (aspect * tanHalfFovy), 0, 0, 0};
  proj[1] = {0, 1.f / tanHalfFovy, 0, 0};
  proj[2] = {0, 0, far / (near - far), -1.f};
  proj[3] = {0, 0, (far * near) / (near - far), 0};

  mat4 vp = linalg::mul(proj, view);

  // Gather light direction
  float3 lightDir = linalg::normalize(float3(0.f, -1.f, -1.f));
  float ambientRad = m_renderer ? m_renderer->ambientRadiance() : 0.3f;

  for (auto *inst : m_world->instances()) {
    if (!inst || !inst->group())
      continue;
    for (auto *light : inst->group()->lights()) {
      auto *dl = dynamic_cast<DirectionalLight *>(light);
      if (dl)
        lightDir = linalg::normalize(dl->direction());
    }
  }

  // Reset depth buffer
  std::fill(m_depthBuffer.begin(), m_depthBuffer.end(),
      std::numeric_limits<float>::max());

  // Rasterize all surfaces
  for (auto *inst : m_world->instances()) {
    if (!inst || !inst->group())
      continue;
    mat4 model = inst->xfm();
    mat4 mvp = linalg::mul(vp, model);

    for (auto *surf : inst->group()->surfaces()) {
      if (!surf || !surf->isValid())
        continue;
      auto *geom = dynamic_cast<const TriangleGeometry *>(surf->geometry());
      auto *mat = dynamic_cast<const MatteMaterial *>(surf->material());
      if (!geom || !geom->isValid())
        continue;

      float4 matColor = mat ? mat->color() : float4(0.8f, 0.8f, 0.8f, 1.f);

      const float *positions = geom->vertexPositions();
      const float *normals = geom->vertexNormals();
      const float *colors = geom->vertexColors();
      const uint32_t *indices = geom->indices();
      uint32_t numTris = geom->numTriangles();

      for (uint32_t t = 0; t < numTris; t++) {
        uint32_t i0, i1, i2;
        if (indices) {
          i0 = indices[t * 3 + 0];
          i1 = indices[t * 3 + 1];
          i2 = indices[t * 3 + 2];
        } else {
          i0 = t * 3 + 0;
          i1 = t * 3 + 1;
          i2 = t * 3 + 2;
        }

        float3 p0 = {
            positions[i0 * 3], positions[i0 * 3 + 1], positions[i0 * 3 + 2]};
        float3 p1 = {
            positions[i1 * 3], positions[i1 * 3 + 1], positions[i1 * 3 + 2]};
        float3 p2 = {
            positions[i2 * 3], positions[i2 * 3 + 1], positions[i2 * 3 + 2]};

        // Face normal for lighting
        float3 faceNormal;
        if (normals) {
          float3 n0 = {
              normals[i0 * 3], normals[i0 * 3 + 1], normals[i0 * 3 + 2]};
          float3 n1 = {
              normals[i1 * 3], normals[i1 * 3 + 1], normals[i1 * 3 + 2]};
          float3 n2 = {
              normals[i2 * 3], normals[i2 * 3 + 1], normals[i2 * 3 + 2]};
          faceNormal = linalg::normalize((n0 + n1 + n2) / 3.f);
        } else {
          faceNormal = linalg::normalize(linalg::cross(p1 - p0, p2 - p0));
        }

        // Vertex colors
        float4 c0 = matColor, c1 = matColor, c2 = matColor;
        if (colors) {
          c0 = {colors[i0 * 4], colors[i0 * 4 + 1], colors[i0 * 4 + 2],
              colors[i0 * 4 + 3]};
          c1 = {colors[i1 * 4], colors[i1 * 4 + 1], colors[i1 * 4 + 2],
              colors[i1 * 4 + 3]};
          c2 = {colors[i2 * 4], colors[i2 * 4 + 1], colors[i2 * 4 + 2],
              colors[i2 * 4 + 3]};
          c0 = c0 * matColor;
          c1 = c1 * matColor;
          c2 = c2 * matColor;
        }

        // Lighting
        float NdotL = std::max(linalg::dot(faceNormal, -lightDir), 0.f);
        float lighting = std::min(NdotL + ambientRad, 1.f);

        // Transform vertices to clip space
        auto transform = [&](float3 p) -> float4 {
          return linalg::mul(mvp, float4(p.x, p.y, p.z, 1.f));
        };

        float4 clip0 = transform(p0);
        float4 clip1 = transform(p1);
        float4 clip2 = transform(p2);

        if (clip0.w <= 0 || clip1.w <= 0 || clip2.w <= 0)
          continue;

        float3 ndc0 = float3(
            clip0.x / clip0.w, clip0.y / clip0.w, clip0.z / clip0.w);
        float3 ndc1 = float3(
            clip1.x / clip1.w, clip1.y / clip1.w, clip1.z / clip1.w);
        float3 ndc2 = float3(
            clip2.x / clip2.w, clip2.y / clip2.w, clip2.z / clip2.w);

        auto ndcToScreen = [&](float3 ndc) -> float3 {
          return {(ndc.x * 0.5f + 0.5f) * m_size.x,
              (1.f - (ndc.y * 0.5f + 0.5f)) * m_size.y, ndc.z};
        };

        float3 scr0 = ndcToScreen(ndc0);
        float3 scr1 = ndcToScreen(ndc1);
        float3 scr2 = ndcToScreen(ndc2);

        float minX = std::max(0.f, std::min({scr0.x, scr1.x, scr2.x}));
        float maxX = std::min(
            (float)(m_size.x - 1), std::max({scr0.x, scr1.x, scr2.x}));
        float minY = std::max(0.f, std::min({scr0.y, scr1.y, scr2.y}));
        float maxY = std::min(
            (float)(m_size.y - 1), std::max({scr0.y, scr1.y, scr2.y}));

        auto edgeFunc = [](float3 a, float3 b, float2 c) -> float {
          return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
        };

        float area = edgeFunc(scr0, scr1, float2(scr2.x, scr2.y));
        if (std::abs(area) < 1e-6f)
          continue;

        for (int py = (int)minY; py <= (int)maxY; py++) {
          for (int px = (int)minX; px <= (int)maxX; px++) {
            float2 p = {px + 0.5f, py + 0.5f};
            float w0 = edgeFunc(scr1, scr2, p) / area;
            float w1 = edgeFunc(scr2, scr0, p) / area;
            float w2 = edgeFunc(scr0, scr1, p) / area;

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
              float depth = w0 * scr0.z + w1 * scr1.z + w2 * scr2.z;
              uint32_t idx = py * m_size.x + px;
              if (depth < m_depthBuffer[idx]) {
                m_depthBuffer[idx] = depth;
                float4 color = c0 * w0 + c1 * w1 + c2 * w2;
                color.x *= lighting;
                color.y *= lighting;
                color.z *= lighting;
                writeSample(px, py, PixelSample(color, depth));
              }
            }
          }
        }
      }
    }
  }
}

void *Frame::map(std::string_view channel,
    uint32_t *width,
    uint32_t *height,
    ANARIDataType *pixelType)
{
  wait();

  *width = m_size.x;
  *height = m_size.y;

  if (channel == "channel.color") {
    *pixelType = m_colorType;
    return m_pixelBuffer.data();
  } else if (channel == "channel.depth" && !m_depthBuffer.empty()) {
    *pixelType = ANARI_FLOAT32;
    return m_depthBuffer.data();
  } else {
    *width = 0;
    *height = 0;
    *pixelType = ANARI_UNKNOWN;
    return nullptr;
  }
}

void Frame::unmap(std::string_view channel)
{
  // no-op
}

int Frame::frameReady(ANARIWaitMask m)
{
  if (m == ANARI_NO_WAIT)
    return ready();
  else {
    wait();
    return 1;
  }
}

void Frame::discard()
{
  // no-op
}

bool Frame::ready() const
{
  return true;
}

void Frame::wait() const
{
  // Synchronous rendering, always ready
}

void Frame::writeSample(int x, int y, const PixelSample &s)
{
  const auto idx = y * m_size.x + x;
  auto *color = m_pixelBuffer.data() + (idx * m_perPixelBytes);
  switch (m_colorType) {
  case ANARI_UFIXED8_VEC4: {
    auto c = helium::cvt_color_to_uint32(s.color);
    std::memcpy(color, &c, sizeof(c));
    break;
  }
  case ANARI_UFIXED8_RGBA_SRGB: {
    auto c = helium::cvt_color_to_uint32_srgb(s.color);
    std::memcpy(color, &c, sizeof(c));
    break;
  }
  case ANARI_FLOAT32_VEC4: {
    std::memcpy(color, &s.color, sizeof(s.color));
    break;
  }
  default:
    break;
  }
  if (!m_depthBuffer.empty())
    m_depthBuffer[idx] = s.depth;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Frame *);

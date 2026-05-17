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
  depthBuf[idx] = 1.0;
}
)";

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

fn depthToU32(d : f32) -> u32 {
  return bitcast<u32>(clamp(d, 0.0, 1.0));
}
fn getPosition(idx : u32) -> vec3<f32> {
  return vec3<f32>(positions[idx*3u], positions[idx*3u+1u], positions[idx*3u+2u]);
}
fn getNormal(idx : u32) -> vec3<f32> {
  return vec3<f32>(normals[idx*3u], normals[idx*3u+1u], normals[idx*3u+2u]);
}
fn getColor(idx : u32) -> vec4<f32> {
  return vec4<f32>(colors[idx*4u], colors[idx*4u+1u], colors[idx*4u+2u], colors[idx*4u+3u]);
}
fn edgeFunction(a : vec2<f32>, b : vec2<f32>, c : vec2<f32>) -> f32 {
  return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

@compute @workgroup_size(256)
fn main(@builtin(global_invocation_id) gid : vec3<u32>) {
  let triIdx = gid.x;
  if (triIdx >= params.numTriangles) { return; }

  let i0 = indices[triIdx*3u];
  let i1 = indices[triIdx*3u+1u];
  let i2 = indices[triIdx*3u+2u];

  let p0 = getPosition(i0); let p1 = getPosition(i1); let p2 = getPosition(i2);
  let clip0 = params.mvp * vec4<f32>(p0, 1.0);
  let clip1 = params.mvp * vec4<f32>(p1, 1.0);
  let clip2 = params.mvp * vec4<f32>(p2, 1.0);
  if (clip0.w <= 0.0 || clip1.w <= 0.0 || clip2.w <= 0.0) { return; }

  let ndc0 = clip0.xyz / clip0.w;
  let ndc1 = clip1.xyz / clip1.w;
  let ndc2 = clip2.xyz / clip2.w;

  let w = f32(params.width); let h = f32(params.height);
  let scr0 = vec3<f32>((ndc0.x*0.5+0.5)*w, (1.0-(ndc0.y*0.5+0.5))*h, ndc0.z);
  let scr1 = vec3<f32>((ndc1.x*0.5+0.5)*w, (1.0-(ndc1.y*0.5+0.5))*h, ndc1.z);
  let scr2 = vec3<f32>((ndc2.x*0.5+0.5)*w, (1.0-(ndc2.y*0.5+0.5))*h, ndc2.z);

  let minX = max(0.0, min(min(scr0.x, scr1.x), scr2.x));
  let maxX = min(w-1.0, max(max(scr0.x, scr1.x), scr2.x));
  let minY = max(0.0, min(min(scr0.y, scr1.y), scr2.y));
  let maxY = min(h-1.0, max(max(scr0.y, scr1.y), scr2.y));

  let area = edgeFunction(scr0.xy, scr1.xy, scr2.xy);
  if (abs(area) < 0.000001) { return; }

  var faceNormal : vec3<f32>;
  if (params.hasNormals != 0u) {
    faceNormal = normalize((getNormal(i0) + getNormal(i1) + getNormal(i2)) / 3.0);
  } else {
    faceNormal = normalize(cross(p1 - p0, p2 - p0));
  }
  let NdotL = max(dot(faceNormal, normalize(-params.lightDir)), 0.0);
  let lighting = min(NdotL + params.ambientRadiance, 1.0);

  var c0 = params.materialColor; var c1 = params.materialColor; var c2 = params.materialColor;
  if (params.hasColors != 0u) {
    c0 = getColor(i0)*params.materialColor; c1 = getColor(i1)*params.materialColor; c2 = getColor(i2)*params.materialColor;
  }

  for (var py = i32(minY); py <= i32(maxY); py++) {
    for (var px = i32(minX); px <= i32(maxX); px++) {
      let p = vec2<f32>(f32(px)+0.5, f32(py)+0.5);
      let w0 = edgeFunction(scr1.xy, scr2.xy, p) / area;
      let w1 = edgeFunction(scr2.xy, scr0.xy, p) / area;
      let w2 = edgeFunction(scr0.xy, scr1.xy, p) / area;
      if (w0 >= 0.0 && w1 >= 0.0 && w2 >= 0.0) {
        let depth = w0*scr0.z + w1*scr1.z + w2*scr2.z;
        let depthU = depthToU32(depth);
        let pixIdx = u32(py)*params.width + u32(px);
        let prev = atomicMin(&depthBufI[pixIdx], depthU);
        if (depthU <= prev) {
          let color = c0*w0 + c1*w1 + c2*w2;
          colorBuf[pixIdx] = vec4<f32>(color.rgb * lighting, color.a);
        }
      }
    }
  }
}
)";

static const char *s_pbrShaderSource = R"(

// PBR rasterizer — Cook-Torrance BRDF with GGX NDF.
// One compute thread handles one triangle (same dispatch scheme as the
// matte raster shader above).
//
// Uniform layout (160 bytes, 16-byte aligned):
//   mvp              mat4x4   64
//   baseColor        vec4     16
//   emissive         vec4     16   (xyz=emissive, w=emissiveStrength)
//   metallicRoughness vec4    16   (x=metallic, y=roughness)
//   lightDir         vec3     12
//   ambientRadiance  f32       4   = 16
//   lightColor       vec3     12
//   lightIrradiance  f32       4   = 16
//   width            u32       4
//   height           u32       4
//   numTriangles     u32       4
//   hasNormals       u32       4   = 16
//   hasColors        u32       4
//   _pad             vec3u    12   = 16
//                            ---
//                            160 bytes total

const PI_PBR : f32 = 3.14159265358979;

struct PBRUniforms {
  mvp              : mat4x4<f32>,
  baseColor        : vec4<f32>,
  emissive         : vec4<f32>,
  metallicRoughness: vec4<f32>,
  lightDir         : vec3<f32>,
  ambientRadiance  : f32,
  lightColor       : vec3<f32>,
  lightIrradiance  : f32,
  width            : u32,
  height           : u32,
  numTriangles     : u32,
  hasNormals       : u32,
  hasColors        : u32,
  _pad             : vec3<u32>,
};

@group(0) @binding(0) var<storage, read_write> pbr_colorBuf  : array<vec4<f32>>;
@group(0) @binding(1) var<storage, read_write> pbr_depthBufI : array<atomic<u32>>;
@group(0) @binding(2) var<uniform>             pbr_params    : PBRUniforms;
@group(0) @binding(3) var<storage, read>       pbr_positions : array<f32>;
@group(0) @binding(4) var<storage, read>       pbr_normals   : array<f32>;
@group(0) @binding(5) var<storage, read>       pbr_colors    : array<f32>;
@group(0) @binding(6) var<storage, read>       pbr_indices   : array<u32>;

fn pbr_depthToU32(d: f32) -> u32 { return bitcast<u32>(clamp(d, 0.0, 1.0)); }
fn pbr_getPos(i: u32) -> vec3<f32> {
  return vec3<f32>(pbr_positions[i*3u], pbr_positions[i*3u+1u], pbr_positions[i*3u+2u]);
}
fn pbr_getNorm(i: u32) -> vec3<f32> {
  return vec3<f32>(pbr_normals[i*3u], pbr_normals[i*3u+1u], pbr_normals[i*3u+2u]);
}
fn pbr_getColor(i: u32) -> vec4<f32> {
  return vec4<f32>(pbr_colors[i*4u], pbr_colors[i*4u+1u], pbr_colors[i*4u+2u], pbr_colors[i*4u+3u]);
}
fn pbr_edge(a: vec2<f32>, b: vec2<f32>, c: vec2<f32>) -> f32 {
  return (c.x-a.x)*(b.y-a.y) - (c.y-a.y)*(b.x-a.x);
}

// GGX/Trowbridge-Reitz normal distribution function
fn D_GGX(NdotH: f32, alpha: f32) -> f32 {
  let a2 = alpha * alpha;
  let d  = NdotH * NdotH * (a2 - 1.0) + 1.0;
  return a2 / (PI_PBR * d * d + 0.0001);
}

// Schlick-GGX visibility (one direction)
fn G_SchlickGGX(NdotX: f32, roughness: f32) -> f32 {
  let r = roughness + 1.0;
  let k = (r * r) / 8.0;
  return NdotX / (NdotX * (1.0 - k) + k + 0.0001);
}

// Smith's combined geometry term
fn G_Smith(NdotV: f32, NdotL: f32, roughness: f32) -> f32 {
  return G_SchlickGGX(NdotV, roughness) * G_SchlickGGX(NdotL, roughness);
}

// Fresnel-Schlick approximation
fn F_Schlick(cosTheta: f32, F0: vec3<f32>) -> vec3<f32> {
  let t  = clamp(1.0 - cosTheta, 0.0, 1.0);
  let t5 = t * t * t * t * t;
  return F0 + (vec3<f32>(1.0) - F0) * t5;
}

@compute @workgroup_size(256)
fn pbr_main(@builtin(global_invocation_id) gid: vec3<u32>) {
  let triIdx = gid.x;
  if (triIdx >= pbr_params.numTriangles) { return; }

  let i0 = pbr_indices[triIdx*3u];
  let i1 = pbr_indices[triIdx*3u+1u];
  let i2 = pbr_indices[triIdx*3u+2u];

  let p0 = pbr_getPos(i0); let p1 = pbr_getPos(i1); let p2 = pbr_getPos(i2);

  let clip0 = pbr_params.mvp * vec4<f32>(p0, 1.0);
  let clip1 = pbr_params.mvp * vec4<f32>(p1, 1.0);
  let clip2 = pbr_params.mvp * vec4<f32>(p2, 1.0);
  if (clip0.w <= 0.0 || clip1.w <= 0.0 || clip2.w <= 0.0) { return; }

  let ndc0 = clip0.xyz / clip0.w;
  let ndc1 = clip1.xyz / clip1.w;
  let ndc2 = clip2.xyz / clip2.w;

  let W = f32(pbr_params.width); let H = f32(pbr_params.height);
  let s0 = vec3<f32>((ndc0.x*0.5+0.5)*W, (1.0-(ndc0.y*0.5+0.5))*H, ndc0.z);
  let s1 = vec3<f32>((ndc1.x*0.5+0.5)*W, (1.0-(ndc1.y*0.5+0.5))*H, ndc1.z);
  let s2 = vec3<f32>((ndc2.x*0.5+0.5)*W, (1.0-(ndc2.y*0.5+0.5))*H, ndc2.z);

  let minX = max(0.0, min(min(s0.x, s1.x), s2.x));
  let maxX = min(W-1.0, max(max(s0.x, s1.x), s2.x));
  let minY = max(0.0, min(min(s0.y, s1.y), s2.y));
  let maxY = min(H-1.0, max(max(s0.y, s1.y), s2.y));

  let area = pbr_edge(s0.xy, s1.xy, s2.xy);
  if (abs(area) < 0.000001) { return; }

  let metallic  = pbr_params.metallicRoughness.x;
  let roughness = pbr_params.metallicRoughness.y;

  // BUG: should be roughness * roughness (Disney remapping).
  // Using raw roughness makes surfaces appear less rough than intended,
  // especially noticeable in the 0.0-0.5 range.
  let alpha = roughness;

  let L  = normalize(-pbr_params.lightDir);

  for (var py = i32(minY); py <= i32(maxY); py++) {
    for (var px = i32(minX); px <= i32(maxX); px++) {
      let pp = vec2<f32>(f32(px)+0.5, f32(py)+0.5);
      let w0 = pbr_edge(s1.xy, s2.xy, pp) / area;
      let w1 = pbr_edge(s2.xy, s0.xy, pp) / area;
      let w2 = pbr_edge(s0.xy, s1.xy, pp) / area;
      if (w0 < 0.0 || w1 < 0.0 || w2 < 0.0) { continue; }

      let depth  = w0*s0.z + w1*s1.z + w2*s2.z;
      let depthU = pbr_depthToU32(depth);
      let pixIdx = u32(py)*pbr_params.width + u32(px);

      let prev = atomicMin(&pbr_depthBufI[pixIdx], depthU);
      if (depthU > prev) { continue; }

      // Interpolate normal
      var N: vec3<f32>;
      if (pbr_params.hasNormals != 0u) {
        N = normalize(pbr_getNorm(i0)*w0 + pbr_getNorm(i1)*w1 + pbr_getNorm(i2)*w2);
      } else {
        N = normalize(cross(p1-p0, p2-p0));
      }

      // Approximate view direction from NDC centroid
      let ndcC  = ndc0*w0 + ndc1*w1 + ndc2*w2;
      let V = normalize(vec3<f32>(-ndcC.x, -ndcC.y, 1.0));
      let H = normalize(V + L);

      let NdotL = max(dot(N, L), 0.0);
      let NdotV = max(dot(N, V), 0.0001);
      let NdotH = max(dot(N, H), 0.0);
      let HdotV = max(dot(H, V), 0.0);

      // Base color (constant or per-vertex modulated)
      var baseRGB = pbr_params.baseColor.rgb;
      if (pbr_params.hasColors != 0u) {
        let vc = pbr_getColor(i0)*w0 + pbr_getColor(i1)*w1 + pbr_getColor(i2)*w2;
        baseRGB = baseRGB * vc.rgb;
      }

      // F0: dielectric 0.04, metal = baseColor
      let F0 = mix(vec3<f32>(0.04), baseRGB, metallic);

      let D = D_GGX(NdotH, alpha);
      let G = G_Smith(NdotV, NdotL, roughness);
      let F = F_Schlick(HdotV, F0);

      let specular = (D * G * F) / (4.0 * NdotV * NdotL + 0.0001);
      let kD = (vec3<f32>(1.0) - F) * (1.0 - metallic);
      let diffuse = kD * baseRGB / PI_PBR;

      let irradiance = pbr_params.lightIrradiance * pbr_params.lightColor;
      var Lo = (diffuse + specular) * irradiance * NdotL;

      // Ambient (image-based lighting approximation: constant ambient * albedo)
      Lo += pbr_params.ambientRadiance * baseRGB;

      // Emissive added after lighting
      Lo += pbr_params.emissive.xyz * pbr_params.emissive.w;

      // Reinhard tone mapping + gamma
      Lo  = Lo / (Lo + vec3<f32>(1.0));
      Lo  = pow(clamp(Lo, vec3<f32>(0.0), vec3<f32>(1.0)), vec3<f32>(1.0/2.2));

      pbr_colorBuf[pixIdx] = vec4<f32>(Lo, pbr_params.baseColor.a);
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

  m_valid = m_renderer && m_camera && m_camera->isValid() && m_world && m_world->isValid();

  cleanupComputeResources();
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

static WGPUComputePipeline makePipeline(WGPUDevice device,
    WGPUShaderModule shader, WGPUBindGroupLayout bgl, const char *label,
    const char *entryPoint)
{
  WGPUPipelineLayoutDescriptor plDesc{};
  plDesc.nextInChain = nullptr;
  plDesc.bindGroupLayoutCount = 1;
  plDesc.bindGroupLayouts = &bgl;
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

static WGPUShaderModule makeWGSLModule(WGPUDevice device, const char *src,
    const char *label)
{
  WGPUShaderSourceWGSL wgslSrc{};
  wgslSrc.chain.sType = WGPUSType_ShaderSourceWGSL;
  wgslSrc.chain.next = nullptr;
  wgslSrc.code = WGPUStringView{src, strlen(src)};

  WGPUShaderModuleDescriptor smDesc{};
  smDesc.nextInChain = &wgslSrc.chain;
  smDesc.label = WGPUStringView{label, strlen(label)};
  return wgpuDeviceCreateShaderModule(device, &smDesc);
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
    m_clearShader = makeWGSLModule(device, s_clearShaderSource, "anari_clear_shader");

    WGPUBindGroupLayoutEntry entries[3] = {
        storageEntry(0, false), storageEntry(1, false), uniformEntry(2)};
    m_clearBGL = makeBGL(device, entries, 3);
    m_clearPipeline = makePipeline(device, m_clearShader, m_clearBGL,
        "anari_clear_pipeline", "main");
  }

  {
    m_rasterShader = makeWGSLModule(device, s_rasterShaderSource, "anari_raster_shader");

    WGPUBindGroupLayoutEntry entries[7] = {
        storageEntry(0, false), storageEntry(1, false), uniformEntry(2),
        storageEntry(3, true), storageEntry(4, true),
        storageEntry(5, true), storageEntry(6, true)};
    m_rasterBGL = makeBGL(device, entries, 7);
    m_rasterPipeline = makePipeline(device, m_rasterShader, m_rasterBGL,
        "anari_raster_pipeline", "main");
  }

  {
    m_pbrShader = makeWGSLModule(device, s_pbrShaderSource, "anari_pbr_shader");

    WGPUBindGroupLayoutEntry entries[7] = {
        storageEntry(0, false), storageEntry(1, false), uniformEntry(2),
        storageEntry(3, true), storageEntry(4, true),
        storageEntry(5, true), storageEntry(6, true)};
    m_pbrBGL = makeBGL(device, entries, 7);
    m_pbrPipeline = makePipeline(device, m_pbrShader, m_pbrBGL,
        "anari_pbr_pipeline", "pbr_main");
  }

  m_gpuResourcesInitialized = true;
}

void Frame::cleanupComputeResources()
{
  auto rel = [](auto &h, auto fn) { if (h) { fn(h); h = nullptr; } };

  rel(m_readbackBuffer,  wgpuBufferRelease);
  rel(m_colorStorageBuf, wgpuBufferRelease);
  rel(m_depthStorageBuf, wgpuBufferRelease);

  rel(m_clearPipeline, wgpuComputePipelineRelease);
  rel(m_clearShader,   wgpuShaderModuleRelease);
  rel(m_clearBGL,      wgpuBindGroupLayoutRelease);

  rel(m_rasterPipeline, wgpuComputePipelineRelease);
  rel(m_rasterShader,   wgpuShaderModuleRelease);
  rel(m_rasterBGL,      wgpuBindGroupLayoutRelease);

  rel(m_pbrPipeline, wgpuComputePipelineRelease);
  rel(m_pbrShader,   wgpuShaderModuleRelease);
  rel(m_pbrBGL,      wgpuBindGroupLayoutRelease);

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

  if (!m_valid || !m_world)
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
                uint32_t hasColors; uint32_t _pad[3];
              } pu;

              for (int c=0;c<4;c++) for (int r=0;r<4;r++) pu.mvp[r*4+c]=mvp[c][r];
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

              WGPUBuffer ub = uploadUniform(device, queue, &pu, sizeof(pu));

              WGPUBindGroup bg = makeBindGroup7(device, m_pbrBGL,
                  m_colorStorageBuf, numPixels*16ULL,
                  m_depthStorageBuf, numPixels*4ULL,
                  ub, sizeof(pu),
                  posBuf, posSize, normBuf, normSize, colBuf, colSize, idxBuf, idxSize);

              WGPUComputePassDescriptor passDesc{};
              passDesc.nextInChain = nullptr;
              passDesc.label = WGPUStringView{"anari_pbr_pass", 14};
              passDesc.timestampWrites = nullptr;
              WGPUComputePassEncoder pass =
                  wgpuCommandEncoderBeginComputePass(encoder, &passDesc);
              wgpuComputePassEncoderSetPipeline(pass, m_pbrPipeline);
              wgpuComputePassEncoderSetBindGroup(pass, 0, bg, 0, nullptr);
              wgpuComputePassEncoderDispatchWorkgroups(
                  pass, (numTris + 255) / 256, 1, 1);
              wgpuComputePassEncoderEnd(pass);
              wgpuComputePassEncoderRelease(pass);
              wgpuBindGroupRelease(bg);
              wgpuBufferRelease(ub);

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

              for (int c=0;c<4;c++) for (int r=0;r<4;r++) ru.mvp[r*4+c]=mvp[c][r];
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

              WGPUComputePassDescriptor passDesc{};
              passDesc.nextInChain = nullptr;
              passDesc.label = WGPUStringView{"anari_raster_pass", 17};
              passDesc.timestampWrites = nullptr;
              WGPUComputePassEncoder pass =
                  wgpuCommandEncoderBeginComputePass(encoder, &passDesc);
              wgpuComputePassEncoderSetPipeline(pass, m_rasterPipeline);
              wgpuComputePassEncoderSetBindGroup(pass, 0, bg, 0, nullptr);
              wgpuComputePassEncoderDispatchWorkgroups(
                  pass, (numTris + 255) / 256, 1, 1);
              wgpuComputePassEncoderEnd(pass);
              wgpuComputePassEncoderRelease(pass);
              wgpuBindGroupRelease(bg);
              wgpuBufferRelease(ub);
            }

            wgpuBufferRelease(posBuf);
            wgpuBufferRelease(normBuf);
            wgpuBufferRelease(colBuf);
            wgpuBufferRelease(idxBuf);
          }
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
  if (!m_world || !m_camera)
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

  std::fill(m_depthBuffer.begin(), m_depthBuffer.end(),
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
            if (depth >= m_depthBuffer[idx]) continue;
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

              float3 em = pbrMat->evalEmissive(uv);
              Lo = float3(Lo.x+em.x, Lo.y+em.y, Lo.z+em.z);

              auto rh = [](float c){ return c/(c+1.f); };
              auto gm = [](float c){ return std::pow(std::max(c,0.f), 1.f/2.2f); };
              fragColor = float4(gm(rh(Lo.x)), gm(rh(Lo.y)), gm(rh(Lo.z)), bc.w);

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

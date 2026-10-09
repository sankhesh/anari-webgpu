// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

import anari_raster_common;
import vtkslang_surface;

type_param M : IMaterial;
type_param L : ILightingModel;
type_param N : INormalModifier;
type_param O : ISurfaceOutput;
type_param V : IVertexModifier;
type_param H : IShaderHooks;
typealias Default_IMaterial       = HostDrivenMaterial;
typealias Default_ILightingModel  = CookTorranceLighting;
typealias Default_INormalModifier = GeometricNormal;
typealias Default_ISurfaceOutput  = StandardOutput;
typealias Default_IVertexModifier = IdentityVertex;
typealias Default_IShaderHooks    = DefaultHooks;

struct PBRUniforms
{
    float4x4 mvp;
    float4   baseColor;
    float4   emissive;
    float4   metallicRoughness;
    float3   lightDir;
    float    ambientRadiance;
    float3   lightColor;
    float    lightIrradiance;
    uint     width;
    uint     height;
    uint     numTriangles;
    uint     hasNormals;
    uint     hasColors;
    uint     alphaMode;
    float    alphaCutoff;
    uint     _pad;
};

[vk::binding(0, 0)] RWStructuredBuffer<float4>    colorBuf;
[vk::binding(1, 0)] RWStructuredBuffer<Atomic<uint>> depthBufI;
[vk::binding(2, 0)] ConstantBuffer<PBRUniforms>   params;
[vk::binding(3, 0)] StructuredBuffer<float>        positions;
[vk::binding(4, 0)] StructuredBuffer<float>        normals;
[vk::binding(5, 0)] StructuredBuffer<float>        colors;
[vk::binding(6, 0)] StructuredBuffer<uint>         indices;
ParameterBlock<Shading<M, L, N, O, V>>             shading;
ParameterBlock<HookBlock<H>>                       hookBlock;

float3 pbr_getPos(uint i)   { return float3(positions[i*3u], positions[i*3u+1u], positions[i*3u+2u]); }
float3 pbr_getNorm(uint i)  { return float3(normals[i*3u],   normals[i*3u+1u],   normals[i*3u+2u]);   }
float4 pbr_getColor(uint i) { return float4(colors[i*4u],    colors[i*4u+1u],    colors[i*4u+2u],    colors[i*4u+3u]); }

uint3 pbr_triangleIndices(uint triIdx)
{
    return uint3(indices[triIdx*3u], indices[triIdx*3u+1u], indices[triIdx*3u+2u]);
}

VertexSample pbr_fetchVertex(uint triIdx, uint idx, out VertexContext c)
{
    VertexSample v = VertexSample();
    v.position    = pbr_getPos(idx);
    if (params.hasNormals != 0u)
        v.normal  = pbr_getNorm(idx);
    if (params.hasColors != 0u)
        v.color   = pbr_getColor(idx);
    v.pointId     = idx;
    v.cellId      = triIdx;
    v.primitiveId = triIdx;
    v.modelToClip = params.mvp;
    return shadeVertex(shading, hookBlock.hooks, v, c);
}

struct PBRTriangle
{
    VertexSample   v0, v1, v2;
    ScreenTriangle t;
    uint           id;
};

bool pbr_setupTriangle(uint triIdx, out PBRTriangle r)
{
    r = PBRTriangle();
    r.id = triIdx;
    uint3 i = pbr_triangleIndices(triIdx);
    VertexContext c0, c1, c2;
    r.v0 = pbr_fetchVertex(triIdx, i.x, c0);
    r.v1 = pbr_fetchVertex(triIdx, i.y, c1);
    r.v2 = pbr_fetchVertex(triIdx, i.z, c2);
    return setupScreenTriangleClip(projectVertex(hookBlock.hooks, c0, mul(params.mvp, c0.positionMC)),
                                   projectVertex(hookBlock.hooks, c1, mul(params.mvp, c1.positionMC)),
                                   projectVertex(hookBlock.hooks, c2, mul(params.mvp, c2.positionMC)),
                                   params.width, params.height, r.t);
}

FragmentContext pbr_fragment(PBRTriangle r, float3 w, int px, int py, float depth)
{
    FragmentContext f = FragmentContext();
    f.positionVC = float4(r.v0.position*w.x + r.v1.position*w.y + r.v2.position*w.z, 1.f);
    if (params.hasNormals != 0u)
        f.normalVC = normalize(r.v0.normal*w.x + r.v1.normal*w.y + r.v2.normal*w.z);
    else
        f.normalVC = normalize(cross(r.v1.position - r.v0.position, r.v2.position - r.v0.position));
    f.uv          = r.v0.uv*w.x + r.v1.uv*w.y + r.v2.uv*w.z;
    f.fragCoord   = float4(float(px) + 0.5f, float(py) + 0.5f, depth, 1.f);
    f.depth       = depth;
    f.primitiveId = r.id;
    f.cellId      = r.id;
    f.frontFacing = r.t.area < 0.f;
    return f;
}

struct PBRSource : ISurfaceSource
{
    PBRTriangle r;
    float3      w;

    void colors(inout FragmentContext f)
    {
        float3 baseRGB = params.baseColor.rgb;
        if (params.hasColors != 0u) {
            float4 vc = r.v0.color*w.x + r.v1.color*w.y + r.v2.color*w.z;
            baseRGB = baseRGB * vc.rgb;
        }
        f.diffuseColor = baseRGB;
        f.opacity      = params.baseColor.a;
    }
    SurfaceSample sample(FragmentContext f)
    {
        SurfaceSample s = surfaceSample(f);
        float3 ndcC = r.t.ndc0*w.x + r.t.ndc1*w.y + r.t.ndc2*w.z;
        s.viewDir   = normalize(float3(-ndcC.x, -ndcC.y, 1.f));
        s.camera.shadingToClip = params.mvp;
        s.host.metallic  = params.metallicRoughness.x;
        s.host.roughness = params.metallicRoughness.y;
        s.host.emissive  = params.emissive.xyz * params.emissive.w;
        return s;
    }
    uint lightCount(SurfaceSample s) { return 1u; }
    LightSample light(uint i, SurfaceSample s)
    {
        LightSample light = LightSample();
        light.direction = normalize(-params.lightDir);
        light.radiance  = params.lightIrradiance * params.lightColor;
        return light;
    }
    float3 ambientRadiance(SurfaceSample s) { return float3(params.ambientRadiance); }
    override bool opacity(MaterialSample m, inout float4 color)
    {
        if (params.alphaMode == 2u) {
            if (m.opacity < params.alphaCutoff)
                return false;
            color.a = 1.f;
        } else if (params.alphaMode != 1u) {
            color.a = 1.f;
        }
        return true;
    }
};

bool pbr_fragmentDepth(PBRSource source, inout FragmentContext f, out SurfaceSample s,
                       out float depth)
{
    depth = f.depth;
    if (!surfaceCoverage(shading, hookBlock.hooks, source, f, s))
        return false;
    depth = outputDepth(hookBlock.hooks, f);
    return true;
}

[shader("compute")]
[numthreads(256, 1, 1)]
void pbr_depth_main(uint3 gid : SV_DispatchThreadID)
{
    uint triIdx = gid.x;
    if (triIdx >= params.numTriangles)
        return;

    PBRTriangle r;
    if (!pbr_setupTriangle(triIdx, r))
        return;

    for (int py = int(r.t.minY); py <= int(r.t.maxY); py++) {
        for (int px = int(r.t.minX); px <= int(r.t.maxX); px++) {
            float3 w = barycentric(r.t, px, py);
            if (!covered(w))
                continue;
            PBRSource       source = { r, w };
            FragmentContext f      = pbr_fragment(r, w, px, py, interpolateDepth(r.t, w));
            SurfaceSample   s;
            float           depth;
            if (!pbr_fragmentDepth(source, f, s, depth))
                continue;
            uint pixIdx = uint(py)*params.width + uint(px);
            depthBufI[pixIdx].min(depthToU32(depth));
        }
    }
}

[shader("compute")]
[numthreads(256, 1, 1)]
void pbr_main(uint3 gid : SV_DispatchThreadID)
{
    uint triIdx = gid.x;
    if (triIdx >= params.numTriangles)
        return;

    PBRTriangle r;
    if (!pbr_setupTriangle(triIdx, r))
        return;

    for (int py = int(r.t.minY); py <= int(r.t.maxY); py++) {
        for (int px = int(r.t.minX); px <= int(r.t.maxX); px++) {
            float3 w = barycentric(r.t, px, py);
            if (!covered(w))
                continue;
            PBRSource       source = { r, w };
            FragmentContext f      = pbr_fragment(r, w, px, py, interpolateDepth(r.t, w));
            SurfaceSample   s;
            float           depth;
            if (!pbr_fragmentDepth(source, f, s, depth))
                continue;
            uint pixIdx = uint(py)*params.width + uint(px);
            if (depthToU32(depth) > depthBufI[pixIdx].load() + kDepthTolerance)
                continue;
            if (!surfaceShade(shading, hookBlock.hooks, source, f, s))
                continue;
            colorBuf[pixIdx] = f.color;
        }
    }
}

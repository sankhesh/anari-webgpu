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
typealias Default_ILightingModel  = LambertLighting;
typealias Default_INormalModifier = GeometricNormal;
typealias Default_ISurfaceOutput  = StandardOutput;
typealias Default_IVertexModifier = IdentityVertex;
typealias Default_IShaderHooks    = DefaultHooks;

struct RasterUniforms
{
    float4x4 mvp;
    float4   materialColor;
    float3   lightDir;
    float    ambientRadiance;
    uint     width;
    uint     height;
    uint     numTriangles;
    uint     hasNormals;
    uint     hasColors;
    uint     _pad0;
    uint     _pad1;
    uint     _pad2;
};

[vk::binding(0, 0)] RWStructuredBuffer<float4>       colorBuf;
[vk::binding(1, 0)] RWStructuredBuffer<Atomic<uint>> depthBufI;
[vk::binding(2, 0)] ConstantBuffer<RasterUniforms>   params;
[vk::binding(3, 0)] StructuredBuffer<float>          positions;
[vk::binding(4, 0)] StructuredBuffer<float>          normals;
[vk::binding(5, 0)] StructuredBuffer<float>          colors;
[vk::binding(6, 0)] StructuredBuffer<uint>           indices;
ParameterBlock<Shading<M, L, N, O, V>>               shading;
ParameterBlock<HookBlock<H>>                         hookBlock;

float3 getPosition(uint i) { return float3(positions[i*3u], positions[i*3u+1u], positions[i*3u+2u]); }
float3 getNormal(uint i)   { return float3(normals[i*3u],   normals[i*3u+1u],   normals[i*3u+2u]);   }
float4 getColor(uint i)    { return float4(colors[i*4u],    colors[i*4u+1u],    colors[i*4u+2u],    colors[i*4u+3u]); }

uint3 triangleIndices(uint triIdx)
{
    return uint3(indices[triIdx*3u], indices[triIdx*3u+1u], indices[triIdx*3u+2u]);
}

VertexSample fetchVertex(uint triIdx, uint idx, out VertexContext c)
{
    VertexSample v = VertexSample();
    v.position    = getPosition(idx);
    if (params.hasNormals != 0u)
        v.normal  = getNormal(idx);
    if (params.hasColors != 0u)
        v.color   = getColor(idx);
    v.pointId     = idx;
    v.cellId      = triIdx;
    v.primitiveId = triIdx;
    v.modelToClip = params.mvp;
    return shadeVertex(shading, hookBlock.hooks, v, c);
}

struct Triangle
{
    VertexSample   v0, v1, v2;
    ScreenTriangle t;
    float3         faceNormal;
    uint           id;
};

bool setupTriangle(uint triIdx, out Triangle r)
{
    r = Triangle();
    r.id = triIdx;
    uint3 i = triangleIndices(triIdx);
    VertexContext c0, c1, c2;
    r.v0 = fetchVertex(triIdx, i.x, c0);
    r.v1 = fetchVertex(triIdx, i.y, c1);
    r.v2 = fetchVertex(triIdx, i.z, c2);
    if (!setupScreenTriangleClip(projectVertex(hookBlock.hooks, c0, mul(params.mvp, c0.positionMC)),
                                 projectVertex(hookBlock.hooks, c1, mul(params.mvp, c1.positionMC)),
                                 projectVertex(hookBlock.hooks, c2, mul(params.mvp, c2.positionMC)),
                                 params.width, params.height, r.t))
        return false;
    if (params.hasNormals != 0u)
        r.faceNormal = normalize((r.v0.normal + r.v1.normal + r.v2.normal) / 3.f);
    else
        r.faceNormal = normalize(cross(r.v1.position - r.v0.position, r.v2.position - r.v0.position));
    return true;
}

FragmentContext rasterFragment(Triangle r, float3 w, int px, int py, float depth)
{
    FragmentContext f = FragmentContext();
    f.positionVC  = float4(r.v0.position*w.x + r.v1.position*w.y + r.v2.position*w.z, 1.f);
    f.normalVC    = r.faceNormal;
    f.uv          = r.v0.uv*w.x + r.v1.uv*w.y + r.v2.uv*w.z;
    f.fragCoord   = float4(float(px) + 0.5f, float(py) + 0.5f, depth, 1.f);
    f.depth       = depth;
    f.primitiveId = r.id;
    f.cellId      = r.id;
    f.frontFacing = r.t.area < 0.f;
    return f;
}

struct RasterSource : ISurfaceSource
{
    Triangle r;
    float3   w;

    void colors(inout FragmentContext f)
    {
        float4 color = (r.v0.color * params.materialColor)*w.x + (r.v1.color * params.materialColor)*w.y
                     + (r.v2.color * params.materialColor)*w.z;
        f.diffuseColor = color.rgb;
        f.opacity      = color.a;
    }
    SurfaceSample sample(FragmentContext f)
    {
        SurfaceSample s = surfaceSample(f);
        s.viewDir = float3(0.f, 0.f, 1.f);
        s.camera.shadingToClip = params.mvp;
        return s;
    }
    uint lightCount(SurfaceSample s) { return 1u; }
    LightSample light(uint i, SurfaceSample s)
    {
        LightSample light = LightSample();
        light.direction = normalize(-params.lightDir);
        light.radiance  = float3(1.f);
        return light;
    }
    float3 ambientRadiance(SurfaceSample s) { return float3(params.ambientRadiance); }
};

bool fragmentDepth(RasterSource source, inout FragmentContext f, out SurfaceSample s,
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
void depth_main(uint3 gid : SV_DispatchThreadID)
{
    uint triIdx = gid.x;
    if (triIdx >= params.numTriangles)
        return;

    Triangle r;
    if (!setupTriangle(triIdx, r))
        return;

    for (int py = int(r.t.minY); py <= int(r.t.maxY); py++) {
        for (int px = int(r.t.minX); px <= int(r.t.maxX); px++) {
            float3 w = barycentric(r.t, px, py);
            if (!covered(w))
                continue;
            RasterSource    source = { r, w };
            FragmentContext f      = rasterFragment(r, w, px, py, interpolateDepth(r.t, w));
            SurfaceSample   s;
            float           depth;
            if (!fragmentDepth(source, f, s, depth))
                continue;
            uint pixIdx = uint(py)*params.width + uint(px);
            depthBufI[pixIdx].min(depthToU32(depth));
        }
    }
}

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 gid : SV_DispatchThreadID)
{
    uint triIdx = gid.x;
    if (triIdx >= params.numTriangles)
        return;

    Triangle r;
    if (!setupTriangle(triIdx, r))
        return;

    for (int py = int(r.t.minY); py <= int(r.t.maxY); py++) {
        for (int px = int(r.t.minX); px <= int(r.t.maxX); px++) {
            float3 w = barycentric(r.t, px, py);
            if (!covered(w))
                continue;
            RasterSource    source = { r, w };
            FragmentContext f      = rasterFragment(r, w, px, py, interpolateDepth(r.t, w));
            SurfaceSample   s;
            float           depth;
            if (!fragmentDepth(source, f, s, depth))
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

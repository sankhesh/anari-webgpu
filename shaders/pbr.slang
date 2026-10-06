// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

import anari_raster_common;
import vtkslang_shading;
import vtkslang_hooks;

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

VertexContext pbr_vertexContext(VertexSample v)
{
    VertexContext c = VertexContext();
    c.positionMC  = float4(v.position, 1.f);
    c.normalMC    = v.normal;
    c.tangentMC   = v.tangent;
    c.uv          = v.uv;
    c.color       = v.color;
    c.pointId     = v.pointId;
    c.cellId      = v.cellId;
    c.primitiveId = v.primitiveId;
    c.modelToClip = params.mvp;
    return c;
}

VertexSample pbr_fetchVertex(uint triIdx, uint idx)
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
    v = shading.vertex.modify(v);
    VertexContext c = pbr_vertexContext(v);
    hookBlock.hooks.vertex(c);
    v.position = c.positionMC.xyz;
    v.normal   = c.normalMC;
    v.tangent  = c.tangentMC;
    v.uv       = c.uv;
    v.color    = c.color;
    return v;
}

float4 pbr_project(VertexSample v)
{
    VertexContext c = pbr_vertexContext(v);
    c.positionClip  = mul(params.mvp, c.positionMC);
    hookBlock.hooks.position(c);
    return c.positionClip;
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
    r.v0 = pbr_fetchVertex(triIdx, i.x);
    r.v1 = pbr_fetchVertex(triIdx, i.y);
    r.v2 = pbr_fetchVertex(triIdx, i.z);
    return setupScreenTriangleClip(pbr_project(r.v0), pbr_project(r.v1), pbr_project(r.v2),
                                   params.width, params.height, r.t);
}

SurfaceSample pbr_surfaceSample(PBRTriangle r, float3 w, int px, int py, float depth)
{
    SurfaceSample s = SurfaceSample();
    s.position = r.v0.position*w.x + r.v1.position*w.y + r.v2.position*w.z;
    if (params.hasNormals != 0u)
        s.normal = normalize(r.v0.normal*w.x + r.v1.normal*w.y + r.v2.normal*w.z);
    else
        s.normal = normalize(cross(r.v1.position - r.v0.position, r.v2.position - r.v0.position));
    float3 ndcC   = r.t.ndc0*w.x + r.t.ndc1*w.y + r.t.ndc2*w.z;
    s.viewDir     = normalize(float3(-ndcC.x, -ndcC.y, 1.f));
    s.uv          = r.v0.uv*w.x + r.v1.uv*w.y + r.v2.uv*w.z;
    s.fragCoord   = float4(float(px) + 0.5f, float(py) + 0.5f, depth, 1.f);
    s.primitiveId = r.id;
    s.cellId      = r.id;
    s.frontFacing = r.t.area < 0.f;
    s.camera.shadingToClip = params.mvp;
    return s;
}

FragmentContext pbr_fragmentContext(SurfaceSample s)
{
    FragmentContext f = FragmentContext();
    f.positionVC  = float4(s.position, 1.f);
    f.normalVC    = s.normal;
    f.tangentVC   = s.tangent;
    f.uv          = s.uv;
    f.fragCoord   = s.fragCoord;
    f.primitiveId = s.primitiveId;
    f.cellId      = s.cellId;
    f.frontFacing = s.frontFacing;
    f.depth       = s.fragCoord.z;
    return f;
}

bool pbr_fragmentDepth(SurfaceSample s, float z, out float depth)
{
    FragmentContext f = pbr_fragmentContext(s);
    depth = z;
    if (!hookBlock.hooks.clip(f) || !shading.output.keep(s))
        return false;
    f.depth = shading.output.depth(z, s);
    hookBlock.hooks.output(f);
    depth = f.depth;
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
            float         z = interpolateDepth(r.t, w);
            SurfaceSample s = pbr_surfaceSample(r, w, px, py, z);
            float         depth;
            if (!pbr_fragmentDepth(s, z, depth))
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

    LightSample light = LightSample();
    light.direction = normalize(-params.lightDir);
    light.radiance  = params.lightIrradiance * params.lightColor;

    for (int py = int(r.t.minY); py <= int(r.t.maxY); py++) {
        for (int px = int(r.t.minX); px <= int(r.t.maxX); px++) {
            float3 w = barycentric(r.t, px, py);
            if (!covered(w))
                continue;

            float         z = interpolateDepth(r.t, w);
            SurfaceSample s = pbr_surfaceSample(r, w, px, py, z);
            float         depth;
            if (!pbr_fragmentDepth(s, z, depth))
                continue;
            uint pixIdx = uint(py)*params.width + uint(px);
            if (depthToU32(depth) > depthBufI[pixIdx].load() + kDepthTolerance)
                continue;

            float3 baseRGB = params.baseColor.rgb;
            if (params.hasColors != 0u) {
                float4 vc = r.v0.color*w.x + r.v1.color*w.y + r.v2.color*w.z;
                baseRGB = baseRGB * vc.rgb;
            }

            FragmentContext f = pbr_fragmentContext(s);
            f.diffuseColor = baseRGB;
            f.opacity      = params.baseColor.a;
            hookBlock.hooks.color(f);
            hookBlock.hooks.normal(f);
            s.host.baseColor = f.diffuseColor;
            s.host.opacity   = f.opacity;
            s.host.metallic  = params.metallicRoughness.x;
            s.host.roughness = params.metallicRoughness.y;
            s.host.emissive  = params.emissive.xyz * params.emissive.w;
            s.normal         = f.normalVC;
            s.normal         = shading.normal.normal(s);

            MaterialSample m = shading.material.evaluate(s);
            float3 radiance  = shading.lighting.ambient(m, s, float3(params.ambientRadiance))
                             + shading.lighting.direct(m, s, light);
            float3 Lo = shading.lighting.resolve(radiance, m) + m.emissive;

            float fragAlpha;
            if (params.alphaMode == 2u) {
                if (m.opacity < params.alphaCutoff)
                    continue;
                fragAlpha = 1.f;
            } else if (params.alphaMode == 1u) {
                fragAlpha = m.opacity;
            } else {
                fragAlpha = 1.f;
            }

            f.color = float4(Lo, fragAlpha);
            hookBlock.hooks.lit(f);
            f.color = shading.output.finish(f.color, m, s);
            f.depth = depth;
            hookBlock.hooks.output(f);
            colorBuf[pixIdx] = f.color;
        }
    }
}

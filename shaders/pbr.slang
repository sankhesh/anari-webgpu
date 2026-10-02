// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

import anari_raster_common;
import vtkslang_shading;

type_param M : IMaterial;
type_param L : ILightingModel;
typealias Default_IMaterial      = HostDrivenMaterial;
typealias Default_ILightingModel = CookTorranceLighting;

struct Shading<TM : IMaterial, TL : ILightingModel>
{
    TM material;
    TL lighting;
};

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
ParameterBlock<Shading<M, L>>                      shading;

float3 pbr_getPos(uint i)   { return float3(positions[i*3u], positions[i*3u+1u], positions[i*3u+2u]); }
float3 pbr_getNorm(uint i)  { return float3(normals[i*3u],   normals[i*3u+1u],   normals[i*3u+2u]);   }
float4 pbr_getColor(uint i) { return float4(colors[i*4u],    colors[i*4u+1u],    colors[i*4u+2u],    colors[i*4u+3u]); }

uint3 pbr_triangleIndices(uint triIdx)
{
    return uint3(indices[triIdx*3u], indices[triIdx*3u+1u], indices[triIdx*3u+2u]);
}

[shader("compute")]
[numthreads(256, 1, 1)]
void pbr_depth_main(uint3 gid : SV_DispatchThreadID)
{
    uint triIdx = gid.x;
    if (triIdx >= params.numTriangles)
        return;

    uint3 i = pbr_triangleIndices(triIdx);
    ScreenTriangle t;
    if (!setupScreenTriangle(params.mvp, pbr_getPos(i.x), pbr_getPos(i.y),
                             pbr_getPos(i.z), params.width, params.height, t))
        return;

    for (int py = int(t.minY); py <= int(t.maxY); py++) {
        for (int px = int(t.minX); px <= int(t.maxX); px++) {
            float3 w = barycentric(t, px, py);
            if (!covered(w))
                continue;
            uint pixIdx = uint(py)*params.width + uint(px);
            depthBufI[pixIdx].min(depthToU32(interpolateDepth(t, w)));
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

    uint3  i  = pbr_triangleIndices(triIdx);
    uint   i0 = i.x;
    uint   i1 = i.y;
    uint   i2 = i.z;
    float3 p0 = pbr_getPos(i0);
    float3 p1 = pbr_getPos(i1);
    float3 p2 = pbr_getPos(i2);

    ScreenTriangle t;
    if (!setupScreenTriangle(params.mvp, p0, p1, p2, params.width, params.height, t))
        return;

    LightSample light;
    light.direction = normalize(-params.lightDir);
    light.radiance  = params.lightIrradiance * params.lightColor;

    for (int py = int(t.minY); py <= int(t.maxY); py++) {
        for (int px = int(t.minX); px <= int(t.maxX); px++) {
            float3 w = barycentric(t, px, py);
            if (!covered(w))
                continue;
            float w0 = w.x;
            float w1 = w.y;
            float w2 = w.z;

            uint pixIdx = uint(py)*params.width + uint(px);
            uint dU     = depthToU32(interpolateDepth(t, w));
            if (dU > depthBufI[pixIdx].load() + kDepthTolerance)
                continue;

            float3 N;
            if (params.hasNormals != 0u)
                N = normalize(pbr_getNorm(i0)*w0 + pbr_getNorm(i1)*w1 + pbr_getNorm(i2)*w2);
            else
                N = normalize(cross(p1-p0, p2-p0));

            float3 baseRGB = params.baseColor.rgb;
            if (params.hasColors != 0u) {
                float4 vc = pbr_getColor(i0)*w0 + pbr_getColor(i1)*w1 + pbr_getColor(i2)*w2;
                baseRGB = baseRGB * vc.rgb;
            }

            SurfaceSample s;
            s.position = p0*w0 + p1*w1 + p2*w2;
            s.normal   = N;
            float3 ndcC = t.ndc0*w0 + t.ndc1*w1 + t.ndc2*w2;
            s.viewDir  = normalize(float3(-ndcC.x, -ndcC.y, 1.f));
            s.host.baseColor = baseRGB;
            s.host.opacity   = params.baseColor.a;
            s.host.metallic  = params.metallicRoughness.x;
            s.host.roughness = params.metallicRoughness.y;
            s.host.emissive  = params.emissive.xyz * params.emissive.w;

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

            colorBuf[pixIdx] = float4(Lo, fragAlpha);
        }
    }
}

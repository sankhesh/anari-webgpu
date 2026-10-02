// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

import anari_raster_common;
import vtkslang_shading;

type_param M : IMaterial;
type_param L : ILightingModel;
typealias Default_IMaterial      = HostDrivenMaterial;
typealias Default_ILightingModel = LambertLighting;

struct Shading<TM : IMaterial, TL : ILightingModel>
{
    TM material;
    TL lighting;
};

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
ParameterBlock<Shading<M, L>>                        shading;

float3 getPosition(uint i) { return float3(positions[i*3u], positions[i*3u+1u], positions[i*3u+2u]); }
float3 getNormal(uint i)   { return float3(normals[i*3u],   normals[i*3u+1u],   normals[i*3u+2u]);   }
float4 getColor(uint i)    { return float4(colors[i*4u],    colors[i*4u+1u],    colors[i*4u+2u],    colors[i*4u+3u]); }

uint3 triangleIndices(uint triIdx)
{
    return uint3(indices[triIdx*3u], indices[triIdx*3u+1u], indices[triIdx*3u+2u]);
}

[shader("compute")]
[numthreads(256, 1, 1)]
void depth_main(uint3 gid : SV_DispatchThreadID)
{
    uint triIdx = gid.x;
    if (triIdx >= params.numTriangles)
        return;

    uint3 i = triangleIndices(triIdx);
    ScreenTriangle t;
    if (!setupScreenTriangle(params.mvp, getPosition(i.x), getPosition(i.y),
                             getPosition(i.z), params.width, params.height, t))
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
void main(uint3 gid : SV_DispatchThreadID)
{
    uint triIdx = gid.x;
    if (triIdx >= params.numTriangles)
        return;

    uint3  i  = triangleIndices(triIdx);
    float3 p0 = getPosition(i.x);
    float3 p1 = getPosition(i.y);
    float3 p2 = getPosition(i.z);

    ScreenTriangle t;
    if (!setupScreenTriangle(params.mvp, p0, p1, p2, params.width, params.height, t))
        return;

    float3 faceNormal;
    if (params.hasNormals != 0u)
        faceNormal = normalize((getNormal(i.x) + getNormal(i.y) + getNormal(i.z)) / 3.f);
    else
        faceNormal = normalize(cross(p1 - p0, p2 - p0));

    LightSample light;
    light.direction = normalize(-params.lightDir);
    light.radiance  = float3(1.f);

    float4 c0 = params.materialColor;
    float4 c1 = params.materialColor;
    float4 c2 = params.materialColor;
    if (params.hasColors != 0u) {
        c0 = getColor(i.x) * params.materialColor;
        c1 = getColor(i.y) * params.materialColor;
        c2 = getColor(i.z) * params.materialColor;
    }

    for (int py = int(t.minY); py <= int(t.maxY); py++) {
        for (int px = int(t.minX); px <= int(t.maxX); px++) {
            float3 w = barycentric(t, px, py);
            if (!covered(w))
                continue;

            uint pixIdx = uint(py)*params.width + uint(px);
            uint dU     = depthToU32(interpolateDepth(t, w));
            if (dU > depthBufI[pixIdx].load() + kDepthTolerance)
                continue;

            float4 color = c0*w.x + c1*w.y + c2*w.z;
            SurfaceSample s;
            s.position       = p0*w.x + p1*w.y + p2*w.z;
            s.normal         = faceNormal;
            s.host.baseColor = color.rgb;
            s.host.opacity   = color.a;

            MaterialSample m = shading.material.evaluate(s);
            float3 radiance  = shading.lighting.ambient(m, s, float3(params.ambientRadiance))
                             + shading.lighting.direct(m, s, light);
            colorBuf[pixIdx] = float4(shading.lighting.resolve(radiance, m) + m.emissive, m.opacity);
        }
    }
}

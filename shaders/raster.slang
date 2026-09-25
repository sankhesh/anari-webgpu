// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

import anari_raster_common;

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

float3 getPosition(uint i) { return float3(positions[i*3u], positions[i*3u+1u], positions[i*3u+2u]); }
float3 getNormal(uint i)   { return float3(normals[i*3u],   normals[i*3u+1u],   normals[i*3u+2u]);   }
float4 getColor(uint i)    { return float4(colors[i*4u],    colors[i*4u+1u],    colors[i*4u+2u],    colors[i*4u+3u]); }

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 gid : SV_DispatchThreadID)
{
    uint triIdx = gid.x;
    if (triIdx >= params.numTriangles)
        return;

    uint i0 = indices[triIdx*3u];
    uint i1 = indices[triIdx*3u+1u];
    uint i2 = indices[triIdx*3u+2u];

    float3 p0 = getPosition(i0);
    float3 p1 = getPosition(i1);
    float3 p2 = getPosition(i2);

    float4 clip0 = mul(params.mvp, float4(p0, 1.f));
    float4 clip1 = mul(params.mvp, float4(p1, 1.f));
    float4 clip2 = mul(params.mvp, float4(p2, 1.f));
    if (clip0.w <= 0.f || clip1.w <= 0.f || clip2.w <= 0.f)
        return;

    float3 ndc0 = clip0.xyz / clip0.w;
    float3 ndc1 = clip1.xyz / clip1.w;
    float3 ndc2 = clip2.xyz / clip2.w;

    float W = float(params.width);
    float H = float(params.height);
    float3 s0 = float3((ndc0.x*0.5f+0.5f)*W, (1.f-(ndc0.y*0.5f+0.5f))*H, ndc0.z);
    float3 s1 = float3((ndc1.x*0.5f+0.5f)*W, (1.f-(ndc1.y*0.5f+0.5f))*H, ndc1.z);
    float3 s2 = float3((ndc2.x*0.5f+0.5f)*W, (1.f-(ndc2.y*0.5f+0.5f))*H, ndc2.z);

    float minX = max(0.f, min(min(s0.x, s1.x), s2.x));
    float maxX = min(W-1.f, max(max(s0.x, s1.x), s2.x));
    float minY = max(0.f, min(min(s0.y, s1.y), s2.y));
    float maxY = min(H-1.f, max(max(s0.y, s1.y), s2.y));

    float area = edgeFunction(s0.xy, s1.xy, s2.xy);
    if (abs(area) < 1e-6f)
        return;

    float3 faceNormal;
    if (params.hasNormals != 0u)
        faceNormal = normalize(getNormal(i0) + getNormal(i1) + getNormal(i2));
    else
        faceNormal = normalize(cross(p1 - p0, p2 - p0));

    float3 L      = normalize(-params.lightDir);
    float  NdotL  = max(dot(faceNormal, L), 0.f);
    float  light  = min(NdotL + params.ambientRadiance, 1.f);

    float4 c0 = params.materialColor;
    float4 c1 = params.materialColor;
    float4 c2 = params.materialColor;
    if (params.hasColors != 0u) {
        c0 = getColor(i0) * params.materialColor;
        c1 = getColor(i1) * params.materialColor;
        c2 = getColor(i2) * params.materialColor;
    }

    for (int py = int(minY); py <= int(maxY); py++) {
        for (int px = int(minX); px <= int(maxX); px++) {
            float2 pp = float2(float(px)+0.5f, float(py)+0.5f);
            float  w0 = edgeFunction(s1.xy, s2.xy, pp) / area;
            float  w1 = edgeFunction(s2.xy, s0.xy, pp) / area;
            float  w2 = edgeFunction(s0.xy, s1.xy, pp) / area;
            if (w0 < 0.f || w1 < 0.f || w2 < 0.f)
                continue;

            float depth  = w0*s0.z + w1*s1.z + w2*s2.z;
            uint  dU     = depthToU32(depth);
            uint  pixIdx = uint(py)*params.width + uint(px);

            uint prev = depthBufI[pixIdx].min(dU);
            if (dU > prev)
                continue;

            float4 color = c0*w0 + c1*w1 + c2*w2;
            colorBuf[pixIdx] = float4(color.rgb * light, color.a);
        }
    }
}

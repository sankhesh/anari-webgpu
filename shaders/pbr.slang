// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

import vtkslang_core;
import anari_raster_common;

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

float3 pbr_getPos(uint i)   { return float3(positions[i*3u], positions[i*3u+1u], positions[i*3u+2u]); }
float3 pbr_getNorm(uint i)  { return float3(normals[i*3u],   normals[i*3u+1u],   normals[i*3u+2u]);   }
float4 pbr_getColor(uint i) { return float4(colors[i*4u],    colors[i*4u+1u],    colors[i*4u+2u],    colors[i*4u+3u]); }

uint3 pbr_triangleIndices(uint triIdx)
{
    return uint3(indices[triIdx*3u], indices[triIdx*3u+1u], indices[triIdx*3u+2u]);
}

float D_GGX(float NdotH, float alpha)
{
    float a2 = alpha * alpha;
    float d  = NdotH * NdotH * (a2 - 1.f) + 1.f;
    return a2 / (PI * d * d + 1e-4f);
}

float G_SchlickGGX(float NdotX, float roughness)
{
    float r = roughness + 1.f;
    float k = (r * r) / 8.f;
    return NdotX / (NdotX * (1.f - k) + k + 1e-4f);
}

float G_Smith(float NdotV, float NdotL, float roughness)
{
    return G_SchlickGGX(NdotV, roughness) * G_SchlickGGX(NdotL, roughness);
}

float3 F_Schlick(float cosTheta, float3 F0)
{
    float  t  = clamp(1.f - cosTheta, 0.f, 1.f);
    float  t5 = t * t * t * t * t;
    return F0 + (float3(1.f, 1.f, 1.f) - F0) * t5;
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

    float metallic  = params.metallicRoughness.x;
    float roughness = params.metallicRoughness.y;
    float alpha     = roughness * roughness;

    float3 L = normalize(-params.lightDir);

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

            float3 ndcC = t.ndc0*w0 + t.ndc1*w1 + t.ndc2*w2;
            float3 V    = normalize(float3(-ndcC.x, -ndcC.y, 1.f));
            float3 H    = normalize(V + L);

            float NdotL = max(dot(N, L), 0.f);
            float NdotV = max(dot(N, V), 1e-4f);
            float NdotH = max(dot(N, H), 0.f);
            float HdotV = max(dot(H, V), 0.f);

            float3 baseRGB = params.baseColor.rgb;
            if (params.hasColors != 0u) {
                float4 vc = pbr_getColor(i0)*w0 + pbr_getColor(i1)*w1 + pbr_getColor(i2)*w2;
                baseRGB = baseRGB * vc.rgb;
            }

            float3 F0       = lerp(float3(0.04f, 0.04f, 0.04f), baseRGB, metallic);
            float  D        = D_GGX(NdotH, alpha);
            float  G        = G_Smith(NdotV, NdotL, roughness);
            float3 F        = F_Schlick(HdotV, F0);
            float3 specular = (D * G * F) / (4.f * NdotV * NdotL + 1e-4f);
            float3 kD       = (float3(1.f, 1.f, 1.f) - F) * (1.f - metallic);
            float3 diffuse  = kD * baseRGB / PI;

            float3 irr = params.lightIrradiance * params.lightColor;
            float3 Lo  = (diffuse + specular) * irr * NdotL;
            Lo        += params.ambientRadiance * baseRGB;

            Lo = tonemapAndGamma(Lo);

            Lo += params.emissive.xyz * params.emissive.w;

            float fragAlpha;
            if (params.alphaMode == 2u) {
                if (params.baseColor.a < params.alphaCutoff)
                    continue;
                fragAlpha = 1.f;
            } else if (params.alphaMode == 1u) {
                fragAlpha = params.baseColor.a;
            } else {
                fragAlpha = 1.f;
            }

            colorBuf[pixIdx] = float4(Lo, fragAlpha);
        }
    }
}

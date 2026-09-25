// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

import vtkslang_core;
import anari_scivis;

struct VolumeUniforms
{
    float4x4 viewInv;
    float4x4 projInv;
    float3   volumeMin;
    float    stepSize;
    float3   volumeMax;
    float    densityScale;
    uint3    dims;
    uint     tfSize;
    float    valueRangeMin;
    float    valueRangeMax;
    uint     width;
    uint     height;
};

[vk::binding(0, 0)] RWStructuredBuffer<float4>   colorBuf;
[vk::binding(1, 0)] StructuredBuffer<float>       depthBuf;
[vk::binding(2, 0)] ConstantBuffer<VolumeUniforms> params;
[vk::binding(3, 0)] StructuredBuffer<float>        volData;
[vk::binding(4, 0)] StructuredBuffer<float4>       tfTable;

float sampleVolume(float3 worldPos)
{
    float3 ext = params.volumeMax - params.volumeMin;
    float3 uvw = (worldPos - params.volumeMin) / ext;
    return SampleVolumeTrilinear(volData, params.dims, uvw);
}

[shader("compute")]
[numthreads(256, 1, 1)]
void vol_main(uint3 gid : SV_DispatchThreadID)
{
    uint pixIdx = gid.x;
    if (pixIdx >= params.width * params.height)
        return;

    uint px = pixIdx % params.width;
    uint py = pixIdx / params.width;

    float ndcX =  (float(px) + 0.5f) / float(params.width)  * 2.f - 1.f;
    float ndcY = -((float(py) + 0.5f) / float(params.height) * 2.f - 1.f);

    float4 clipPos  = float4(ndcX, ndcY, -1.f, 1.f);
    float4 viewPos4 = mul(params.projInv, clipPos);
    float3 viewDir  = normalize(float3(viewPos4.x / viewPos4.w,
                                       viewPos4.y / viewPos4.w,
                                       -1.f));
    float3 worldDir = normalize(mul(params.viewInv, float4(viewDir, 0.f)).xyz);
    float3 worldOri = mul(params.viewInv, float4(0.f, 0.f, 0.f, 1.f)).xyz;

    float2 tHit = IntersectAABB(worldOri, worldDir, params.volumeMin, params.volumeMax);
    if (tHit.x > tHit.y || tHit.y < 0.f)
        return;

    float tStart = max(tHit.x, 0.f);
    float tEnd   = tHit.y;

    float3 accColor = float3(0.f, 0.f, 0.f);
    float  accAlpha = 0.f;

    int numSteps = int((tEnd - tStart) / params.stepSize) + 1;
    for (int i = 0; i < numSteps; i++) {
        float tCurr = tStart + float(i) * params.stepSize;
        if (tCurr > tEnd)
            break;

        float3 pos    = worldOri + tCurr * worldDir;
        float  scalar = sampleVolume(pos);
        float4 sample = ApplyTransferFunction(tfTable, params.tfSize, scalar,
                                              params.valueRangeMin,
                                              params.valueRangeMax);
        float  sa     = clamp(sample.a * params.densityScale * params.stepSize,
                               0.f, 1.f);

        CompositeFrontToBack(accColor, accAlpha, sample, sa);

        if (accAlpha >= 0.99f)
            break;
    }

    if (accAlpha < 0.001f)
        return;

    float4 geomColor = colorBuf[pixIdx];
    float3 outColor  = accColor + geomColor.rgb * (1.f - accAlpha);
    colorBuf[pixIdx] = float4(outColor, min(geomColor.a + accAlpha, 1.f));
}

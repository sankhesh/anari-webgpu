// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

module anari_scivis;
import vtkslang_core;

public struct SciVisParams
{
    public int   numClippingPlanes;
    public float isoValue;
    public int   useTransferFunction;
    public int   _pad;
};

public float4 SampleTransferFunction(StructuredBuffer<float4> tfTable,
                                      uint                     tfSize,
                                      float                    t)
{
    t = clamp(t, 0.f, 1.f);
    uint idx = uint(t * float(tfSize - 1u));
    return tfTable[idx];
}

public float4 ApplyTransferFunction(StructuredBuffer<float4> tfTable,
                                     uint   tfSize,
                                     float  scalar,
                                     float  valueRangeMin,
                                     float  valueRangeMax)
{
    float t = (scalar - valueRangeMin) / (valueRangeMax - valueRangeMin + 1e-4f);
    return SampleTransferFunction(tfTable, tfSize, t);
}

public float SampleVolumeTrilinear(StructuredBuffer<float> volData,
                                    uint3                   dims,
                                    float3                  uvw)
{
    if (any(uvw < float3(0.f, 0.f, 0.f)) || any(uvw > float3(1.f, 1.f, 1.f)))
        return 0.f;

    float3 coord = uvw * float3(dims - uint3(1u, 1u, 1u));
    uint3  c0    = uint3(coord);
    uint3  c1    = min(c0 + uint3(1u, 1u, 1u), dims - uint3(1u, 1u, 1u));
    float3 fr    = frac(coord);

    uint dxy = dims.x * dims.y;
    float v000 = volData[c0.x + c0.y*dims.x + c0.z*dxy];
    float v100 = volData[c1.x + c0.y*dims.x + c0.z*dxy];
    float v010 = volData[c0.x + c1.y*dims.x + c0.z*dxy];
    float v110 = volData[c1.x + c1.y*dims.x + c0.z*dxy];
    float v001 = volData[c0.x + c0.y*dims.x + c1.z*dxy];
    float v101 = volData[c1.x + c0.y*dims.x + c1.z*dxy];
    float v011 = volData[c0.x + c1.y*dims.x + c1.z*dxy];
    float v111 = volData[c1.x + c1.y*dims.x + c1.z*dxy];

    float v00 = lerp(v000, v100, fr.x);
    float v10 = lerp(v010, v110, fr.x);
    float v01 = lerp(v001, v101, fr.x);
    float v11 = lerp(v011, v111, fr.x);
    float v0  = lerp(v00,  v10,  fr.y);
    float v1  = lerp(v01,  v11,  fr.y);
    return lerp(v0, v1, fr.z);
}

public float2 IntersectAABB(float3 rayOrigin,
                              float3 rayDir,
                              float3 boxMin,
                              float3 boxMax)
{
    float3 invDir = 1.f / rayDir;
    float3 t0     = (boxMin - rayOrigin) * invDir;
    float3 t1     = (boxMax - rayOrigin) * invDir;
    float3 tSmall = min(t0, t1);
    float3 tLarge = max(t0, t1);
    float  tMin   = max(max(tSmall.x, tSmall.y), tSmall.z);
    float  tMax   = min(min(tLarge.x, tLarge.y), tLarge.z);
    return float2(tMin, tMax);
}

public void CompositeFrontToBack(inout float3 accColor,
                                  inout float  accAlpha,
                                  float4       sampleRGBA,
                                  float        stepAlpha)
{
    float sa = clamp(stepAlpha, 0.f, 1.f);
    accColor += (1.f - accAlpha) * sampleRGBA.rgb * sa;
    accAlpha += (1.f - accAlpha) * sa;
}

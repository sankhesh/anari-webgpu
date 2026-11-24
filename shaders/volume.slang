// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

import slangkit_volume;

static const uint kExtremesSize = 1024u;
static const uint kClassifyRow  = 65536u;

struct VolumeUniforms
{
    float4x4 inverseViewProjection;
    float4x4 viewProjection;
    float4x4 worldToTexture;
    float4   dims;
    float4   spacing;
    float4   cellDims;
    float4   eye;
    float    valueMin;
    float    valueScale;
    float    baseStep;
    float    unitDistance;
    uint     width;
    uint     height;
    uint     colorCount;
    uint     opacityCount;
    uint     maxSteps;
    uint     nearest;
    uint     _pad;
    uint     cellCount;
};

[vk::binding(0, 0)]  RWStructuredBuffer<float4>     colorBuf;
[vk::binding(1, 0)]  RWStructuredBuffer<uint>       depthBuf;
[vk::binding(2, 0)]  ConstantBuffer<VolumeUniforms> params;
[vk::binding(3, 0)]  StructuredBuffer<float>        voxels;
[vk::binding(4, 0)]  StructuredBuffer<float4>       colorTable;
[vk::binding(5, 0)]  StructuredBuffer<float>        opacityTable;
[vk::binding(6, 0)]  StructuredBuffer<float2>       cells;
[vk::binding(7, 0)]  StructuredBuffer<float2>       cellRanges;
[vk::binding(8, 0)]  StructuredBuffer<uint>         surfaceDepth;
[vk::binding(9, 0)]  StructuredBuffer<float2>       extremeTable;
[vk::binding(10, 0)] RWStructuredBuffer<float2>     classified;

float voxel(uint3 i)
{
    uint3 d = uint3(params.dims.xyz);
    return voxels[(i.z * d.y + i.y) * d.x + i.x];
}

struct FieldSampler : IVolumeSampler
{
    typealias Cursor = float3;

    VolumeGrid grid()
    {
        VolumeGrid g = VolumeGrid();
        g.dims       = params.dims.xyz;
        g.spacing    = params.spacing.xyz;
        return g;
    }

    float3 locate(float3 texCoord) { return grid().voxelOf(texCoord); }

    float4 sample(float3 v)
    {
        uint3 last = uint3(params.dims.xyz) - 1u;
        v = clamp(v, float3(0.f), float3(last));
        if (params.nearest != 0u)
            return float4(voxel(min(uint3(round(v)), last)), 0.f, 0.f, 0.f);
        uint3  i0 = min(uint3(v), last);
        uint3  i1 = min(i0 + 1u, last);
        float3 f  = v - float3(i0);
        float c00 = lerp(voxel(i0), voxel(uint3(i1.x, i0.y, i0.z)), f.x);
        float c10 = lerp(voxel(uint3(i0.x, i1.y, i0.z)), voxel(uint3(i1.x, i1.y, i0.z)), f.x);
        float c01 = lerp(voxel(uint3(i0.x, i0.y, i1.z)), voxel(uint3(i1.x, i0.y, i1.z)), f.x);
        float c11 = lerp(voxel(uint3(i0.x, i1.y, i1.z)), voxel(i1), f.x);
        return float4(lerp(lerp(c00, c10, f.y), lerp(c01, c11, f.y), f.z), 0.f, 0.f, 0.f);
    }
};

float4 colorAt(float t)
{
    uint  last     = params.colorCount - 1u;
    float position = t * float(last);
    uint  lower    = min(uint(position), last);
    return lerp(colorTable[lower], colorTable[min(lower + 1u, last)], position - float(lower));
}

float opacityAt(float t)
{
    uint  last     = params.opacityCount - 1u;
    float position = t * float(last);
    uint  lower    = min(uint(position), last);
    return lerp(opacityTable[lower], opacityTable[min(lower + 1u, last)], position - float(lower));
}

struct FieldTransfer : ITransferFunction
{
    float normalize(uint component, float value)
    {
        return clamp((value - params.valueMin) * params.valueScale, 0.f, 1.f);
    }
    float opacity(uint component, float normalized) { return opacityAt(normalized) * colorAt(normalized).a; }
    float3 color(uint component, float normalized) { return colorAt(normalized).rgb; }
};

struct FieldCells : IMacrocells
{
    float cellSize() { return params.cellDims.w; }
    uint index(float3 voxel)
    {
        float3 cell = clamp(floor(voxel / params.cellDims.w), float3(0.f), params.cellDims.xyz - 1.f);
        return uint((cell.z * params.cellDims.y + cell.y) * params.cellDims.x + cell.x);
    }
    float2 cell(float3 voxel) { return cells[index(voxel)]; }
    float2 range(float3 voxel) { return cellRanges[index(voxel)]; }
};

struct ExtremesTable : IOpacityExtremes
{
    float2 extremes(uint function, uint level, uint entry) { return extremeTable[level * kExtremesSize + entry]; }
};

[shader("compute")]
[numthreads(64, 1, 1)]
void vol_classify(uint3 gid : SV_DispatchThreadID)
{
    uint i = gid.y * kClassifyRow + gid.x;
    if (i >= params.cellCount)
        return;
    float2 range  = cellRanges[i];
    classified[i] = range.x > range.y
        ? float2(1.f)
        : classifyCell(ExtremesTable(), 0u, kExtremesSize,
                       params.valueScale < 0.f ? range.yx : range, params.valueMin,
                       params.valueScale);
}

[shader("compute")]
[numthreads(256, 1, 1)]
void vol_main(uint3 gid : SV_DispatchThreadID)
{
    uint pix = gid.x;
    if (pix >= params.width * params.height)
        return;

    uint   px  = pix % params.width;
    uint   py  = pix / params.width;
    float2 ndc = float2((float(px) + 0.5f) / float(params.width) * 2.f - 1.f,
                        1.f - (float(py) + 0.5f) / float(params.height) * 2.f);

    float3 origin    = params.eye.xyz;
    float3 farPoint  = unproject(params.inverseViewProjection, ndc, 1.f);
    float3 direction = normalize(farPoint - origin);
    float  tLimit    = distance(origin, farPoint);
    float  surface   = asfloat(surfaceDepth[pix]);
    if (surface < 1.f)
        tLimit = min(tLimit, dot(unproject(params.inverseViewProjection, ndc, surface) - origin,
                                 direction));

    VolumeRay ray = volumeRay(params.worldToTexture, origin, direction, tLimit);
    if (ray.tNear >= ray.tFar)
        return;

    VolumeMarch m  = VolumeMarch();
    m.baseStep     = params.baseStep;
    m.unitDistance = params.unitDistance;
    m.maxSteps     = params.maxSteps;
    m.skipEmpty    = true;
    m.jitter       = 0.5f * params.baseStep;

    VolumeResult r = marchVolume(FieldSampler(), FieldTransfer(), FieldCells(),
                                 DefaultVolumeHooks(), ray, m);

    float4 entry  = mul(params.viewProjection, float4(origin + ray.tNear * direction, 1.f));
    depthBuf[pix] = min(depthBuf[pix], asuint(clamp(entry.z / entry.w, 0.f, 1.f)));

    if (!r.covered)
        return;
    float4 behind = colorBuf[pix];
    float  a      = r.color.a;
    colorBuf[pix] = float4(r.color.rgb + behind.rgb * (1.f - a), a + behind.a * (1.f - a));
}

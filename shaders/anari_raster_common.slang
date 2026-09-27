// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

module anari_raster_common;

public uint depthToU32(float d)
{
    return asuint(clamp(d, 0.f, 1.f));
}

public float edgeFunction(float2 a, float2 b, float2 c)
{
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

public static const uint kDepthTolerance = 256u;

public struct ScreenTriangle
{
    public float3 s0 = float3(0.f);
    public float3 s1 = float3(0.f);
    public float3 s2 = float3(0.f);
    public float3 ndc0 = float3(0.f);
    public float3 ndc1 = float3(0.f);
    public float3 ndc2 = float3(0.f);
    public float  area = 0.f;
    public float  minX = 0.f;
    public float  maxX = 0.f;
    public float  minY = 0.f;
    public float  maxY = 0.f;
};

public bool setupScreenTriangle(float4x4 mvp, float3 p0, float3 p1, float3 p2,
                                uint width, uint height, out ScreenTriangle t)
{
    t = ScreenTriangle();
    float4 c0 = mul(mvp, float4(p0, 1.f));
    float4 c1 = mul(mvp, float4(p1, 1.f));
    float4 c2 = mul(mvp, float4(p2, 1.f));
    if (c0.w <= 0.f || c1.w <= 0.f || c2.w <= 0.f)
        return false;

    t.ndc0 = c0.xyz / c0.w;
    t.ndc1 = c1.xyz / c1.w;
    t.ndc2 = c2.xyz / c2.w;

    float W = float(width);
    float H = float(height);
    t.s0 = float3((t.ndc0.x*0.5f+0.5f)*W, (1.f-(t.ndc0.y*0.5f+0.5f))*H, t.ndc0.z);
    t.s1 = float3((t.ndc1.x*0.5f+0.5f)*W, (1.f-(t.ndc1.y*0.5f+0.5f))*H, t.ndc1.z);
    t.s2 = float3((t.ndc2.x*0.5f+0.5f)*W, (1.f-(t.ndc2.y*0.5f+0.5f))*H, t.ndc2.z);

    t.minX = max(0.f, min(min(t.s0.x, t.s1.x), t.s2.x));
    t.maxX = min(W-1.f, max(max(t.s0.x, t.s1.x), t.s2.x));
    t.minY = max(0.f, min(min(t.s0.y, t.s1.y), t.s2.y));
    t.maxY = min(H-1.f, max(max(t.s0.y, t.s1.y), t.s2.y));

    t.area = edgeFunction(t.s0.xy, t.s1.xy, t.s2.xy);
    return abs(t.area) >= 1e-6f;
}

public float3 barycentric(ScreenTriangle t, int px, int py)
{
    float2 p = float2(float(px)+0.5f, float(py)+0.5f);
    return float3(edgeFunction(t.s1.xy, t.s2.xy, p) / t.area,
                  edgeFunction(t.s2.xy, t.s0.xy, p) / t.area,
                  edgeFunction(t.s0.xy, t.s1.xy, p) / t.area);
}

public bool covered(float3 w) { return w.x >= 0.f && w.y >= 0.f && w.z >= 0.f; }

public float interpolateDepth(ScreenTriangle t, float3 w)
{
    return w.x*t.s0.z + w.y*t.s1.z + w.z*t.s2.z;
}

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

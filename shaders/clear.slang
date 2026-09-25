// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

import vtkslang_core;

struct ClearUniforms
{
    float4 bgColor;
    uint   width;
    uint   height;
    uint2  _pad;
};

[vk::binding(0, 0)] RWStructuredBuffer<float4> colorBuf;
[vk::binding(1, 0)] RWStructuredBuffer<float>  depthBuf;
[vk::binding(2, 0)] ConstantBuffer<ClearUniforms> params;

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 gid : SV_DispatchThreadID)
{
    uint idx = gid.x;
    if (idx >= params.width * params.height)
        return;

    colorBuf[idx] = params.bgColor;
    depthBuf[idx] = 1.f;
}

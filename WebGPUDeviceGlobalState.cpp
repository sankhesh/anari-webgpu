// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "WebGPUDeviceGlobalState.h"
#include <webgpu/webgpu.h>

namespace anari_webgpu {

WebGPUDeviceGlobalState::WebGPUDeviceGlobalState(ANARIDevice d)
    : helium::BaseGlobalDeviceState(d) {}

WebGPUDeviceGlobalState::~WebGPUDeviceGlobalState()
{
  wgpuQueueRelease(wgpuQueue);
  wgpuDeviceRelease(wgpuDevice);
  wgpuAdapterRelease(wgpuAdapter);
  wgpuInstanceRelease(wgpuInstance);
}

} // namespace anari_webgpu

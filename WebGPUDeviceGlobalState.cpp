// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "WebGPUDeviceGlobalState.h"
#include <webgpu/webgpu.h>

namespace anari_webgpu {

WebGPUDeviceGlobalState::WebGPUDeviceGlobalState(ANARIDevice d)
    : helium::BaseGlobalDeviceState(d)
{}

WebGPUDeviceGlobalState::~WebGPUDeviceGlobalState()
{
  // Only release handles we own (internally created).
  // Externally provided (shared) handles are managed by the caller.
  if (wgpuQueue && ownsQueue) {
    wgpuQueueRelease(wgpuQueue);
  }
  wgpuQueue = nullptr;

  if (wgpuDevice && ownsDevice) {
    wgpuDeviceRelease(wgpuDevice);
  }
  wgpuDevice = nullptr;

  if (wgpuAdapter && ownsAdapter) {
    wgpuAdapterRelease(wgpuAdapter);
  }
  wgpuAdapter = nullptr;

  if (wgpuInstance && ownsInstance) {
    wgpuInstanceRelease(wgpuInstance);
  }
  wgpuInstance = nullptr;
}

} // namespace anari_webgpu

// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "helium/BaseGlobalDeviceState.h"

typedef struct WGPUInstanceImpl *WGPUInstance;
typedef struct WGPUAdapterImpl *WGPUAdapter;
typedef struct WGPUDeviceImpl *WGPUDevice;
typedef struct WGPUQueueImpl *WGPUQueue;

namespace anari_webgpu {

struct WebGPUDeviceGlobalState : public helium::BaseGlobalDeviceState
{
  WGPUInstance wgpuInstance{nullptr};
  WGPUAdapter wgpuAdapter{nullptr};
  WGPUDevice wgpuDevice{nullptr};
  WGPUQueue wgpuQueue{nullptr};

  WebGPUDeviceGlobalState(ANARIDevice d);
  ~WebGPUDeviceGlobalState();
};

inline WebGPUDeviceGlobalState *asWebGPUDeviceState(
    helium::BaseGlobalDeviceState *s)
{
  return (WebGPUDeviceGlobalState *)s;
}

#define WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(type, anari_type) \
  namespace anari { ANARI_TYPEFOR_SPECIALIZATION(type, anari_type); }

#define WEBGPU_ANARI_TYPEFOR_DEFINITION(type) \
  namespace anari { ANARI_TYPEFOR_DEFINITION(type); }

} // namespace anari_webgpu

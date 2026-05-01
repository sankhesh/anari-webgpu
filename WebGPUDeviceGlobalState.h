// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

// helium
#include "helium/BaseGlobalDeviceState.h"

// Forward-declare wgpu types to avoid pulling in webgpu.h everywhere.
// The actual header is only included in implementation files that need it.
typedef struct WGPUInstanceImpl *WGPUInstance;
typedef struct WGPUAdapterImpl *WGPUAdapter;
typedef struct WGPUDeviceImpl *WGPUDevice;
typedef struct WGPUQueueImpl *WGPUQueue;

namespace anari_webgpu {

struct WebGPUDeviceGlobalState : public helium::BaseGlobalDeviceState
{
  // WebGPU state available to all objects.
  // These handles may be externally provided (shared) or internally created.
  // When externally provided, the device does NOT release them on destruction.
  WGPUInstance wgpuInstance{nullptr};
  WGPUAdapter wgpuAdapter{nullptr};
  WGPUDevice wgpuDevice{nullptr};
  WGPUQueue wgpuQueue{nullptr};

  // Ownership flags — false when handles are externally provided
  bool ownsInstance{true};
  bool ownsAdapter{true};
  bool ownsDevice{true};
  bool ownsQueue{true};

  WebGPUDeviceGlobalState(ANARIDevice d);
  ~WebGPUDeviceGlobalState();
};

// Helper functions/macros ////////////////////////////////////////////////////

inline WebGPUDeviceGlobalState *asWebGPUDeviceState(
    helium::BaseGlobalDeviceState *s)
{
  return (WebGPUDeviceGlobalState *)s;
}

#define WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(type, anari_type)                  \
  namespace anari {                                                            \
  ANARI_TYPEFOR_SPECIALIZATION(type, anari_type);                              \
  }

#define WEBGPU_ANARI_TYPEFOR_DEFINITION(type)                                  \
  namespace anari {                                                            \
  ANARI_TYPEFOR_DEFINITION(type);                                              \
  }

} // namespace anari_webgpu

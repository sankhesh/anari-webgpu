// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"

namespace anari_webgpu {

struct Sampler : public Object
{
  Sampler(WebGPUDeviceGlobalState *d);
  virtual ~Sampler() = default;
  static Sampler *createInstance(
      std::string_view subtype, WebGPUDeviceGlobalState *d);
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::Sampler *, ANARI_SAMPLER);

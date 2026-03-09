// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"

namespace anari_webgpu {

struct SpatialField : public Object
{
  SpatialField(WebGPUDeviceGlobalState *d);
  virtual ~SpatialField() = default;
  static SpatialField *createInstance(
      std::string_view subtype, WebGPUDeviceGlobalState *d);
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(
    anari_webgpu::SpatialField *, ANARI_SPATIAL_FIELD);

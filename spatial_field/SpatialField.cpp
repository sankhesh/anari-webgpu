// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "SpatialField.h"

namespace anari_webgpu {

SpatialField::SpatialField(WebGPUDeviceGlobalState *s)
    : Object(ANARI_SPATIAL_FIELD, s)
{}

SpatialField *SpatialField::createInstance(
    std::string_view subtype, WebGPUDeviceGlobalState *s)
{
  return (SpatialField *)new UnknownObject(ANARI_SPATIAL_FIELD, s);
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::SpatialField *);

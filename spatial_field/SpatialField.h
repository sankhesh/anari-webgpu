// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"
#include "array/Array3D.h"

namespace anari_webgpu {

struct SpatialField : public Object
{
  SpatialField(WebGPUDeviceGlobalState *d);
  virtual ~SpatialField() = default;
  static SpatialField *createInstance(
      std::string_view subtype, WebGPUDeviceGlobalState *d);
};

struct StructuredRegularField : public SpatialField
{
  StructuredRegularField(WebGPUDeviceGlobalState *s);
  void commitParameters() override;
  bool isValid() const override;

  float sample(float3 uvw) const;

  float sampleWorld(float3 worldPos) const;

  float3 origin() const;
  float3 spacing() const;
  uint3 dims() const;
  float3 worldMin() const;
  float3 worldMax() const;

  const float *rawData() const;
  uint64_t numVoxels() const;

 private:
  helium::IntrusivePtr<Array3D> m_data;
  float3 m_origin{0.f, 0.f, 0.f};
  float3 m_spacing{1.f, 1.f, 1.f};
  uint3 m_dims{0u, 0u, 0u};
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(
    anari_webgpu::SpatialField *, ANARI_SPATIAL_FIELD);

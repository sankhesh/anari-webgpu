// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"
#include "array/Array3D.h"

#include <vector>

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
  static constexpr uint32_t kCellSize = 8;

  StructuredRegularField(WebGPUDeviceGlobalState *s);
  void commitParameters() override;
  void finalize() override;
  bool isValid() const override;

  float sampleVoxel(float3 voxel) const;

  float3 origin() const;
  float3 spacing() const;
  uint3 dims() const;
  float3 worldMin() const;
  float3 worldMax() const;
  bool nearestFilter() const;
  float stepSize() const;

  const std::vector<float> &values() const;
  uint3 cellDims() const;
  const std::vector<float2> &cellRanges() const;

 private:
  float valueAt(uint32_t x, uint32_t y, uint32_t z) const;

  helium::IntrusivePtr<Array3D> m_data;
  float3 m_origin{0.f, 0.f, 0.f};
  float3 m_spacing{1.f, 1.f, 1.f};
  uint3 m_dims{0u, 0u, 0u};
  bool m_nearest{false};
  std::vector<float> m_values;
  uint3 m_cellDims{0u, 0u, 0u};
  std::vector<float2> m_cellRanges;
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(
    anari_webgpu::SpatialField *, ANARI_SPATIAL_FIELD);

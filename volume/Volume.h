// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"
#include "array/Array1D.h"
#include "spatial_field/SpatialField.h"

namespace anari_webgpu {

struct Volume : public Object
{
  Volume(WebGPUDeviceGlobalState *d);
  virtual ~Volume() = default;
  static Volume *createInstance(
      std::string_view subtype, WebGPUDeviceGlobalState *d);

  void commitParameters() override;
  uint32_t id() const;

 private:
  uint32_t m_id{~0u};
};

struct TransferFunction1DVolume : public Volume
{
  TransferFunction1DVolume(WebGPUDeviceGlobalState *s);
  void commitParameters() override;
  bool isValid() const override;

  const StructuredRegularField *field() const;

  float2 valueRange() const;
  float densityScale() const;
  float unitDistance() const;

  uint32_t tfSize() const;

  float4 transferFunction(float t) const;

  std::vector<float4> buildTFTable(uint32_t resolution) const;

 private:
  helium::IntrusivePtr<SpatialField> m_field;
  helium::IntrusivePtr<Array1D> m_color;
  helium::IntrusivePtr<Array1D> m_opacity;
  float2 m_valueRange{0.f, 1.f};
  float m_densityScale{1.f};
  float m_unitDistance{1.f};
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::Volume *, ANARI_VOLUME);

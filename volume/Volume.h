// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"
#include "array/Array1D.h"
#include "spatial_field/SpatialField.h"

#include <vector>

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
  static constexpr uint32_t kExtremesSize = 1024;
  static constexpr uint32_t kExtremesLevels = 11;

  TransferFunction1DVolume(WebGPUDeviceGlobalState *s);
  void commitParameters() override;
  void finalize() override;
  bool isValid() const override;

  const StructuredRegularField *field() const;

  float2 valueRange() const;
  float unitDistance() const;

  float normalized(float value) const;
  float4 colorOf(float normalized) const;
  float opacityOf(float normalized) const;

  const std::vector<float4> &colors() const;
  const std::vector<float> &opacities() const;
  std::vector<float2> opacityExtremes() const;

 private:
  helium::IntrusivePtr<SpatialField> m_field;
  helium::IntrusivePtr<Array1D> m_colorData;
  helium::IntrusivePtr<Array1D> m_opacityData;
  float4 m_uniformColor{1.f, 1.f, 1.f, 1.f};
  float m_uniformOpacity{1.f};
  float2 m_valueRange{0.f, 1.f};
  float m_unitDistance{1.f};
  std::vector<float4> m_colors;
  std::vector<float> m_opacities;
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::Volume *, ANARI_VOLUME);

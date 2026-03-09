// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"

namespace anari_webgpu {

struct Light : public Object
{
  Light(WebGPUDeviceGlobalState *d);
  ~Light() override = default;
  static Light *createInstance(
      std::string_view subtype, WebGPUDeviceGlobalState *d);
};

struct DirectionalLight : public Light
{
  DirectionalLight(WebGPUDeviceGlobalState *s);
  void commitParameters() override;
  bool isValid() const override;

  float3 direction() const;
  float3 color() const;
  float irradiance() const;

 private:
  float3 m_direction{0.f, 0.f, -1.f};
  float3 m_color{1.f, 1.f, 1.f};
  float m_irradiance{1.f};
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::Light *, ANARI_LIGHT);

// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "Light.h"

namespace anari_webgpu {

Light::Light(WebGPUDeviceGlobalState *s) : Object(ANARI_LIGHT, s) {}

Light *Light::createInstance(
    std::string_view subtype, WebGPUDeviceGlobalState *s)
{
  if (subtype == "directional")
    return new DirectionalLight(s);
  return (Light *)new UnknownObject(ANARI_LIGHT, s);
}

// DirectionalLight ///////////////////////////////////////////////////////////

DirectionalLight::DirectionalLight(WebGPUDeviceGlobalState *s) : Light(s) {}

void DirectionalLight::commitParameters()
{
  m_direction = getParam<float3>("direction", float3(0.f, 0.f, -1.f));
  m_color = getParam<float3>("color", float3(1.f, 1.f, 1.f));
  m_irradiance = getParam<float>("irradiance", 1.f);
}

bool DirectionalLight::isValid() const
{
  return true;
}

float3 DirectionalLight::direction() const
{
  return m_direction;
}

float3 DirectionalLight::color() const
{
  return m_color;
}

float DirectionalLight::irradiance() const
{
  return m_irradiance;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Light *);

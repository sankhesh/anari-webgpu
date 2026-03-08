// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "Material.h"

namespace anari_webgpu {

Material::Material(WebGPUDeviceGlobalState *s) : Object(ANARI_MATERIAL, s) {}

Material *Material::createInstance(
    std::string_view subtype, WebGPUDeviceGlobalState *s)
{
  if (subtype == "matte")
    return new MatteMaterial(s);
  return (Material *)new UnknownObject(ANARI_MATERIAL, s);
}

// MatteMaterial //////////////////////////////////////////////////////////////

MatteMaterial::MatteMaterial(WebGPUDeviceGlobalState *s) : Material(s) {}

void MatteMaterial::commitParameters()
{
  m_color = getParam<float4>("color", float4(0.8f, 0.8f, 0.8f, 1.f));
}

bool MatteMaterial::isValid() const
{
  return true;
}

float4 MatteMaterial::color() const
{
  return m_color;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Material *);

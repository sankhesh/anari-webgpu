// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "Material.h"
#include <algorithm>

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
  m_colorSampler = getParamObject<Sampler>("color");

  m_opacity = getParam<float>("opacity", 1.f);
  m_opacitySampler = getParamObject<Sampler>("opacity");

  auto mode = getParam<std::string>("alphaMode", "opaque");
  if (mode == "blend")
    m_alphaMode = AlphaMode::BLEND;
  else if (mode == "mask")
    m_alphaMode = AlphaMode::MASK;
  else
    m_alphaMode = AlphaMode::OPAQUE;

  m_alphaCutoff = getParam<float>("alphaCutoff", 0.5f);
}

bool MatteMaterial::isValid() const
{
  return true;
}

float4 MatteMaterial::evalColor(float2 uv) const
{
  if (m_colorSampler && m_colorSampler->isValid())
    return m_colorSampler->eval(uv);
  return m_color;
}

float MatteMaterial::evalOpacity(float2 uv, float colorAlpha) const
{
  float op = m_opacity;
  if (m_opacitySampler && m_opacitySampler->isValid())
    op = m_opacitySampler->eval(uv).x;

  if (m_colorSampler && m_colorSampler->isValid())
    op *= colorAlpha;

  switch (m_alphaMode) {
  case AlphaMode::OPAQUE:
    return 1.f;
  case AlphaMode::BLEND:
    return std::max(0.f, std::min(1.f, op));
  case AlphaMode::MASK:
    return op >= m_alphaCutoff ? 1.f : 0.f;
  }
  return 1.f;
}

float4 MatteMaterial::color() const
{
  return m_color;
}

AlphaMode MatteMaterial::alphaMode() const
{
  return m_alphaMode;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Material *);

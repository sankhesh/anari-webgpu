// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "Material.h"
#include <algorithm>
#include <cmath>

namespace anari_webgpu {

Material::Material(WebGPUDeviceGlobalState *s) : Object(ANARI_MATERIAL, s) {}

Material *Material::createInstance(
    std::string_view subtype, WebGPUDeviceGlobalState *s)
{
  if (subtype == "matte")
    return new MatteMaterial(s);
  if (subtype == "physicallyBased")
    return new PhysicallyBasedMaterial(s);
  return (Material *)new UnknownObject(ANARI_MATERIAL, s);
}

void Material::commitSlangParameters()
{
  m_slangOverride.source = getParamString("slang.source", "");
  m_slangOverride.materialType = getParamString("slang.material", "");
  m_slangOverride.lightingType = getParamString("slang.lighting", "");
  m_slangOverride.normalType = getParamString("slang.normal", "");
  m_slangOverride.outputType = getParamString("slang.output", "");
  m_slangOverride.vertexType = getParamString("slang.vertex", "");
  m_slangOverride.hooksType = getParamString("slang.hooks", "");
  m_slangOverride.parameters.clear();
  const std::string prefix = "slang.";
  for (auto it = params_begin(); it != params_end(); ++it) {
    const std::string &name = it->first;
    if (name.compare(0, prefix.size(), prefix) != 0 || name == "slang.source"
        || name == "slang.material" || name == "slang.lighting"
        || name == "slang.normal" || name == "slang.output"
        || name == "slang.vertex" || name == "slang.hooks")
      continue;
    m_slangOverride.parameters.emplace_back(name.substr(prefix.size()), it->second);
  }
}

// MatteMaterial //////////////////////////////////////////////////////////////

MatteMaterial::MatteMaterial(WebGPUDeviceGlobalState *s) : Material(s) {}

void MatteMaterial::commitParameters()
{
  m_color = getParam<float4>("color", float4(0.8f, 0.8f, 0.8f, 1.f));
  m_colorSampler = getParamObject<Sampler>("color");

  m_opacity = getParam<float>("opacity", 1.f);
  m_opacitySampler = getParamObject<Sampler>("opacity");

  auto mode = getParamString("alphaMode", "opaque");
  if (mode == "blend")
    m_alphaMode = AlphaMode::BLEND;
  else if (mode == "mask")
    m_alphaMode = AlphaMode::MASK;
  else
    m_alphaMode = AlphaMode::OPAQUE;

  m_alphaCutoff = getParam<float>("alphaCutoff", 0.5f);
  commitSlangParameters();
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

PhysicallyBasedMaterial::PhysicallyBasedMaterial(WebGPUDeviceGlobalState *s)
    : Material(s)
{}

void PhysicallyBasedMaterial::commitParameters()
{
  {
    float4 tryVec4 = getParam<float4>("baseColor", float4(-1.f, -1.f, -1.f, -1.f));
    if (tryVec4.x >= 0.f) {
      m_baseColor = tryVec4;
    } else {
      float3 tryVec3 = getParam<float3>("baseColor", float3(0.8f, 0.8f, 0.8f));
      m_baseColor = float4(tryVec3.x, tryVec3.y, tryVec3.z, 1.f);
    }
  }
  m_baseColorSampler = getParamObject<Sampler>("baseColor");

  m_metallic = getParam<float>("metallic", 0.f);
  m_roughness = getParam<float>("roughness", 1.f);
  m_metallicRoughnessSampler = getParamObject<Sampler>("metallicRoughness");

  m_emissive = getParam<float3>("emissive", float3(0.f, 0.f, 0.f));
  m_emissiveStrength = getParam<float>("emissiveStrength", 1.f);
  m_emissiveSampler = getParamObject<Sampler>("emissive");

  m_normalSampler = getParamObject<Sampler>("normal");
  m_occlusionSampler = getParamObject<Sampler>("occlusion");

  m_opacity = getParam<float>("opacity", 1.f);

  auto mode = getParamString("alphaMode", "opaque");
  if (mode == "blend")
    m_alphaMode = AlphaMode::BLEND;
  else if (mode == "mask")
    m_alphaMode = AlphaMode::MASK;
  else
    m_alphaMode = AlphaMode::OPAQUE;

  m_alphaCutoff = getParam<float>("alphaCutoff", 0.5f);
  commitSlangParameters();
}

bool PhysicallyBasedMaterial::isValid() const
{
  return true;
}

float4 PhysicallyBasedMaterial::baseColor() const
{
  return m_baseColor;
}

float PhysicallyBasedMaterial::metallic() const
{
  return m_metallic;
}

float PhysicallyBasedMaterial::roughness() const
{
  return m_roughness;
}

float3 PhysicallyBasedMaterial::emissive() const
{
  return m_emissive;
}

float PhysicallyBasedMaterial::emissiveStrength() const
{
  return m_emissiveStrength;
}

float PhysicallyBasedMaterial::opacity() const
{
  return m_opacity;
}

AlphaMode PhysicallyBasedMaterial::alphaMode() const
{
  return m_alphaMode;
}

float PhysicallyBasedMaterial::alphaCutoff() const
{
  return m_alphaCutoff;
}

float4 PhysicallyBasedMaterial::evalBaseColor(float2 uv) const
{
  if (m_baseColorSampler && m_baseColorSampler->isValid())
    return m_baseColorSampler->eval(uv);
  return m_baseColor;
}

float2 PhysicallyBasedMaterial::evalMetallicRoughness(float2 uv) const
{
  if (m_metallicRoughnessSampler && m_metallicRoughnessSampler->isValid()) {
    float4 s = m_metallicRoughnessSampler->eval(uv);
    return float2(s.z, s.y);
  }
  return float2(m_metallic, m_roughness);
}

float3 PhysicallyBasedMaterial::evalEmissive(float2 uv) const
{
  if (m_emissiveSampler && m_emissiveSampler->isValid()) {
    float4 s = m_emissiveSampler->eval(uv);
    return float3(s.x, s.y, s.z) * m_emissiveStrength;
  }
  return m_emissive * m_emissiveStrength;
}

float PhysicallyBasedMaterial::evalOpacity(float2 uv, float baseAlpha) const
{
  float op = m_opacity * baseAlpha;

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

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Material *);

// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"
#include "sampler/Sampler.h"

#include <string>
#include <utility>
#include <vector>

namespace anari_webgpu {

struct SlangOverride
{
  std::string source;
  std::string materialType;
  std::string lightingType;
  std::string normalType;
  std::string outputType;
  std::string vertexType;
  std::vector<std::pair<std::string, helium::AnariAny>> parameters;

  bool active() const
  {
    return !materialType.empty() || !lightingType.empty() || !normalType.empty()
        || !outputType.empty() || !vertexType.empty();
  }
};

struct Material : public Object
{
  Material(WebGPUDeviceGlobalState *s);
  ~Material() override = default;
  static Material *createInstance(
      std::string_view subtype, WebGPUDeviceGlobalState *s);

  const SlangOverride &slangOverride() const { return m_slangOverride; }

 protected:
  void commitSlangParameters();

 private:
  SlangOverride m_slangOverride;
};

enum class AlphaMode { OPAQUE, BLEND, MASK };

struct MatteMaterial : public Material
{
  MatteMaterial(WebGPUDeviceGlobalState *s);
  void commitParameters() override;
  bool isValid() const override;

  float4 evalColor(float2 uv) const;

  float evalOpacity(float2 uv, float colorAlpha) const;

  float4 color() const;

  AlphaMode alphaMode() const;

 private:
  float4 m_color{0.8f, 0.8f, 0.8f, 1.f};
  helium::IntrusivePtr<Sampler> m_colorSampler;

  float m_opacity{1.f};
  helium::IntrusivePtr<Sampler> m_opacitySampler;

  AlphaMode m_alphaMode{AlphaMode::OPAQUE};
  float m_alphaCutoff{0.5f};
};

struct PhysicallyBasedMaterial : public Material
{
  PhysicallyBasedMaterial(WebGPUDeviceGlobalState *s);
  void commitParameters() override;
  bool isValid() const override;

  float4 baseColor() const;
  float metallic() const;
  float roughness() const;
  float3 emissive() const;
  float emissiveStrength() const;
  float opacity() const;
  AlphaMode alphaMode() const;
  float alphaCutoff() const;

  float4 evalBaseColor(float2 uv) const;
  float2 evalMetallicRoughness(float2 uv) const;
  float3 evalEmissive(float2 uv) const;
  float evalOpacity(float2 uv, float baseAlpha) const;

 private:
  float4 m_baseColor{0.8f, 0.8f, 0.8f, 1.f};
  float m_metallic{0.f};
  float m_roughness{1.f};
  float3 m_emissive{0.f, 0.f, 0.f};
  float m_emissiveStrength{1.f};
  float m_opacity{1.f};

  helium::IntrusivePtr<Sampler> m_baseColorSampler;
  helium::IntrusivePtr<Sampler> m_metallicRoughnessSampler;
  helium::IntrusivePtr<Sampler> m_normalSampler;
  helium::IntrusivePtr<Sampler> m_emissiveSampler;
  helium::IntrusivePtr<Sampler> m_occlusionSampler;

  AlphaMode m_alphaMode{AlphaMode::OPAQUE};
  float m_alphaCutoff{0.5f};
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::Material *, ANARI_MATERIAL);

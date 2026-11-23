// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"
#include "array/Array2D.h"

#include <string>
#include <vector>

namespace anari_webgpu {

struct Sampler : public Object
{
  Sampler(WebGPUDeviceGlobalState *d);
  virtual ~Sampler() = default;
  static Sampler *createInstance(
      std::string_view subtype, WebGPUDeviceGlobalState *d);

  virtual float4 eval(float4 attribute) const;
  float4 eval(float2 uv) const { return eval(float4(uv.x, uv.y, 0.f, 1.f)); }
};

enum class SamplerWrap : uint32_t { CLAMP_TO_EDGE, REPEAT, MIRROR_REPEAT };

struct Image2DSampler : public Sampler
{
  struct Params
  {
    mat4 inTransform{linalg::identity};
    float4 inOffset{0.f, 0.f, 0.f, 0.f};
    mat4 outTransform{linalg::identity};
    float4 outOffset{0.f, 0.f, 0.f, 0.f};
    bool linearFilter{true};
    SamplerWrap wrap1{SamplerWrap::CLAMP_TO_EDGE};
    SamplerWrap wrap2{SamplerWrap::CLAMP_TO_EDGE};
  };

  Image2DSampler(WebGPUDeviceGlobalState *s);
  void commitParameters() override;
  bool isValid() const override;

  using Sampler::eval;
  float4 eval(float4 attribute) const override;

  const std::string &inAttribute() const { return m_inAttribute; }
  const Params &params() const { return m_params; }
  uint32_t width() const { return uint32_t(m_width); }
  uint32_t height() const { return uint32_t(m_height); }
  std::vector<float4> texels() const;

 private:
  float4 texel(int32_t x, int32_t y) const;

  helium::IntrusivePtr<Array2D> m_image;
  std::string m_inAttribute{"attribute0"};
  Params m_params;
  uint64_t m_width{0};
  uint64_t m_height{0};
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::Sampler *, ANARI_SAMPLER);

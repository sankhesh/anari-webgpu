// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"
#include "array/Array2D.h"

namespace anari_webgpu {

struct Sampler : public Object
{
  Sampler(WebGPUDeviceGlobalState *d);
  virtual ~Sampler() = default;
  static Sampler *createInstance(
      std::string_view subtype, WebGPUDeviceGlobalState *d);

  virtual float4 eval(float2 uv) const;
};

struct Image2DSampler : public Sampler
{
  Image2DSampler(WebGPUDeviceGlobalState *s);
  void commitParameters() override;
  bool isValid() const override;

  float4 eval(float2 uv) const override;

 private:
  helium::IntrusivePtr<Array2D> m_image;
  std::string m_inAttribute{"attribute0"};
  ANARIDataType m_imageElementType{ANARI_UNKNOWN};
  uint64_t m_width{0};
  uint64_t m_height{0};
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::Sampler *, ANARI_SAMPLER);

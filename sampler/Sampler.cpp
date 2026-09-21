// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "Sampler.h"
#include <algorithm>
#include <cmath>

namespace anari_webgpu {

Sampler::Sampler(WebGPUDeviceGlobalState *s) : Object(ANARI_SAMPLER, s) {}

Sampler *Sampler::createInstance(
    std::string_view subtype, WebGPUDeviceGlobalState *s)
{
  if (subtype == "image2D")
    return new Image2DSampler(s);
  return (Sampler *)new UnknownObject(ANARI_SAMPLER, s);
}

float4 Sampler::eval(float2 ) const
{
  return float4(1.f, 1.f, 1.f, 1.f);
}

Image2DSampler::Image2DSampler(WebGPUDeviceGlobalState *s) : Sampler(s) {}

void Image2DSampler::commitParameters()
{
  m_image = getParamObject<Array2D>("image");
  m_inAttribute = getParamString("inAttribute", "attribute0");

  if (!m_image) {
    reportMessage(
        ANARI_SEVERITY_WARNING, "missing 'image' on image2D sampler");
    m_width = m_height = 0;
    m_imageElementType = ANARI_UNKNOWN;
    return;
  }

  m_width = m_image->size(0);
  m_height = m_image->size(1);
  m_imageElementType = m_image->elementType();
}

bool Image2DSampler::isValid() const
{
  return bool(m_image);
}

float4 Image2DSampler::eval(float2 uv) const
{
  if (!m_image || m_width == 0 || m_height == 0)
    return float4(1.f, 1.f, 1.f, 1.f);

  float u = std::max(0.f, std::min(1.f, uv.x));
  float v = std::max(0.f, std::min(1.f, uv.y));

  uint64_t px = std::min((uint64_t)(u * m_width), m_width - 1);
  uint64_t py = std::min((uint64_t)(v * m_height), m_height - 1);
  uint64_t idx = py * m_width + px;

  const auto *data = m_image->data();

  if (m_imageElementType == ANARI_FLOAT32_VEC4) {
    const float *p = (const float *)data + idx * 4;
    return float4(p[0], p[1], p[2], p[3]);
  } else if (m_imageElementType == ANARI_FLOAT32_VEC3) {
    const float *p = (const float *)data + idx * 3;
    return float4(p[0], p[1], p[2], 1.f);
  } else if (m_imageElementType == ANARI_UFIXED8_VEC4) {
    const uint8_t *p = (const uint8_t *)data + idx * 4;
    return float4(p[0] / 255.f, p[1] / 255.f, p[2] / 255.f, p[3] / 255.f);
  } else if (m_imageElementType == ANARI_UFIXED8_RGBA_SRGB) {
    const uint8_t *p = (const uint8_t *)data + idx * 4;
    auto srgbToLinear = [](float c) {
      return c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
    };
    return float4(srgbToLinear(p[0] / 255.f),
        srgbToLinear(p[1] / 255.f),
        srgbToLinear(p[2] / 255.f),
        p[3] / 255.f);
  }

  return float4(1.f, 1.f, 1.f, 1.f);
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Sampler *);

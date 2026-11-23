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

float4 Sampler::eval(float4) const
{
  return float4(1.f, 1.f, 1.f, 1.f);
}

static SamplerWrap wrapFromString(const std::string &s)
{
  if (s == "repeat")
    return SamplerWrap::REPEAT;
  if (s == "mirrorRepeat")
    return SamplerWrap::MIRROR_REPEAT;
  return SamplerWrap::CLAMP_TO_EDGE;
}

static int32_t wrapIndex(int32_t i, int32_t size, SamplerWrap wrap)
{
  switch (wrap) {
  case SamplerWrap::REPEAT:
    return ((i % size) + size) % size;
  case SamplerWrap::MIRROR_REPEAT: {
    const int32_t x = std::abs(i + (i < 0)) % (2 * size);
    return x >= size ? 2 * size - x - 1 : x;
  }
  default:
    return std::clamp(i, 0, size - 1);
  }
}

Image2DSampler::Image2DSampler(WebGPUDeviceGlobalState *s) : Sampler(s) {}

void Image2DSampler::commitParameters()
{
  m_image = getParamObject<Array2D>("image");
  m_inAttribute = getParamString("inAttribute", "attribute0");
  m_params.linearFilter = getParamString("filter", "linear") != "nearest";
  m_params.wrap1 = wrapFromString(getParamString("wrapMode1", "clampToEdge"));
  m_params.wrap2 = wrapFromString(getParamString("wrapMode2", "clampToEdge"));
  m_params.inTransform = getParam<mat4>("inTransform", mat4(linalg::identity));
  m_params.inOffset = getParam<float4>("inOffset", float4(0.f, 0.f, 0.f, 0.f));
  m_params.outTransform =
      getParam<mat4>("outTransform", mat4(linalg::identity));
  m_params.outOffset =
      getParam<float4>("outOffset", float4(0.f, 0.f, 0.f, 0.f));

  if (!m_image) {
    reportMessage(
        ANARI_SEVERITY_WARNING, "missing 'image' on image2D sampler");
    m_width = m_height = 0;
    return;
  }

  m_width = m_image->size(0);
  m_height = m_image->size(1);
}

bool Image2DSampler::isValid() const
{
  return m_image && m_width > 0 && m_height > 0;
}

float4 Image2DSampler::texel(int32_t x, int32_t y) const
{
  const size_t i = size_t(wrapIndex(y, int32_t(m_height), m_params.wrap2))
          * m_width
      + size_t(wrapIndex(x, int32_t(m_width), m_params.wrap1));
  return readAsAttributeValueFlat(m_image->data(), m_image->elementType(), i);
}

float4 Image2DSampler::eval(float4 attribute) const
{
  if (!isValid())
    return float4(1.f, 1.f, 1.f, 1.f);

  const float4 a =
      linalg::mul(m_params.inTransform, attribute) + m_params.inOffset;
  const float x =
      m_params.wrap1 == SamplerWrap::REPEAT ? a.x - std::floor(a.x) : a.x;
  const float y =
      m_params.wrap2 == SamplerWrap::REPEAT ? a.y - std::floor(a.y) : a.y;

  const auto ix = getInterpolant(x, m_width, true);
  const auto iy = getInterpolant(y, m_height, true);
  const float4 v00 = texel(ix.lower, iy.lower);
  const float4 v01 = texel(ix.lower, iy.upper);
  const float4 v10 = texel(ix.upper, iy.lower);
  const float4 v11 = texel(ix.upper, iy.upper);

  float4 v;
  if (m_params.linearFilter) {
    v = linalg::lerp(
        linalg::lerp(v00, v01, iy.frac), linalg::lerp(v10, v11, iy.frac), ix.frac);
  } else {
    const float4 v0 = iy.frac < 0.5f ? v00 : v01;
    const float4 v1 = iy.frac < 0.5f ? v10 : v11;
    v = ix.frac < 0.5f ? v0 : v1;
  }
  return linalg::mul(m_params.outTransform, v) + m_params.outOffset;
}

std::vector<float4> Image2DSampler::texels() const
{
  std::vector<float4> t;
  if (!isValid())
    return t;
  t.resize(m_width * m_height);
  for (size_t i = 0; i < t.size(); i++)
    t[i] = readAsAttributeValueFlat(m_image->data(), m_image->elementType(), i);
  return t;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Sampler *);

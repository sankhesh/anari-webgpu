// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "Volume.h"
#include <algorithm>
#include <cmath>

namespace anari_webgpu {

Volume::Volume(WebGPUDeviceGlobalState *s) : Object(ANARI_VOLUME, s) {}

Volume *Volume::createInstance(
    std::string_view subtype, WebGPUDeviceGlobalState *s)
{
  if (subtype == "transferFunction1D")
    return new TransferFunction1DVolume(s);
  return (Volume *)new UnknownObject(ANARI_VOLUME, s);
}

void Volume::commitParameters()
{
  m_id = getParam<uint32_t>("id", ~0u);
}

uint32_t Volume::id() const
{
  return m_id;
}

TransferFunction1DVolume::TransferFunction1DVolume(WebGPUDeviceGlobalState *s)
    : Volume(s)
{}

void TransferFunction1DVolume::commitParameters()
{
  Volume::commitParameters();

  m_field = getParamObject<SpatialField>("value");

  m_valueRange = float2(0.f, 1.f);
  getParam("valueRange", ANARI_FLOAT32_BOX1, &m_valueRange);
  double valueRange[2] = {0.0, 1.0};
  if (getParam("valueRange", ANARI_FLOAT64_BOX1, valueRange))
    m_valueRange = float2(float(valueRange[0]), float(valueRange[1]));

  m_colorData = getParamObject<Array1D>("color");
  m_uniformColor = float4(1.f, 1.f, 1.f, 1.f);
  getParam("color", ANARI_FLOAT32_VEC3, &m_uniformColor);
  getParam("color", ANARI_FLOAT32_VEC4, &m_uniformColor);

  m_opacityData = getParamObject<Array1D>("opacity");
  m_uniformOpacity = getParam<float>("opacity", 1.f);
  m_unitDistance = getParam<float>("unitDistance", 1.f);
}

void TransferFunction1DVolume::finalize()
{
  if (!m_field) {
    reportMessage(ANARI_SEVERITY_WARNING,
        "missing required parameter 'value' on transferFunction1D volume");
  }

  m_colors.clear();
  if (m_colorData) {
    const size_t n = m_colorData->size();
    const auto type = m_colorData->elementType();
    for (size_t i = 0; i < n; i++) {
      if (type == ANARI_FLOAT32_VEC3) {
        const float3 c = m_colorData->beginAs<float3>()[i];
        m_colors.push_back(float4(c.x, c.y, c.z, 1.f));
      } else if (type == ANARI_FLOAT32_VEC4) {
        m_colors.push_back(m_colorData->beginAs<float4>()[i]);
      } else {
        m_colors.push_back(float4(0.f, 0.f, 0.f, 0.f));
      }
    }
  }
  if (m_colors.empty())
    m_colors.push_back(m_uniformColor);

  m_opacities.clear();
  if (m_opacityData && m_opacityData->elementType() == ANARI_FLOAT32) {
    const float *o = m_opacityData->beginAs<float>();
    m_opacities.assign(o, o + m_opacityData->size());
  }
  if (m_opacities.empty())
    m_opacities.push_back(m_uniformOpacity);
}

bool TransferFunction1DVolume::isValid() const
{
  return m_field && m_field->isValid() && field();
}

const StructuredRegularField *TransferFunction1DVolume::field() const
{
  return dynamic_cast<const StructuredRegularField *>(m_field.ptr);
}

float2 TransferFunction1DVolume::valueRange() const
{
  return m_valueRange;
}

float TransferFunction1DVolume::unitDistance() const
{
  return m_unitDistance;
}

float TransferFunction1DVolume::normalized(float value) const
{
  return std::clamp(
      (value - m_valueRange.x) / (m_valueRange.y - m_valueRange.x), 0.f, 1.f);
}

template <typename T>
static T sampleTable(const std::vector<T> &table, float t)
{
  const auto i = getInterpolant(t, table.size(), false);
  const int32_t last = int32_t(table.size()) - 1;
  const T &a = table[std::clamp(i.lower, 0, last)];
  const T &b = table[std::clamp(i.upper, 0, last)];
  return a + (b - a) * i.frac;
}

float4 TransferFunction1DVolume::colorOf(float t) const
{
  return sampleTable(m_colors, t);
}

float TransferFunction1DVolume::opacityOf(float t) const
{
  return sampleTable(m_opacities, t) * colorOf(t).w;
}

const std::vector<float4> &TransferFunction1DVolume::colors() const
{
  return m_colors;
}

const std::vector<float> &TransferFunction1DVolume::opacities() const
{
  return m_opacities;
}

template <typename F>
static float2 extremesOver(float a, float b, size_t knots, F f)
{
  float lo = std::min(f(a), f(b));
  float hi = std::max(f(a), f(b));
  if (knots > 1) {
    const float scale = float(knots - 1);
    for (size_t k = size_t(std::ceil(a * scale)); k < knots && k <= b * scale;
         k++) {
      lo = std::min(lo, f(k / scale));
      hi = std::max(hi, f(k / scale));
    }
  }
  return float2(lo, hi);
}

std::vector<float2> TransferFunction1DVolume::opacityExtremes() const
{
  const uint32_t n = kExtremesSize;
  std::vector<float2> table(size_t(n) * kExtremesLevels);
  auto opacity = [&](float t) { return sampleTable(m_opacities, t); };
  auto alpha = [&](float t) { return colorOf(t).w; };
  for (uint32_t e = 0; e < n; e++) {
    const float a = float(e) / n, b = float(e + 1) / n;
    const float2 o = extremesOver(a, b, m_opacities.size(), opacity);
    const float2 w = extremesOver(a, b, m_colors.size(), alpha);
    table[e] = float2(o.x * w.x, o.y * w.y);
  }
  for (uint32_t level = 1; level < kExtremesLevels; level++) {
    const uint32_t half = 1u << (level - 1);
    for (uint32_t e = 0; e < n; e++) {
      const float2 lo = table[size_t(level - 1) * n + e];
      const float2 hi = table[size_t(level - 1) * n + std::min(e + half, n - 1)];
      table[size_t(level) * n + e] =
          float2(std::min(lo.x, hi.x), std::max(lo.y, hi.y));
    }
  }
  return table;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Volume *);

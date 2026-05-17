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

  m_field = getParamObject<SpatialField>("field");

  m_color = getParamObject<Array1D>("color");
  m_opacity = getParamObject<Array1D>("opacity");

  m_valueRange = getParam<float2>("valueRange", float2(0.f, 1.f));
  m_densityScale = getParam<float>("densityScale", 1.f);
  m_unitDistance = getParam<float>("unitDistance", 1.f);

  if (!m_field) {
    reportMessage(ANARI_SEVERITY_WARNING,
        "missing required parameter 'field' on transferFunction1D volume");
  }
}

bool TransferFunction1DVolume::isValid() const
{
  if (!m_field || !m_field->isValid())
    return false;
  return true;
}

const StructuredRegularField *TransferFunction1DVolume::field() const
{
  return dynamic_cast<const StructuredRegularField *>(m_field.ptr);
}

float2 TransferFunction1DVolume::valueRange() const
{
  return m_valueRange;
}

float TransferFunction1DVolume::densityScale() const
{
  return m_densityScale;
}

float TransferFunction1DVolume::unitDistance() const
{
  return m_unitDistance;
}

uint32_t TransferFunction1DVolume::tfSize() const
{
  if (m_color)
    return (uint32_t)m_color->totalSize();
  if (m_opacity)
    return (uint32_t)m_opacity->totalSize();
  return 0u;
}

float4 TransferFunction1DVolume::transferFunction(float t) const
{
  t = std::max(0.f, std::min(1.f, t));

  float3 rgb{1.f, 1.f, 1.f};
  float a{1.f};

  if (m_color && m_color->totalSize() > 0) {
    uint32_t n = (uint32_t)m_color->totalSize();
    float fidx = t * float(n - 1);
    uint32_t i0 = (uint32_t)fidx;
    uint32_t i1 = std::min(i0 + 1u, n - 1u);
    float frac = fidx - float(i0);

    const float *col = reinterpret_cast<const float *>(m_color->begin());
    float3 c0{col[i0 * 3], col[i0 * 3 + 1], col[i0 * 3 + 2]};
    float3 c1{col[i1 * 3], col[i1 * 3 + 1], col[i1 * 3 + 2]};
    rgb = float3(
        c0.x * (1.f - frac) + c1.x * frac,
        c0.y * (1.f - frac) + c1.y * frac,
        c0.z * (1.f - frac) + c1.z * frac);
  }

  if (m_opacity && m_opacity->totalSize() > 0) {
    uint32_t n = (uint32_t)m_opacity->totalSize();
    float fidx = t * float(n - 1);
    uint32_t i0 = (uint32_t)fidx;
    uint32_t i1 = std::min(i0 + 1u, n - 1u);
    float frac = fidx - float(i0);

    const float *op = reinterpret_cast<const float *>(m_opacity->begin());
    a = op[i0] * (1.f - frac) + op[i1] * frac;
  }

  return float4(rgb.x, rgb.y, rgb.z, a);
}

std::vector<float4> TransferFunction1DVolume::buildTFTable(
    uint32_t resolution) const
{
  std::vector<float4> table(resolution);
  for (uint32_t i = 0; i < resolution; i++) {
    float t = float(i) / float(resolution - 1);
    table[i] = transferFunction(t);
  }
  return table;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Volume *);

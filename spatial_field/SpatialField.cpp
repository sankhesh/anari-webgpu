// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "SpatialField.h"
#include <algorithm>
#include <cmath>

namespace anari_webgpu {

SpatialField::SpatialField(WebGPUDeviceGlobalState *s)
    : Object(ANARI_SPATIAL_FIELD, s)
{}

SpatialField *SpatialField::createInstance(
    std::string_view subtype, WebGPUDeviceGlobalState *s)
{
  if (subtype == "structuredRegular")
    return new StructuredRegularField(s);
  return (SpatialField *)new UnknownObject(ANARI_SPATIAL_FIELD, s);
}

StructuredRegularField::StructuredRegularField(WebGPUDeviceGlobalState *s)
    : SpatialField(s)
{}

void StructuredRegularField::commitParameters()
{
  m_data = getParamObject<Array3D>("data");

  m_origin = getParam<float3>("origin", float3(0.f, 0.f, 0.f));
  m_spacing = getParam<float3>("spacing", float3(1.f, 1.f, 1.f));

  if (m_data) {
    m_dims = uint3(
        (uint32_t)m_data->size(0),
        (uint32_t)m_data->size(1),
        (uint32_t)m_data->size(2));
  } else {
    m_dims = uint3(0u, 0u, 0u);
    reportMessage(ANARI_SEVERITY_WARNING,
        "missing required parameter 'data' on structuredRegular field");
  }
}

bool StructuredRegularField::isValid() const
{
  return bool(m_data)
      && m_dims.x > 0 && m_dims.y > 0 && m_dims.z > 0;
}

float StructuredRegularField::sample(float3 uvw) const
{
  if (!m_data || m_dims.x == 0)
    return 0.f;

  const float *src = rawData();
  if (!src)
    return 0.f;

  float u = std::max(0.f, std::min(1.f, uvw.x));
  float v = std::max(0.f, std::min(1.f, uvw.y));
  float w = std::max(0.f, std::min(1.f, uvw.z));

  float cx = u * float(m_dims.x - 1);
  float cy = v * float(m_dims.y - 1);
  float cz = w * float(m_dims.z - 1);

  uint32_t x0 = (uint32_t)cx;
  uint32_t y0 = (uint32_t)cy;
  uint32_t z0 = (uint32_t)cz;
  uint32_t x1 = std::min(x0 + 1u, m_dims.x - 1u);
  uint32_t y1 = std::min(y0 + 1u, m_dims.y - 1u);
  uint32_t z1 = std::min(z0 + 1u, m_dims.z - 1u);

  float fx = cx - float(x0);
  float fy = cy - float(y0);
  float fz = cz - float(z0);

  uint32_t dxy = m_dims.x * m_dims.y;
  auto idx = [&](uint32_t xi, uint32_t yi, uint32_t zi) -> uint32_t {
    return xi + yi * m_dims.x + zi * dxy;
  };

  float v000 = src[idx(x0, y0, z0)];
  float v100 = src[idx(x1, y0, z0)];
  float v010 = src[idx(x0, y1, z0)];
  float v110 = src[idx(x1, y1, z0)];
  float v001 = src[idx(x0, y0, z1)];
  float v101 = src[idx(x1, y0, z1)];
  float v011 = src[idx(x0, y1, z1)];
  float v111 = src[idx(x1, y1, z1)];

  float v00 = v000 * (1.f - fx) + v100 * fx;
  float v10 = v010 * (1.f - fx) + v110 * fx;
  float v01 = v001 * (1.f - fx) + v101 * fx;
  float v11 = v011 * (1.f - fx) + v111 * fx;
  float v0  = v00  * (1.f - fy) + v10  * fy;
  float v1  = v01  * (1.f - fy) + v11  * fy;
  return v0 * (1.f - fz) + v1 * fz;
}

float StructuredRegularField::sampleWorld(float3 worldPos) const
{
  if (m_dims.x == 0)
    return 0.f;
  float3 ext = float3(
      float(m_dims.x - 1) * m_spacing.x,
      float(m_dims.y - 1) * m_spacing.y,
      float(m_dims.z - 1) * m_spacing.z);

  float3 uvw = float3(
      (worldPos.x - m_origin.x) / ext.x,
      (worldPos.y - m_origin.y) / ext.y,
      (worldPos.z - m_origin.z) / ext.z);
  return sample(uvw);
}

float3 StructuredRegularField::origin() const
{
  return m_origin;
}

float3 StructuredRegularField::spacing() const
{
  return m_spacing;
}

uint3 StructuredRegularField::dims() const
{
  return m_dims;
}

float3 StructuredRegularField::worldMin() const
{
  return m_origin;
}

float3 StructuredRegularField::worldMax() const
{
  return float3(
      m_origin.x + float(m_dims.x - 1) * m_spacing.x,
      m_origin.y + float(m_dims.y - 1) * m_spacing.y,
      m_origin.z + float(m_dims.z - 1) * m_spacing.z);
}

const float *StructuredRegularField::rawData() const
{
  if (!m_data)
    return nullptr;
  return reinterpret_cast<const float *>(m_data->data());
}

uint64_t StructuredRegularField::numVoxels() const
{
  return uint64_t(m_dims.x) * m_dims.y * m_dims.z;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::SpatialField *);

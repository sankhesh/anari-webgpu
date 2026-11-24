// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "SpatialField.h"

#include <algorithm>
#include <cmath>
#include <limits>

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
  m_nearest = getParamString("filter", "linear") == "nearest";
}

static float readVoxel(const void *data, ANARIDataType type, size_t i)
{
  switch (type) {
  case ANARI_FLOAT32:
    return static_cast<const float *>(data)[i];
  case ANARI_FLOAT64:
    return float(static_cast<const double *>(data)[i]);
  case ANARI_UFIXED8:
    return static_cast<const uint8_t *>(data)[i]
        / float(std::numeric_limits<uint8_t>::max());
  case ANARI_UFIXED16:
    return static_cast<const uint16_t *>(data)[i]
        / float(std::numeric_limits<uint16_t>::max());
  case ANARI_FIXED16:
    return static_cast<const int16_t *>(data)[i]
        / float(std::numeric_limits<int16_t>::max());
  default:
    return 0.f;
  }
}

void StructuredRegularField::finalize()
{
  m_values.clear();
  m_cellRanges.clear();
  m_dims = uint3(0u, 0u, 0u);
  m_cellDims = uint3(0u, 0u, 0u);
  if (!m_data) {
    reportMessage(ANARI_SEVERITY_WARNING,
        "missing required parameter 'data' on structuredRegular field");
    return;
  }

  m_dims = uint3(uint32_t(m_data->size(0)),
      uint32_t(m_data->size(1)),
      uint32_t(m_data->size(2)));
  const size_t count = size_t(m_dims.x) * m_dims.y * m_dims.z;
  m_values.resize(count);
  for (size_t i = 0; i < count; i++)
    m_values[i] = readVoxel(m_data->data(), m_data->elementType(), i);

  auto cells = [](uint32_t d) {
    return std::max(1u, (std::max(d, 2u) - 2u) / kCellSize + 1u);
  };
  m_cellDims = uint3(cells(m_dims.x), cells(m_dims.y), cells(m_dims.z));
  m_cellRanges.assign(size_t(m_cellDims.x) * m_cellDims.y * m_cellDims.z,
      float2(std::numeric_limits<float>::max(),
          std::numeric_limits<float>::lowest()));
  for (uint32_t z = 0; z < m_dims.z; z++) {
    for (uint32_t y = 0; y < m_dims.y; y++) {
      for (uint32_t x = 0; x < m_dims.x; x++) {
        const float v = valueAt(x, y, z);
        const uint32_t cx0 = x ? (x - 1) / kCellSize : 0;
        const uint32_t cy0 = y ? (y - 1) / kCellSize : 0;
        const uint32_t cz0 = z ? (z - 1) / kCellSize : 0;
        const uint32_t cx1 = std::min(x / kCellSize, m_cellDims.x - 1);
        const uint32_t cy1 = std::min(y / kCellSize, m_cellDims.y - 1);
        const uint32_t cz1 = std::min(z / kCellSize, m_cellDims.z - 1);
        for (uint32_t cz = cz0; cz <= cz1; cz++) {
          for (uint32_t cy = cy0; cy <= cy1; cy++) {
            for (uint32_t cx = cx0; cx <= cx1; cx++) {
              float2 &r = m_cellRanges[(size_t(cz) * m_cellDims.y + cy)
                      * m_cellDims.x
                  + cx];
              r.x = std::min(r.x, v);
              r.y = std::max(r.y, v);
            }
          }
        }
      }
    }
  }
}

bool StructuredRegularField::isValid() const
{
  return m_data && !m_values.empty();
}

float StructuredRegularField::valueAt(uint32_t x, uint32_t y, uint32_t z) const
{
  return m_values[(size_t(z) * m_dims.y + y) * m_dims.x + x];
}

float StructuredRegularField::sampleVoxel(float3 voxel) const
{
  if (!isValid())
    return 0.f;
  const float3 upper = float3(float(m_dims.x - 1),
      float(m_dims.y - 1),
      float(m_dims.z - 1));
  const float3 local = linalg::clamp(voxel, float3(0.f, 0.f, 0.f), upper);
  if (m_nearest) {
    return valueAt(uint32_t(std::round(local.x)),
        uint32_t(std::round(local.y)),
        uint32_t(std::round(local.z)));
  }
  const uint32_t x0 = uint32_t(local.x), y0 = uint32_t(local.y),
                 z0 = uint32_t(local.z);
  const uint32_t x1 = std::min(x0 + 1u, m_dims.x - 1u);
  const uint32_t y1 = std::min(y0 + 1u, m_dims.y - 1u);
  const uint32_t z1 = std::min(z0 + 1u, m_dims.z - 1u);
  const float fx = local.x - x0, fy = local.y - y0, fz = local.z - z0;
  auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };
  const float v00 = lerp(valueAt(x0, y0, z0), valueAt(x1, y0, z0), fx);
  const float v10 = lerp(valueAt(x0, y1, z0), valueAt(x1, y1, z0), fx);
  const float v01 = lerp(valueAt(x0, y0, z1), valueAt(x1, y0, z1), fx);
  const float v11 = lerp(valueAt(x0, y1, z1), valueAt(x1, y1, z1), fx);
  return lerp(lerp(v00, v10, fy), lerp(v01, v11, fy), fz);
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
  return m_origin
      + float3(float(m_dims.x ? m_dims.x - 1 : 0),
            float(m_dims.y ? m_dims.y - 1 : 0),
            float(m_dims.z ? m_dims.z - 1 : 0))
      * m_spacing;
}

bool StructuredRegularField::nearestFilter() const
{
  return m_nearest;
}

float StructuredRegularField::stepSize() const
{
  return std::min({m_spacing.x, m_spacing.y, m_spacing.z}) / 2.f;
}

const std::vector<float> &StructuredRegularField::values() const
{
  return m_values;
}

uint3 StructuredRegularField::cellDims() const
{
  return m_cellDims;
}

const std::vector<float2> &StructuredRegularField::cellRanges() const
{
  return m_cellRanges;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::SpatialField *);

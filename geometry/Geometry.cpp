// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "Geometry.h"
// std
#include <algorithm>
#include <iterator>

namespace anari_webgpu {

Geometry::Geometry(WebGPUDeviceGlobalState *s) : Object(ANARI_GEOMETRY, s) {}

Geometry *Geometry::createInstance(
    std::string_view subtype, WebGPUDeviceGlobalState *s)
{
  if (subtype == "triangle")
    return new TriangleGeometry(s);
  return (Geometry *)new UnknownObject(ANARI_GEOMETRY, s);
}

// TriangleGeometry ///////////////////////////////////////////////////////////

static const char *kAttributeNames[5] = {
    "color", "attribute0", "attribute1", "attribute2", "attribute3"};

TriangleGeometry::TriangleGeometry(WebGPUDeviceGlobalState *s) : Geometry(s) {}

void TriangleGeometry::commitParameters()
{
  m_vertex_position = getParamObject<Array1D>("vertex.position");
  m_vertex_normal = getParamObject<Array1D>("vertex.normal");
  for (int i = 0; i < 5; i++) {
    const std::string name = kAttributeNames[i];
    m_vertex_attributes[i] = getParamObject<Array1D>("vertex." + name);
    m_primitive_attributes[i] = getParamObject<Array1D>("primitive." + name);
  }
  m_index = getParamObject<Array1D>("primitive.index");

  if (!m_vertex_position) {
    reportMessage(
        ANARI_SEVERITY_WARNING, "missing 'vertex.position' on triangle geometry");
  }
}

bool TriangleGeometry::isValid() const
{
  return m_vertex_position;
}

const float *TriangleGeometry::vertexPositions() const
{
  return m_vertex_position ? (const float *)m_vertex_position->data() : nullptr;
}

uint32_t TriangleGeometry::numVertices() const
{
  return m_vertex_position ? (uint32_t)m_vertex_position->totalSize() : 0;
}

static std::vector<float4> readColors(const Array1D *array, size_t count)
{
  std::vector<float4> colors(count, DEFAULT_ATTRIBUTE_VALUE);
  if (array) {
    const size_t n = std::min(count, array->size());
    for (size_t i = 0; i < n; i++)
      colors[i] =
          readAsAttributeValueFlat(array->begin(), array->elementType(), i);
  }
  return colors;
}

std::vector<float4> TriangleGeometry::attribute(
    const std::string &name, bool &perPrimitive) const
{
  perPrimitive = false;
  const auto it =
      std::find(std::begin(kAttributeNames), std::end(kAttributeNames), name);
  if (it == std::end(kAttributeNames))
    return readColors(nullptr, numVertices());
  const size_t i = it - std::begin(kAttributeNames);
  if (m_vertex_attributes[i])
    return readColors(m_vertex_attributes[i].ptr, numVertices());
  if (m_primitive_attributes[i]) {
    perPrimitive = true;
    return readColors(m_primitive_attributes[i].ptr, numTriangles());
  }
  return readColors(nullptr, numVertices());
}

const float *TriangleGeometry::vertexNormals() const
{
  return m_vertex_normal ? (const float *)m_vertex_normal->data() : nullptr;
}

const uint32_t *TriangleGeometry::indices() const
{
  return m_index ? (const uint32_t *)m_index->data() : nullptr;
}

uint32_t TriangleGeometry::numIndices() const
{
  return m_index ? (uint32_t)m_index->totalSize() : 0;
}

uint32_t TriangleGeometry::numTriangles() const
{
  if (m_index)
    return numIndices();
  return numVertices() / 3;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Geometry *);

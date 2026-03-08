// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "Geometry.h"

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

TriangleGeometry::TriangleGeometry(WebGPUDeviceGlobalState *s) : Geometry(s) {}

void TriangleGeometry::commitParameters()
{
  m_vertex_position = getParamObject<Array1D>("vertex.position");
  m_vertex_color = getParamObject<Array1D>("vertex.color");
  m_vertex_normal = getParamObject<Array1D>("vertex.normal");
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
  return m_vertex_position ? (const float *)m_vertex_position->begin() : nullptr;
}

uint32_t TriangleGeometry::numVertices() const
{
  return m_vertex_position ? (uint32_t)m_vertex_position->totalSize() : 0;
}

const float *TriangleGeometry::vertexColors() const
{
  return m_vertex_color ? (const float *)m_vertex_color->begin() : nullptr;
}

const float *TriangleGeometry::vertexNormals() const
{
  return m_vertex_normal ? (const float *)m_vertex_normal->begin() : nullptr;
}

const uint32_t *TriangleGeometry::indices() const
{
  return m_index ? (const uint32_t *)m_index->begin() : nullptr;
}

uint32_t TriangleGeometry::numIndices() const
{
  return m_index ? (uint32_t)m_index->totalSize() : 0;
}

uint32_t TriangleGeometry::numTriangles() const
{
  if (m_index)
    return numIndices() / 3;
  return numVertices() / 3;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Geometry *);

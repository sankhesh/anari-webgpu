// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"
#include "array/Array1D.h"
// std
#include <vector>

namespace anari_webgpu {

struct Geometry : public Object
{
  Geometry(WebGPUDeviceGlobalState *s);
  ~Geometry() override = default;
  static Geometry *createInstance(
      std::string_view subtype, WebGPUDeviceGlobalState *s);
};

struct TriangleGeometry : public Geometry
{
  TriangleGeometry(WebGPUDeviceGlobalState *s);
  void commitParameters() override;
  bool isValid() const override;

  const float *vertexPositions() const;
  uint32_t numVertices() const;
  const float *vertexColors() const;
  const float *vertexNormals() const;
  const uint32_t *indices() const;
  uint32_t numIndices() const;
  uint32_t numTriangles() const;

 private:
  helium::IntrusivePtr<Array1D> m_vertex_position;
  helium::IntrusivePtr<Array1D> m_vertex_color;
  helium::IntrusivePtr<Array1D> m_vertex_normal;
  helium::IntrusivePtr<Array1D> m_index;
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::Geometry *, ANARI_GEOMETRY);

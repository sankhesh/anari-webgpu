// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "Surface.h"

namespace anari_webgpu {

Surface::Surface(WebGPUDeviceGlobalState *s) : Object(ANARI_SURFACE, s) {}

void Surface::commitParameters()
{
  m_id = getParam<uint32_t>("id", ~0u);
  m_geometry = getParamObject<Geometry>("geometry");
  m_material = getParamObject<Material>("material");

  if (!m_material)
    reportMessage(ANARI_SEVERITY_WARNING, "missing 'material' on ANARISurface");
  if (!m_geometry)
    reportMessage(ANARI_SEVERITY_WARNING, "missing 'geometry' on ANARISurface");
}

uint32_t Surface::id() const
{
  return m_id;
}

const Geometry *Surface::geometry() const
{
  return m_geometry.ptr;
}

const Material *Surface::material() const
{
  return m_material.ptr;
}

bool Surface::isValid() const
{
  return m_geometry && m_material && m_geometry->isValid()
      && m_material->isValid();
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Surface *);

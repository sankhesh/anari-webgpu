// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "Group.h"
#include <iterator>

namespace anari_webgpu {

Group::Group(WebGPUDeviceGlobalState *s)
    : Object(ANARI_GROUP, s),
      m_surfaceData(this),
      m_volumeData(this),
      m_lightData(this)
{}

Group::~Group() = default;

bool Group::getProperty(const std::string_view &name,
    ANARIDataType type,
    void *ptr,
    uint64_t size,
    uint32_t flags)
{
  if (name == "bounds" && type == ANARI_FLOAT32_BOX3) {
    const box3 b = bounds();
    if (b.lower.x > b.upper.x)
      return false;
    std::memcpy(ptr, &b, sizeof(b));
    return true;
  }
  return Object::getProperty(name, type, ptr, size, flags);
}

void Group::commitParameters()
{
  m_surfaceData = getParamObject<ObjectArray>("surface");
  m_volumeData = getParamObject<ObjectArray>("volume");
  m_lightData = getParamObject<ObjectArray>("light");
}

void Group::finalize()
{
  m_surfaces.clear();
  m_volumes.clear();
  m_lights.clear();

  if (m_surfaceData) {
    std::transform(m_surfaceData->handlesBegin(),
        m_surfaceData->handlesEnd(),
        std::back_inserter(m_surfaces),
        [](auto *o) { return (Surface *)o; });
  }

  if (m_volumeData) {
    std::transform(m_volumeData->handlesBegin(),
        m_volumeData->handlesEnd(),
        std::back_inserter(m_volumes),
        [](auto *o) { return (Volume *)o; });
  }

  if (m_lightData) {
    std::transform(m_lightData->handlesBegin(),
        m_lightData->handlesEnd(),
        std::back_inserter(m_lights),
        [](auto *o) { return (Light *)o; });
  }
}

const std::vector<Surface *> &Group::surfaces() const
{
  return m_surfaces;
}

const std::vector<Volume *> &Group::volumes() const
{
  return m_volumes;
}

const std::vector<Light *> &Group::lights() const
{
  return m_lights;
}

box3 Group::bounds() const
{
  box3 b;
  for (auto *s : m_surfaces) {
    auto *geom =
        s ? dynamic_cast<const TriangleGeometry *>(s->geometry()) : nullptr;
    if (!geom || !geom->isValid())
      continue;
    const float *p = geom->vertexPositions();
    const uint32_t *idx = geom->indices();
    const uint32_t nv = geom->numVertices();
    const uint32_t n = idx ? geom->numTriangles() * 3 : nv;
    for (uint32_t i = 0; i < n; i++) {
      const uint32_t v = idx ? idx[i] : i;
      if (v < nv)
        b.extend(float3(p[v * 3], p[v * 3 + 1], p[v * 3 + 2]));
    }
  }
  for (auto *v : m_volumes) {
    auto *vol = dynamic_cast<const TransferFunction1DVolume *>(v);
    auto *field = vol && vol->isValid() ? vol->field() : nullptr;
    if (field) {
      b.extend(field->worldMin());
      b.extend(field->worldMax());
    }
  }
  return b;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Group *);

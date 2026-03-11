// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "World.h"

namespace anari_webgpu {

World::World(WebGPUDeviceGlobalState *s)
    : Object(ANARI_WORLD, s),
      m_zeroSurfaceData(this),
      m_zeroVolumeData(this),
      m_zeroLightData(this),
      m_instanceData(this)
{
  m_zeroGroup = new Group(s);
  m_zeroInstance = new Instance(s);
  m_zeroInstance->setParamDirect("group", m_zeroGroup.ptr);

  m_zeroGroup->refDec(helium::RefType::PUBLIC);
  m_zeroInstance->refDec(helium::RefType::PUBLIC);
}

World::~World() = default;

bool World::getProperty(const std::string_view &name,
    ANARIDataType type,
    void *ptr,
    uint64_t size,
    uint32_t flags)
{
  return Object::getProperty(name, type, ptr, size, flags);
}

void World::commitParameters()
{
  m_zeroSurfaceData = getParamObject<ObjectArray>("surface");
  m_zeroVolumeData = getParamObject<ObjectArray>("volume");
  m_zeroLightData = getParamObject<ObjectArray>("light");
  m_instanceData = getParamObject<ObjectArray>("instance");
}

void World::finalize()
{
  const bool addZeroInstance =
      m_zeroSurfaceData || m_zeroVolumeData || m_zeroLightData;

  if (m_zeroSurfaceData)
    m_zeroGroup->setParamDirect("surface", getParamDirect("surface"));
  else
    m_zeroGroup->removeParam("surface");

  if (m_zeroVolumeData)
    m_zeroGroup->setParamDirect("volume", getParamDirect("volume"));
  else
    m_zeroGroup->removeParam("volume");

  if (m_zeroLightData)
    m_zeroGroup->setParamDirect("light", getParamDirect("light"));
  else
    m_zeroGroup->removeParam("light");

  m_zeroInstance->setParam("id", getParam<uint32_t>("id", ~0u));

  m_zeroGroup->commitParameters();
  m_zeroInstance->commitParameters();

  m_instances.clear();

  if (m_instanceData) {
    std::for_each(m_instanceData->handlesBegin(),
        m_instanceData->handlesEnd(),
        [&](auto *o) {
          if (o && o->isValid())
            m_instances.push_back((Instance *)o);
        });
  }

  if (addZeroInstance)
    m_instances.push_back(m_zeroInstance.ptr);
}

const std::vector<Instance *> &World::instances() const
{
  return m_instances;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::World *);

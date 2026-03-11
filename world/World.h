// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Instance.h"

namespace anari_webgpu {

struct World : public Object
{
  World(WebGPUDeviceGlobalState *s);
  ~World() override;

  bool getProperty(const std::string_view &name,
      ANARIDataType type,
      void *ptr,
      uint64_t size,
      uint32_t flags) override;

  void commitParameters() override;
  void finalize() override;

  const std::vector<Instance *> &instances() const;

 private:
  helium::ChangeObserverPtr<ObjectArray> m_zeroSurfaceData;
  helium::ChangeObserverPtr<ObjectArray> m_zeroVolumeData;
  helium::ChangeObserverPtr<ObjectArray> m_zeroLightData;

  helium::ChangeObserverPtr<ObjectArray> m_instanceData;
  std::vector<Instance *> m_instances;

  helium::IntrusivePtr<Group> m_zeroGroup;
  helium::IntrusivePtr<Instance> m_zeroInstance;
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::World *, ANARI_WORLD);

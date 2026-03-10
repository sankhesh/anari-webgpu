// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "array/ObjectArray.h"
#include "light/Light.h"
#include "surface/Surface.h"
#include "volume/Volume.h"

namespace anari_webgpu {

struct Group : public Object
{
  Group(WebGPUDeviceGlobalState *s);
  ~Group() override;

  bool getProperty(const std::string_view &name,
      ANARIDataType type,
      void *ptr,
      uint64_t size,
      uint32_t flags) override;

  void commitParameters() override;
  void finalize() override;

  const std::vector<Surface *> &surfaces() const;
  const std::vector<Volume *> &volumes() const;
  const std::vector<Light *> &lights() const;

 private:
  helium::ChangeObserverPtr<ObjectArray> m_surfaceData;
  std::vector<Surface *> m_surfaces;

  helium::ChangeObserverPtr<ObjectArray> m_volumeData;
  std::vector<Volume *> m_volumes;

  helium::ChangeObserverPtr<ObjectArray> m_lightData;
  std::vector<Light *> m_lights;
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::Group *, ANARI_GROUP);

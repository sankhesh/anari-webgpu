// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "geometry/Geometry.h"
#include "material/Material.h"

namespace anari_webgpu {

struct Surface : public Object
{
  Surface(WebGPUDeviceGlobalState *s);
  ~Surface() override = default;

  void commitParameters() override;

  uint32_t id() const;
  const Geometry *geometry() const;
  const Material *material() const;

  bool isValid() const override;

 private:
  uint32_t m_id{~0u};
  helium::IntrusivePtr<Geometry> m_geometry;
  helium::IntrusivePtr<Material> m_material;
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::Surface *, ANARI_SURFACE);

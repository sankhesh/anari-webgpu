// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"
#include "sampler/Sampler.h"

namespace anari_webgpu {

struct Material : public Object
{
  Material(WebGPUDeviceGlobalState *s);
  ~Material() override = default;
  static Material *createInstance(
      std::string_view subtype, WebGPUDeviceGlobalState *s);
};

struct MatteMaterial : public Material
{
  MatteMaterial(WebGPUDeviceGlobalState *s);
  void commitParameters() override;
  bool isValid() const override;

  float4 color() const;

 private:
  float4 m_color{0.8f, 0.8f, 0.8f, 1.f};
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::Material *, ANARI_MATERIAL);

// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"
#include <limits>

namespace anari_webgpu {

struct PixelSample
{
  float4 color{0.f, 0.f, 0.f, 1.f};
  float depth{std::numeric_limits<float>::max()};

  PixelSample() = default;
  PixelSample(float4 c) : color(c) {}
  PixelSample(float4 c, float d) : color(c), depth(d) {}
};

struct Renderer : public Object
{
  Renderer(WebGPUDeviceGlobalState *s);
  ~Renderer() override = default;

  static Renderer *createInstance(
      std::string_view subtype, WebGPUDeviceGlobalState *d);

  void commitParameters() override;

  float4 background() const;
  float ambientRadiance() const;

 private:
  float4 m_background{0.f, 0.f, 0.f, 1.f};
  float m_ambientRadiance{1.f};
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::Renderer *, ANARI_RENDERER);

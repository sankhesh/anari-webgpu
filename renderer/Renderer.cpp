// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "Renderer.h"

namespace anari_webgpu {

Renderer::Renderer(WebGPUDeviceGlobalState *s) : Object(ANARI_RENDERER, s) {}

Renderer *Renderer::createInstance(
    std::string_view /* subtype */, WebGPUDeviceGlobalState *s)
{
  return new Renderer(s);
}

void Renderer::commitParameters()
{
  m_background = getParam<float4>("background", float4(float3(0.f), 1.f));
  m_ambientRadiance = getParam<float>("ambientRadiance", 1.f);
}

float4 Renderer::background() const
{
  return m_background;
}

float Renderer::ambientRadiance() const
{
  return m_ambientRadiance;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Renderer *);

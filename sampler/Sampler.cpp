// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "Sampler.h"

namespace anari_webgpu {

Sampler::Sampler(WebGPUDeviceGlobalState *s) : Object(ANARI_SAMPLER, s) {}

Sampler *Sampler::createInstance(
    std::string_view subtype, WebGPUDeviceGlobalState *s)
{
  return (Sampler *)new UnknownObject(ANARI_SAMPLER, s);
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Sampler *);

// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"

namespace anari_webgpu {

struct Volume : public Object
{
  Volume(WebGPUDeviceGlobalState *d);
  virtual ~Volume() = default;
  static Volume *createInstance(
      std::string_view subtype, WebGPUDeviceGlobalState *d);

  void commitParameters() override;
  uint32_t id() const;

 private:
  uint32_t m_id{~0u};
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::Volume *, ANARI_VOLUME);

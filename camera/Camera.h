// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "../Object.h"

namespace anari_webgpu {

struct Camera : public Object
{
  Camera(WebGPUDeviceGlobalState *s);
  ~Camera() override = default;
  static Camera *createInstance(
      std::string_view type, WebGPUDeviceGlobalState *state);

  void commitParameters() override;

  float3 position() const;
  float3 direction() const;
  float3 up() const;

 protected:
  float3 m_position{0.f, 0.f, 0.f};
  float3 m_direction{0.f, 0.f, -1.f};
  float3 m_up{0.f, 1.f, 0.f};
};

struct PerspectiveCamera : public Camera
{
  PerspectiveCamera(WebGPUDeviceGlobalState *s);
  void commitParameters() override;
  float fovy() const;
  float aspect() const;
  bool isValid() const override;

 private:
  float m_fovy{M_PI / 3.f};
  float m_aspect{1.f};
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::Camera *, ANARI_CAMERA);

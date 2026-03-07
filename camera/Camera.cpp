// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "Camera.h"
#include <cmath>

namespace anari_webgpu {

// Camera /////////////////////////////////////////////////////////////////////

Camera::Camera(WebGPUDeviceGlobalState *s) : Object(ANARI_CAMERA, s) {}

Camera *Camera::createInstance(
    std::string_view type, WebGPUDeviceGlobalState *s)
{
  if (type == "perspective")
    return new PerspectiveCamera(s);
  return (Camera *)new UnknownObject(ANARI_CAMERA, s);
}

void Camera::commitParameters()
{
  m_position = getParam<float3>("position", float3(0.f, 0.f, 0.f));
  m_direction = getParam<float3>("direction", float3(0.f, 0.f, -1.f));
  m_up = getParam<float3>("up", float3(0.f, 1.f, 0.f));
}

float3 Camera::position() const
{
  return m_position;
}

float3 Camera::direction() const
{
  return m_direction;
}

float3 Camera::up() const
{
  return m_up;
}

// PerspectiveCamera //////////////////////////////////////////////////////////

PerspectiveCamera::PerspectiveCamera(WebGPUDeviceGlobalState *s) : Camera(s) {}

void PerspectiveCamera::commitParameters()
{
  Camera::commitParameters();
  m_fovy = getParam<float>("fovy", static_cast<float>(M_PI / 2.0));
  m_aspect = getParam<float>("aspect", 1.f);
}

float PerspectiveCamera::fovy() const
{
  return m_fovy;
}

float PerspectiveCamera::aspect() const
{
  return m_aspect;
}

bool PerspectiveCamera::isValid() const
{
  return true;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Camera *);

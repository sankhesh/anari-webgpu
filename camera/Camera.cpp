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
  m_imageRegion = float4(0.f, 0.f, 1.f, 1.f);
  getParam("imageRegion", ANARI_FLOAT32_BOX2, &m_imageRegion);
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

mat4 Camera::viewMatrix() const
{
  const float3 f = linalg::normalize(m_direction);
  const float3 s = linalg::normalize(linalg::cross(f, m_up));
  const float3 u = linalg::cross(s, f);

  mat4 view;
  view[0] = {s.x, u.x, -f.x, 0.f};
  view[1] = {s.y, u.y, -f.y, 0.f};
  view[2] = {s.z, u.z, -f.z, 0.f};
  view[3] = {-linalg::dot(s, m_position),
      -linalg::dot(u, m_position),
      linalg::dot(f, m_position),
      1.f};
  return view;
}

// PerspectiveCamera //////////////////////////////////////////////////////////

PerspectiveCamera::PerspectiveCamera(WebGPUDeviceGlobalState *s) : Camera(s) {}

void PerspectiveCamera::commitParameters()
{
  Camera::commitParameters();
  m_fovy = getParam<float>("fovy", static_cast<float>(M_PI / 3.0));
  if (hasParam("aspect", ANARI_FLOAT32))
    m_aspect = getParam<float>("aspect", 1.f);
  else
    m_aspect.reset();
}

float PerspectiveCamera::fovy() const
{
  return m_fovy;
}

float PerspectiveCamera::aspect(float frameAspect) const
{
  return m_aspect.value_or(frameAspect);
}

mat4 PerspectiveCamera::projectionMatrix(
    float frameAspect, float near, float far) const
{
  const float th = std::tan(m_fovy / 2.f);
  mat4 proj;
  proj[0] = {1.f / (aspect(frameAspect) * th), 0.f, 0.f, 0.f};
  proj[1] = {0.f, 1.f / th, 0.f, 0.f};
  proj[2] = {0.f, 0.f, far / (near - far), -1.f};
  proj[3] = {0.f, 0.f, (far * near) / (near - far), 0.f};

  const float4 r = m_imageRegion;
  const float sx = 1.f / (r.z - r.x);
  const float sy = 1.f / (r.w - r.y);
  mat4 region = linalg::identity;
  region[0][0] = sx;
  region[1][1] = sy;
  region[3][0] = (1.f - r.x - r.z) * sx;
  region[3][1] = (1.f - r.y - r.w) * sy;
  return linalg::mul(region, proj);
}

bool PerspectiveCamera::isValid() const
{
  return true;
}

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_DEFINITION(anari_webgpu::Camera *);

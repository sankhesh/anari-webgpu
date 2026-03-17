// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "camera/Camera.h"
#include "renderer/Renderer.h"
#include "world/World.h"
// helium
#include "helium/BaseFrame.h"
// webgpu
#include <webgpu/webgpu.h>
// std
#include <vector>

namespace anari_webgpu {

struct Frame : public helium::BaseFrame
{
  Frame(WebGPUDeviceGlobalState *s);
  ~Frame();

  bool isValid() const override;

  WebGPUDeviceGlobalState *deviceState() const;

  bool getProperty(const std::string_view &name,
      ANARIDataType type,
      void *ptr,
      uint64_t size,
      uint32_t flags) override;

  void commitParameters() override;
  void finalize() override;

  void renderFrame() override;

  void *map(std::string_view channel,
      uint32_t *width,
      uint32_t *height,
      ANARIDataType *pixelType) override;
  void unmap(std::string_view channel) override;
  int frameReady(ANARIWaitMask m) override;
  void discard() override;

  bool ready() const;
  void wait() const;

  // --- Shared resource access ---
  // External WebGPU consumers can access the framebuffer storage buffer
  // directly for compositing, post-processing, or display without readback.
  WGPUBuffer colorStorageBuffer() const { return m_colorStorageBuf; }
  WGPUBuffer depthStorageBuffer() const { return m_depthStorageBuf; }

 private:
  void initComputeResources();
  void cleanupComputeResources();
  void writeSample(int x, int y, const PixelSample &s);

  // Software rasterizer fallback for basic rendering
  void softwareRasterize();

  //// Data ////

  bool m_valid{false};
  int m_perPixelBytes{1};
  uint2 m_size{0u, 0u};

  anari::DataType m_colorType{ANARI_UNKNOWN};
  anari::DataType m_depthType{ANARI_UNKNOWN};

  std::vector<uint8_t> m_pixelBuffer;
  std::vector<float> m_depthBuffer;

  helium::IntrusivePtr<Renderer> m_renderer;
  helium::IntrusivePtr<Camera> m_camera;
  helium::IntrusivePtr<World> m_world;

  float m_duration{0.f};

  // --- Compute pipeline resources ---
  // The framebuffer is a storage buffer (not a render texture), making it
  // directly accessible to compute shaders for rendering, compositing, and
  // post-processing. External WebGPU code sharing the same WGPUDevice can
  // bind these buffers in their own compute/render pipelines.
  WGPUBuffer m_colorStorageBuf{nullptr};   // RGBA32Float per pixel
  WGPUBuffer m_depthStorageBuf{nullptr};   // Float32 per pixel
  WGPUBuffer m_readbackBuffer{nullptr};    // For CPU readback

  // Clear pass
  WGPUComputePipeline m_clearPipeline{nullptr};
  WGPUShaderModule m_clearShader{nullptr};
  WGPUBindGroupLayout m_clearBGL{nullptr};

  // Rasterize pass
  WGPUComputePipeline m_rasterPipeline{nullptr};
  WGPUShaderModule m_rasterShader{nullptr};
  WGPUBindGroupLayout m_rasterBGL{nullptr};

  bool m_gpuResourcesInitialized{false};
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::Frame *, ANARI_FRAME);

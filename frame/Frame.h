// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "camera/Camera.h"
#include "renderer/Renderer.h"
#include "material/Material.h"
#include "world/World.h"
// helium
#include "helium/BaseFrame.h"
#include "vtkSlangCompiler.h"
// webgpu
#include <webgpu/webgpu.h>
// std
#include <map>
#include <memory>
#include <set>
#include <string>
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
  WGPUBuffer colorStorageBuffer() const { return m_colorStorageBuf; }
  WGPUBuffer depthStorageBuffer() const { return m_depthStorageBuf; }

 private:
  void initComputeResources();
  void cleanupComputeResources();
  WGPUShaderModule createBuiltinShader(WGPUDevice device, const char *name);

  struct SlangPipelines
  {
    bool valid{false};
    WGPUShaderModule shader{nullptr};
    WGPUComputePipeline color{nullptr};
    WGPUComputePipeline depth{nullptr};
    WGPUBindGroupLayout parametersBGL{nullptr};
    uint64_t parametersSize{0};
    std::vector<vtkslang::UniformField> fields;
  };
  const SlangPipelines *slangPipelines(WGPUDevice device,
      const char *shader,
      WGPUBindGroupLayout group0,
      const char *colorEntryPoint,
      const char *depthEntryPoint,
      const SlangOverride &slangOverride);
  std::vector<uint8_t> packSlangParameters(
      const SlangPipelines &p, const SlangOverride &slangOverride);
  void reportOnce(ANARIStatusSeverity severity, const std::string &message);
  void writeSample(int x, int y, const PixelSample &s);

  void softwareRasterize();
  void softwareRaymarchVolume();

  //// Data ////

  bool m_valid{false};
  int m_perPixelBytes{1};
  uint2 m_size{0u, 0u};

  anari::DataType m_colorType{ANARI_UNKNOWN};
  anari::DataType m_depthType{ANARI_UNKNOWN};

  std::vector<uint8_t> m_pixelBuffer;
  std::vector<float> m_depthBuffer;
  std::vector<float> m_zBuffer;

  helium::IntrusivePtr<Renderer> m_renderer;
  helium::IntrusivePtr<Camera> m_camera;
  helium::IntrusivePtr<World> m_world;

  float m_duration{0.f};

  vtkslang::vtkSlangCompiler m_slangCompiler;

  WGPUBuffer m_colorStorageBuf{nullptr};
  WGPUBuffer m_depthStorageBuf{nullptr};
  WGPUBuffer m_readbackBuffer{nullptr};

  // Clear pass
  WGPUComputePipeline m_clearPipeline{nullptr};
  WGPUShaderModule m_clearShader{nullptr};
  WGPUBindGroupLayout m_clearBGL{nullptr};

  WGPUComputePipeline m_rasterPipeline{nullptr};
  WGPUComputePipeline m_rasterDepthPipeline{nullptr};
  WGPUShaderModule m_rasterShader{nullptr};
  WGPUBindGroupLayout m_rasterBGL{nullptr};

  WGPUComputePipeline m_pbrPipeline{nullptr};
  WGPUComputePipeline m_pbrDepthPipeline{nullptr};
  WGPUShaderModule m_pbrShader{nullptr};
  WGPUBindGroupLayout m_pbrBGL{nullptr};

  WGPUComputePipeline m_volumePipeline{nullptr};
  WGPUShaderModule m_volumeShader{nullptr};
  WGPUBindGroupLayout m_volumeBGL{nullptr};

  std::map<std::string, SlangPipelines> m_slangPipelines;
  std::set<std::string> m_reportedMessages;

  bool m_gpuResourcesInitialized{false};
};

} // namespace anari_webgpu

WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(anari_webgpu::Frame *, ANARI_FRAME);

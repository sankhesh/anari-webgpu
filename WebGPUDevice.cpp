// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "WebGPUDevice.h"

#include "array/Array1D.h"
#include "array/Array2D.h"
#include "array/Array3D.h"
#include "array/ObjectArray.h"
#include "frame/Frame.h"
#include "spatial_field/SpatialField.h"

#include <webgpu/webgpu.h>

#include "anari_library_webgpu_queries.h"

// Context structs for Dawn async callbacks
struct AdapterResult
{
  WGPUAdapter adapter{ nullptr };
  bool done{ false };
};
struct DeviceResult
{
  WGPUDevice device{ nullptr };
  bool done{ false };
};
// Free functions for Dawn async callbacks
namespace
{
void adapterCallback(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message,
  void* userdata1, void* userdata2)
{
  auto* result = static_cast<AdapterResult*>(userdata1);
  if (status == WGPURequestAdapterStatus_Success)
  {
    result->adapter = adapter;
  }
  result->done = true;
}
void deviceCallback(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message,
  void* userdata1, void* userdata2)
{
  auto* result = static_cast<DeviceResult*>(userdata1);
  if (status == WGPURequestDeviceStatus_Success)
  {
    result->device = device;
  }
  result->done = true;
}
void uncapturedErrorCallback(WGPUDevice const* /*device*/, WGPUErrorType type,
  WGPUStringView message, void* /*userdata1*/, void* /*userdata2*/)
{
  fprintf(stderr, "WebGPU error (%d): %.*s\n", (int)type, (int)message.length, message.data);
}
void deviceLostCallback(WGPUDevice const* /*device*/, WGPUDeviceLostReason reason,
  WGPUStringView message, void* /*userdata1*/, void* /*userdata2*/)
{
  // Silence Dawn's warning about no device-lost callback. Reason 1 is
  // WGPUDeviceLostReason_Destroyed — the normal path during teardown.
  if (reason != WGPUDeviceLostReason_Destroyed)
  {
    fprintf(stderr, "WebGPU device lost (reason %d): %.*s\n", (int)reason, (int)message.length,
      message.data);
  }
}
} // namespace

namespace anari_webgpu
{

// API Objects ////////////////////////////////////////////////////////////////

ANARIArray1D WebGPUDevice::newArray1D(const void* appMemory, ANARIMemoryDeleter deleter,
  const void* userData, ANARIDataType type, uint64_t numItems)
{
  initDevice();

  Array1DMemoryDescriptor md;
  md.appMemory = appMemory;
  md.deleter = deleter;
  md.deleterPtr = userData;
  md.elementType = type;
  md.numItems = numItems;

  if (anari::isObject(type))
    return (ANARIArray1D) new ObjectArray(deviceState(), md);
  else
    return (ANARIArray1D) new Array1D(deviceState(), md);
}

ANARIArray2D WebGPUDevice::newArray2D(const void* appMemory, ANARIMemoryDeleter deleter,
  const void* userData, ANARIDataType type, uint64_t numItems1, uint64_t numItems2)
{
  initDevice();

  Array2DMemoryDescriptor md;
  md.appMemory = appMemory;
  md.deleter = deleter;
  md.deleterPtr = userData;
  md.elementType = type;
  md.numItems1 = numItems1;
  md.numItems2 = numItems2;

  return (ANARIArray2D) new Array2D(deviceState(), md);
}

ANARIArray3D WebGPUDevice::newArray3D(const void* appMemory, ANARIMemoryDeleter deleter,
  const void* userData, ANARIDataType type, uint64_t numItems1, uint64_t numItems2,
  uint64_t numItems3)
{
  initDevice();

  Array3DMemoryDescriptor md;
  md.appMemory = appMemory;
  md.deleter = deleter;
  md.deleterPtr = userData;
  md.elementType = type;
  md.numItems1 = numItems1;
  md.numItems2 = numItems2;
  md.numItems3 = numItems3;

  return (ANARIArray3D) new Array3D(deviceState(), md);
}

ANARICamera WebGPUDevice::newCamera(const char* subtype)
{
  initDevice();
  return (ANARICamera)Camera::createInstance(subtype, deviceState());
}

ANARIFrame WebGPUDevice::newFrame()
{
  initDevice();
  return (ANARIFrame) new Frame(deviceState());
}

ANARIGeometry WebGPUDevice::newGeometry(const char* subtype)
{
  initDevice();
  return (ANARIGeometry)Geometry::createInstance(subtype, deviceState());
}

ANARIGroup WebGPUDevice::newGroup()
{
  initDevice();
  return (ANARIGroup) new Group(deviceState());
}

ANARIInstance WebGPUDevice::newInstance(const char* /*subtype*/)
{
  initDevice();
  return (ANARIInstance) new Instance(deviceState());
}

ANARILight WebGPUDevice::newLight(const char* subtype)
{
  initDevice();
  return (ANARILight)Light::createInstance(subtype, deviceState());
}

ANARIMaterial WebGPUDevice::newMaterial(const char* subtype)
{
  initDevice();
  return (ANARIMaterial)Material::createInstance(subtype, deviceState());
}

ANARIRenderer WebGPUDevice::newRenderer(const char* subtype)
{
  initDevice();
  return (ANARIRenderer)Renderer::createInstance(subtype, deviceState());
}

ANARISampler WebGPUDevice::newSampler(const char* subtype)
{
  initDevice();
  return (ANARISampler)Sampler::createInstance(subtype, deviceState());
}

ANARISpatialField WebGPUDevice::newSpatialField(const char* subtype)
{
  initDevice();
  return (ANARISpatialField)SpatialField::createInstance(subtype, deviceState());
}

ANARISurface WebGPUDevice::newSurface()
{
  initDevice();
  return (ANARISurface) new Surface(deviceState());
}

ANARIVolume WebGPUDevice::newVolume(const char* subtype)
{
  initDevice();
  return (ANARIVolume)Volume::createInstance(subtype, deviceState());
}

ANARIWorld WebGPUDevice::newWorld()
{
  initDevice();
  return (ANARIWorld) new World(deviceState());
}

// Query functions ////////////////////////////////////////////////////////////

const char** WebGPUDevice::getObjectSubtypes(ANARIDataType objectType)
{
  return anari_webgpu::query_object_types(objectType);
}

const void* WebGPUDevice::getObjectInfo(
  ANARIDataType objectType, const char* objectSubtype, const char* infoName, ANARIDataType infoType)
{
  return anari_webgpu::query_object_info(objectType, objectSubtype, infoName, infoType);
}

const void* WebGPUDevice::getParameterInfo(ANARIDataType objectType, const char* objectSubtype,
  const char* parameterName, ANARIDataType parameterType, const char* infoName,
  ANARIDataType infoType)
{
  return anari_webgpu::query_param_info(
    objectType, objectSubtype, parameterName, parameterType, infoName, infoType);
}

// Other WebGPUDevice definitions /////////////////////////////////////////////

WebGPUDevice::WebGPUDevice(ANARIStatusCallback cb, const void* ptr)
  : helium::BaseDevice(cb, ptr)
{
  m_state = std::make_unique<WebGPUDeviceGlobalState>(this_device());
  deviceCommitParameters();
}

WebGPUDevice::WebGPUDevice(ANARILibrary l)
  : helium::BaseDevice(l)
{
  m_state = std::make_unique<WebGPUDeviceGlobalState>(this_device());
  deviceCommitParameters();
}

WebGPUDevice::~WebGPUDevice()
{
  auto& state = *deviceState();
  state.commitBuffer.clear();
  reportMessage(ANARI_SEVERITY_DEBUG, "destroying WebGPU device (%p)", this);
}

void WebGPUDevice::initDevice()
{
  if (m_initialized)
    return;
  reportMessage(ANARI_SEVERITY_DEBUG, "initializing WebGPU device (%p)", this);

  initWebGPU();

  m_initialized = true;
}

void WebGPUDevice::initWebGPU()
{
  auto* state = deviceState();

  // --- External handle path ---
  // If an external WGPUDevice was provided, adopt it without creating our own.
  // This enables resource sharing: GPU buffers and textures created by the
  // external owner are directly usable by this ANARI device and vice versa.
  if (m_externalDevice)
  {
    state->wgpuDevice = (WGPUDevice)m_externalDevice;
    state->ownsDevice = false;

    if (m_externalQueue)
    {
      state->wgpuQueue = (WGPUQueue)m_externalQueue;
      state->ownsQueue = false;
    }
    else
    {
      // Get the default queue from the external device
      state->wgpuQueue = wgpuDeviceGetQueue(state->wgpuDevice);
      state->ownsQueue = true;
    }

    if (m_externalInstance)
    {
      state->wgpuInstance = (WGPUInstance)m_externalInstance;
      state->ownsInstance = false;
    }
    if (m_externalAdapter)
    {
      state->wgpuAdapter = (WGPUAdapter)m_externalAdapter;
      state->ownsAdapter = false;
    }

    reportMessage(ANARI_SEVERITY_DEBUG,
      "WebGPU device initialized with external handles "
      "(device=%p, queue=%p)",
      state->wgpuDevice, state->wgpuQueue);
    return;
  }

  // --- Internal creation path ---
  // Create our own WebGPU instance, adapter, and device.

  WGPUInstanceDescriptor instanceDesc{};
  instanceDesc.nextInChain = nullptr;
  state->wgpuInstance = wgpuCreateInstance(&instanceDesc);
  state->ownsInstance = true;

  if (!state->wgpuInstance)
  {
    reportMessage(ANARI_SEVERITY_WARNING,
      "Failed to create WebGPU instance. "
      "Falling back to software rasterization.");
    return;
  }

  // Request adapter (synchronous pattern using callbacks)
  AdapterResult adapterResult;

  WGPURequestAdapterOptions adapterOpts{};
  adapterOpts.nextInChain = nullptr;
  adapterOpts.powerPreference = WGPUPowerPreference_HighPerformance;
  adapterOpts.compatibleSurface = nullptr;
  adapterOpts.forceFallbackAdapter = false;

  WGPURequestAdapterCallbackInfo adapterCallbackInfo{};
  adapterCallbackInfo.nextInChain = nullptr;
  adapterCallbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
  adapterCallbackInfo.callback = adapterCallback;
  adapterCallbackInfo.userdata1 = &adapterResult;
  adapterCallbackInfo.userdata2 = nullptr;
  wgpuInstanceRequestAdapter(state->wgpuInstance, &adapterOpts, adapterCallbackInfo);

#if !defined(__EMSCRIPTEN__)
  while (!adapterResult.done)
  {
    wgpuInstanceProcessEvents(state->wgpuInstance);
  }
#endif

  if (!adapterResult.adapter)
  {
    reportMessage(ANARI_SEVERITY_WARNING,
      "Failed to get WebGPU adapter. "
      "Falling back to software rasterization.");
    return;
  }
  state->wgpuAdapter = adapterResult.adapter;
  state->ownsAdapter = true;

  // Request device
  DeviceResult deviceResult;

  WGPUDeviceDescriptor deviceDesc{};
  deviceDesc.nextInChain = nullptr;

  deviceDesc.label = WGPUStringView{ "anari_webgpu_device", strlen("anari_webgpu_device") };
  deviceDesc.requiredFeatureCount = 0;
  deviceDesc.requiredLimits = nullptr;
  deviceDesc.defaultQueue.nextInChain = nullptr;
  deviceDesc.defaultQueue.label =
    WGPUStringView{ "anari_webgpu_queue", strlen("anari_webgpu_queue") };
  deviceDesc.uncapturedErrorCallbackInfo.nextInChain = nullptr;
  deviceDesc.uncapturedErrorCallbackInfo.callback = uncapturedErrorCallback;
  deviceDesc.uncapturedErrorCallbackInfo.userdata1 = nullptr;
  deviceDesc.uncapturedErrorCallbackInfo.userdata2 = nullptr;
  deviceDesc.deviceLostCallbackInfo.nextInChain = nullptr;
  deviceDesc.deviceLostCallbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
  deviceDesc.deviceLostCallbackInfo.callback = deviceLostCallback;
  deviceDesc.deviceLostCallbackInfo.userdata1 = nullptr;
  deviceDesc.deviceLostCallbackInfo.userdata2 = nullptr;

  WGPURequestDeviceCallbackInfo deviceCallbackInfo{};
  deviceCallbackInfo.nextInChain = nullptr;
  deviceCallbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
  deviceCallbackInfo.callback = deviceCallback;
  deviceCallbackInfo.userdata1 = &deviceResult;
  deviceCallbackInfo.userdata2 = nullptr;
  wgpuAdapterRequestDevice(state->wgpuAdapter, &deviceDesc, deviceCallbackInfo);

#if !defined(__EMSCRIPTEN__)
  while (!deviceResult.done)
  {
    wgpuInstanceProcessEvents(state->wgpuInstance);
  }
#endif

  if (!deviceResult.device)
  {
    reportMessage(ANARI_SEVERITY_WARNING,
      "Failed to get WebGPU device. "
      "Falling back to software rasterization.");
    return;
  }
  state->wgpuDevice = deviceResult.device;
  state->ownsDevice = true;
  state->wgpuQueue = wgpuDeviceGetQueue(state->wgpuDevice);
  state->ownsQueue = true;

  reportMessage(ANARI_SEVERITY_DEBUG, "WebGPU device initialized successfully");
}

void WebGPUDevice::deviceCommitParameters()
{
  helium::BaseDevice::deviceCommitParameters();

  // Check for externally provided WebGPU handles.
  // These allow the ANARI device to share a WGPUDevice with other WebGPU
  // consumers (e.g. a WebGPU-based renderer, a UI framework, etc.), enabling
  // zero-copy sharing of GPU resources like buffers and textures.
  //
  // Usage from C:
  //   WGPUDevice myDevice = ...;
  //   anariSetParameter(device, device, "webgpu.device",
  //                     ANARI_VOID_POINTER, &myDevice);
  //   anariCommitParameters(device, device);
  //
  bool hadExternalDevice = (m_externalDevice != nullptr);
  m_externalInstance = (WGPUInstance)getParam<void*>("webgpu.instance", (void*)m_externalInstance);
  m_externalAdapter = (WGPUAdapter)getParam<void*>("webgpu.adapter", (void*)m_externalAdapter);
  m_externalDevice = (WGPUDevice)getParam<void*>("webgpu.device", (void*)m_externalDevice);
  m_externalQueue = (WGPUQueue)getParam<void*>("webgpu.queue", (void*)m_externalQueue);

  // If the external device changed after initialization, re-init
  if (m_initialized && !hadExternalDevice && m_externalDevice)
  {
    m_initialized = false;
  }
}

int WebGPUDevice::deviceGetProperty(
  const char* name, ANARIDataType type, void* mem, uint64_t size, uint32_t mask)
{
  std::string_view prop = name;
  if (prop == "extension" && type == ANARI_STRING_LIST)
  {
    helium::writeToVoidP(mem, query_extensions());
    return 1;
  }
  else if (prop == "WebGPU" && type == ANARI_BOOL)
  {
    helium::writeToVoidP(mem, true);
    return 1;
  }
  else if (prop == "webgpu.device" && type == ANARI_VOID_POINTER)
  {
    // Allow external code to retrieve the underlying WGPUDevice for
    // direct resource sharing (creating buffers, textures, etc.)
    helium::writeToVoidP(mem, (void*)deviceState()->wgpuDevice);
    return 1;
  }
  else if (prop == "webgpu.queue" && type == ANARI_VOID_POINTER)
  {
    helium::writeToVoidP(mem, (void*)deviceState()->wgpuQueue);
    return 1;
  }
  else if (prop == "webgpu.instance" && type == ANARI_VOID_POINTER)
  {
    helium::writeToVoidP(mem, (void*)deviceState()->wgpuInstance);
    return 1;
  }
  return 0;
}

WebGPUDeviceGlobalState* WebGPUDevice::deviceState() const
{
  return (WebGPUDeviceGlobalState*)helium::BaseDevice::m_state.get();
}

} // namespace anari_webgpu

// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "WebGPUDevice.h"
#include "anari/backend/LibraryImpl.h"
#include "anari_library_webgpu_export.h"

namespace anari_webgpu {

struct WebGPULibrary : public anari::LibraryImpl
{
  WebGPULibrary(
      void *lib, ANARIStatusCallback defaultStatusCB, const void *statusCBPtr);

  ANARIDevice newDevice(const char *subtype) override;
  const char **getDeviceExtensions(const char *deviceType) override;
};

// Definitions ////////////////////////////////////////////////////////////////

WebGPULibrary::WebGPULibrary(
    void *lib, ANARIStatusCallback defaultStatusCB, const void *statusCBPtr)
    : anari::LibraryImpl(lib, defaultStatusCB, statusCBPtr)
{}

ANARIDevice WebGPULibrary::newDevice(const char * /*subtype*/)
{
  return (ANARIDevice) new WebGPUDevice(this_library());
}

const char **WebGPULibrary::getDeviceExtensions(const char * /*deviceType*/)
{
  return nullptr;
}

} // namespace anari_webgpu

// Define library entrypoint //////////////////////////////////////////////////

extern "C" WEBGPU_EXPORT ANARI_DEFINE_LIBRARY_ENTRYPOINT(
    webgpu, handle, scb, scbPtr)
{
  return (ANARILibrary) new anari_webgpu::WebGPULibrary(handle, scb, scbPtr);
}

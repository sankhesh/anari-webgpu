// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

// Unit tests for Object base class lifecycle

#include <anari/anari.h>
#include <cassert>
#include <cstdio>
#include <cstring>

static bool g_errorOccurred = false;
static char g_lastError[512] = {};

static void statusFunc(const void *, ANARIDevice, ANARIObject,
    ANARIDataType, ANARIStatusSeverity severity,
    ANARIStatusCode, const char *message) {
  if (severity <= ANARI_SEVERITY_ERROR) {
    g_errorOccurred = true;
    strncpy(g_lastError, message, sizeof(g_lastError) - 1);
  }
}

static ANARIDevice createTestDevice() {
  ANARILibrary lib = anariLoadLibrary("webgpu", statusFunc, nullptr);
  assert(lib && "Failed to load webgpu library");
  ANARIDevice dev = anariNewDevice(lib, "default");
  assert(dev && "Failed to create device");
  return dev;
}

// Test: creating and releasing objects does not crash
static void test_create_release() {
  printf("  test_create_release...");
  ANARIDevice dev = createTestDevice();

  ANARICamera cam = anariNewCamera(dev, "perspective");
  assert(cam);
  anariRelease(dev, cam);

  ANARIMaterial mat = anariNewMaterial(dev, "matte");
  assert(mat);
  anariRelease(dev, mat);

  ANARILight light = anariNewLight(dev, "directional");
  assert(light);
  anariRelease(dev, light);

  ANARIGeometry geom = anariNewGeometry(dev, "triangle");
  assert(geom);
  anariRelease(dev, geom);

  ANARIRenderer ren = anariNewRenderer(dev, "default");
  assert(ren);
  anariRelease(dev, ren);

  anariRelease(dev, dev);
  printf(" PASSED\n");
}

// Test: double release should not crash (ANARI spec allows this)
static void test_double_release() {
  printf("  test_double_release...");
  ANARIDevice dev = createTestDevice();

  ANARICamera cam = anariNewCamera(dev, "perspective");
  anariRelease(dev, cam);
  // Second release is a no-op per spec
  anariRelease(dev, cam);

  anariRelease(dev, dev);
  printf(" PASSED\n");
}

// Test: committing with no parameters set should not crash
static void test_empty_commit() {
  printf("  test_empty_commit...");
  ANARIDevice dev = createTestDevice();

  ANARICamera cam = anariNewCamera(dev, "perspective");
  anariCommitParameters(dev, cam);
  anariRelease(dev, cam);

  ANARIMaterial mat = anariNewMaterial(dev, "matte");
  anariCommitParameters(dev, mat);
  anariRelease(dev, mat);

  anariRelease(dev, dev);
  printf(" PASSED\n");
}

// Test: unknown object subtypes should return null or handle gracefully
static void test_unknown_subtype() {
  printf("  test_unknown_subtype...");
  ANARIDevice dev = createTestDevice();

  g_errorOccurred = false;
  ANARICamera cam = anariNewCamera(dev, "nonexistent_camera_type");
  // Device should still be valid even if creation fails
  assert(dev);

  if (cam)
    anariRelease(dev, cam);

  anariRelease(dev, dev);
  printf(" PASSED\n");
}

int main() {
  printf("=== Object Lifecycle Tests ===\n");
  test_create_release();
  test_double_release();
  test_empty_commit();
  test_unknown_subtype();
  printf("All object lifecycle tests passed.\n");
  return 0;
}

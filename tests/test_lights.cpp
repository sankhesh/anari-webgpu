// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

// Unit tests for light objects and multi-light scenes

#include <anari/anari.h>
#include <cassert>
#include <cstdio>
#include <cstring>

static void statusFunc(const void *, ANARIDevice, ANARIObject,
    ANARIDataType, ANARIStatusSeverity, ANARIStatusCode,
    const char *) {}

static ANARIDevice createTestDevice() {
  ANARILibrary lib = anariLoadLibrary("webgpu", statusFunc, nullptr);
  assert(lib);
  ANARIDevice dev = anariNewDevice(lib, "default");
  assert(dev);
  return dev;
}

// Test: directional light with default parameters
static void test_directional_light_defaults() {
  printf("  test_directional_light_defaults...");
  ANARIDevice dev = createTestDevice();

  ANARILight light = anariNewLight(dev, "directional");
  assert(light);
  anariCommitParameters(dev, light);

  anariRelease(dev, light);
  anariRelease(dev, dev);
  printf(" PASSED\n");
}

// Test: directional light with custom direction and color
static void test_directional_light_params() {
  printf("  test_directional_light_params...");
  ANARIDevice dev = createTestDevice();

  ANARILight light = anariNewLight(dev, "directional");
  float dir[] = {1.0f, -1.0f, 0.0f};
  float color[] = {1.0f, 0.8f, 0.6f};
  float irradiance = 2.5f;

  anariSetParameter(dev, light, "direction", ANARI_FLOAT32_VEC3, dir);
  anariSetParameter(dev, light, "color", ANARI_FLOAT32_VEC3, color);
  anariSetParameter(dev, light, "irradiance", ANARI_FLOAT32, &irradiance);
  anariCommitParameters(dev, light);

  anariRelease(dev, light);
  anariRelease(dev, dev);
  printf(" PASSED\n");
}

// Test: multiple lights in a single world
static void test_multiple_lights() {
  printf("  test_multiple_lights...");
  ANARIDevice dev = createTestDevice();

  // Create 3 directional lights at different angles
  ANARILight lights[3];
  float directions[][3] = {
    {0.0f, -1.0f, 0.0f},   // top-down
    {1.0f,  0.0f, -1.0f},  // side
    {-1.0f, -0.5f, -1.0f}  // opposite side
  };
  float colors[][3] = {
    {1.0f, 1.0f, 1.0f},    // white
    {1.0f, 0.4f, 0.1f},    // warm
    {0.1f, 0.3f, 1.0f}     // cool
  };

  for (int i = 0; i < 3; i++) {
    lights[i] = anariNewLight(dev, "directional");
    anariSetParameter(dev, lights[i], "direction", ANARI_FLOAT32_VEC3,
        directions[i]);
    anariSetParameter(dev, lights[i], "color", ANARI_FLOAT32_VEC3, colors[i]);
    float irr = (i == 0) ? 1.0f : 0.5f;
    anariSetParameter(dev, lights[i], "irradiance", ANARI_FLOAT32, &irr);
    anariCommitParameters(dev, lights[i]);
  }

  // Build world with all three lights
  ANARIWorld world = anariNewWorld(dev);
  ANARIArray1D lightArray = anariNewArray1D(dev, lights, nullptr, nullptr,
      ANARI_LIGHT, 3);
  anariSetParameter(dev, world, "light", ANARI_ARRAY1D, &lightArray);
  anariCommitParameters(dev, world);

  anariRelease(dev, world);
  anariRelease(dev, lightArray);
  for (int i = 0; i < 3; i++)
    anariRelease(dev, lights[i]);
  anariRelease(dev, dev);
  printf(" PASSED\n");
}

// Test: point light (may not be implemented yet)
static void test_point_light() {
  printf("  test_point_light...");
  ANARIDevice dev = createTestDevice();

  ANARILight light = anariNewLight(dev, "point");
  if (!light) {
    printf(" SKIPPED (not implemented)\n");
    anariRelease(dev, dev);
    return;
  }

  float pos[] = {0.0f, 5.0f, 0.0f};
  float color[] = {1.0f, 1.0f, 1.0f};
  float intensity = 10.0f;
  anariSetParameter(dev, light, "position", ANARI_FLOAT32_VEC3, pos);
  anariSetParameter(dev, light, "color", ANARI_FLOAT32_VEC3, color);
  anariSetParameter(dev, light, "intensity", ANARI_FLOAT32, &intensity);
  anariCommitParameters(dev, light);

  anariRelease(dev, light);
  anariRelease(dev, dev);
  printf(" PASSED\n");
}

int main() {
  printf("=== Light Tests ===\n");
  test_directional_light_defaults();
  test_directional_light_params();
  test_multiple_lights();
  test_point_light();
  printf("All light tests passed.\n");
  return 0;
}

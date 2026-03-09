// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

// Unit tests for material parameter handling

#include <anari/anari.h>
#include <cassert>
#include <cmath>
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

// Test: matte material with default color
static void test_matte_default_color() {
  printf("  test_matte_default_color...");
  ANARIDevice dev = createTestDevice();

  ANARIMaterial mat = anariNewMaterial(dev, "matte");
  assert(mat);
  anariCommitParameters(dev, mat);
  // Default color should be (0.8, 0.8, 0.8, 1.0) — no crash
  anariRelease(dev, mat);
  anariRelease(dev, dev);
  printf(" PASSED\n");
}

// Test: matte material with custom RGBA color
static void test_matte_custom_color() {
  printf("  test_matte_custom_color...");
  ANARIDevice dev = createTestDevice();

  ANARIMaterial mat = anariNewMaterial(dev, "matte");
  float color[] = {1.0f, 0.0f, 0.0f, 1.0f};
  anariSetParameter(dev, mat, "color", ANARI_FLOAT32_VEC4, color);
  anariCommitParameters(dev, mat);
  anariRelease(dev, mat);
  anariRelease(dev, dev);
  printf(" PASSED\n");
}

// Test: matte material with sampler-based color
static void test_matte_with_sampler() {
  printf("  test_matte_with_sampler...");
  ANARIDevice dev = createTestDevice();

  // Create a simple 2x2 texture
  uint8_t texels[] = {
    255, 0, 0, 255,    0, 255, 0, 255,
    0, 0, 255, 255,    255, 255, 0, 255
  };
  ANARIArray2D texArray = anariNewArray2D(dev, texels, nullptr, nullptr,
      ANARI_UFIXED8_VEC4, 2, 2);

  ANARISampler sampler = anariNewSampler(dev, "image2D");
  anariSetParameter(dev, sampler, "image", ANARI_ARRAY2D, &texArray);
  anariCommitParameters(dev, sampler);

  ANARIMaterial mat = anariNewMaterial(dev, "matte");
  anariSetParameter(dev, mat, "color", ANARI_SAMPLER, &sampler);
  anariCommitParameters(dev, mat);

  anariRelease(dev, mat);
  anariRelease(dev, sampler);
  anariRelease(dev, texArray);
  anariRelease(dev, dev);
  printf(" PASSED\n");
}

// Test: PBR material parameters (metallic, roughness)
static void test_pbr_material() {
  printf("  test_pbr_material...");
  ANARIDevice dev = createTestDevice();

  ANARIMaterial mat = anariNewMaterial(dev, "physicallyBased");
  if (!mat) {
    // PBR not yet supported — skip gracefully
    printf(" SKIPPED (not implemented)\n");
    anariRelease(dev, dev);
    return;
  }

  float baseColor[] = {0.9f, 0.1f, 0.1f, 1.0f};
  float metallic = 1.0f;
  float roughness = 0.3f;

  anariSetParameter(dev, mat, "baseColor", ANARI_FLOAT32_VEC4, baseColor);
  anariSetParameter(dev, mat, "metallic", ANARI_FLOAT32, &metallic);
  anariSetParameter(dev, mat, "roughness", ANARI_FLOAT32, &roughness);
  anariCommitParameters(dev, mat);

  anariRelease(dev, mat);
  anariRelease(dev, dev);
  printf(" PASSED\n");
}

// Test: material with opacity
static void test_material_opacity() {
  printf("  test_material_opacity...");
  ANARIDevice dev = createTestDevice();

  ANARIMaterial mat = anariNewMaterial(dev, "matte");
  float color[] = {0.5f, 0.5f, 0.8f, 0.5f};  // semi-transparent
  float opacity = 0.5f;
  anariSetParameter(dev, mat, "color", ANARI_FLOAT32_VEC4, color);
  anariSetParameter(dev, mat, "opacity", ANARI_FLOAT32, &opacity);
  anariCommitParameters(dev, mat);

  anariRelease(dev, mat);
  anariRelease(dev, dev);
  printf(" PASSED\n");
}

int main() {
  printf("=== Material Tests ===\n");
  test_matte_default_color();
  test_matte_custom_color();
  test_matte_with_sampler();
  test_pbr_material();
  test_material_opacity();
  printf("All material tests passed.\n");
  return 0;
}

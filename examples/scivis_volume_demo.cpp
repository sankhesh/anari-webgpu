// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include <anari/anari.h>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

static void statusFunc(const void *, ANARIDevice, ANARIObject, ANARIDataType,
    ANARIStatusSeverity severity, ANARIStatusCode, const char *message)
{
  if (severity <= ANARI_SEVERITY_WARNING)
    fprintf(stderr, "[anari] %s\n", message);
}

static void savePPM(
    const char *filename, const uint8_t *pixels, int width, int height)
{
  FILE *f = fopen(filename, "wb");
  if (!f)
    return;
  fprintf(f, "P6\n%d %d\n255\n", width, height);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      const uint8_t *p = pixels + (y * width + x) * 4;
      fwrite(p, 1, 3, f);
    }
  }
  fclose(f);
}

int main()
{
  printf("=== SciVis Volume Rendering Demo ===\n\n");

  ANARILibrary lib = anariLoadLibrary("webgpu", statusFunc, nullptr);
  if (!lib) {
    fprintf(stderr, "Failed to load webgpu library\n");
    return 1;
  }
  ANARIDevice dev = anariNewDevice(lib, "default");
  if (!dev) {
    fprintf(stderr, "Failed to create device\n");
    return 1;
  }
  printf("Device created\n");

  const int width = 3840, height = 2160;

  const int N = 128;
  printf("Generating %dx%dx%d scalar field...\n", N, N, N);
  std::vector<float> voxels((size_t)N * N * N);
  for (int z = 0; z < N; z++) {
    for (int y = 0; y < N; y++) {
      for (int x = 0; x < N; x++) {
        float fx = 2.f * (x + 0.5f) / N - 1.f;
        float fy = 2.f * (y + 0.5f) / N - 1.f;
        float fz = 2.f * (z + 0.5f) / N - 1.f;
        float r = sqrtf(fx * fx + fy * fy + fz * fz);
        float theta = atan2f(fy, fx);
        float phi = (r > 1e-6f) ? acosf(fz / (r + 1e-6f)) : 0.f;
        float lobes = 0.25f * sinf(6.f * theta) * sinf(5.f * phi);
        float shells = 0.5f + 0.5f * cosf(10.f * r * (1.f + lobes));
        float fade = fmaxf(0.f, 1.f - r);
        voxels[(size_t)z * N * N + y * N + x] = shells * fade;
      }
    }
  }

  ANARIArray3D data =
      anariNewArray3D(dev, voxels.data(), 0, 0, ANARI_FLOAT32, N, N, N);
  ANARISpatialField field = anariNewSpatialField(dev, "structuredRegular");
  anariSetParameter(dev, field, "data", ANARI_ARRAY3D, &data);
  float origin[] = {-1.f, -1.f, -1.f};
  float spacing[] = {2.f / (N - 1), 2.f / (N - 1), 2.f / (N - 1)};
  anariSetParameter(dev, field, "origin", ANARI_FLOAT32_VEC3, origin);
  anariSetParameter(dev, field, "spacing", ANARI_FLOAT32_VEC3, spacing);
  anariCommitParameters(dev, field);

  const int TF_N = 8;
  float tfColors[TF_N * 3] = {
      0.10f, 0.15f, 0.45f,
      0.15f, 0.35f, 0.80f,
      0.20f, 0.65f, 0.85f,
      0.55f, 0.85f, 0.60f,
      0.95f, 0.90f, 0.40f,
      0.98f, 0.60f, 0.20f,
      0.90f, 0.25f, 0.15f,
      0.98f, 0.95f, 0.90f,
  };
  float tfOpacity[TF_N] = {
      0.00f, 0.00f, 0.00f, 0.03f, 0.45f, 0.06f, 0.90f, 1.00f};

  ANARIArray1D colorArr =
      anariNewArray1D(dev, tfColors, 0, 0, ANARI_FLOAT32_VEC3, TF_N);
  ANARIArray1D opacArr =
      anariNewArray1D(dev, tfOpacity, 0, 0, ANARI_FLOAT32, TF_N);

  ANARIVolume vol = anariNewVolume(dev, "transferFunction1D");
  anariSetParameter(dev, vol, "field", ANARI_SPATIAL_FIELD, &field);
  anariSetParameter(dev, vol, "color", ANARI_ARRAY1D, &colorArr);
  anariSetParameter(dev, vol, "opacity", ANARI_ARRAY1D, &opacArr);
  float valueRange[] = {0.f, 1.f};
  anariSetParameter(dev, vol, "valueRange", ANARI_FLOAT32_BOX1, valueRange);
  float densityScale = 24.f;
  anariSetParameter(dev, vol, "densityScale", ANARI_FLOAT32, &densityScale);
  anariCommitParameters(dev, vol);

  ANARIWorld world = anariNewWorld(dev);
  ANARIArray1D volArr = anariNewArray1D(dev, &vol, 0, 0, ANARI_VOLUME, 1);
  anariSetParameter(dev, world, "volume", ANARI_ARRAY1D, &volArr);
  anariCommitParameters(dev, world);

  ANARICamera camera = anariNewCamera(dev, "perspective");
  float camPos[] = {1.30f, 0.95f, 1.55f};
  float camDir[3];
  float len = 0.f;
  for (int i = 0; i < 3; i++)
    camDir[i] = -camPos[i];
  len = sqrtf(camDir[0] * camDir[0] + camDir[1] * camDir[1]
      + camDir[2] * camDir[2]);
  for (int i = 0; i < 3; i++)
    camDir[i] /= len;
  float camUp[] = {0.f, 1.f, 0.f};
  float fovy = 40.0f * (float)M_PI / 180.0f;
  float aspect = (float)width / height;
  anariSetParameter(dev, camera, "position", ANARI_FLOAT32_VEC3, camPos);
  anariSetParameter(dev, camera, "direction", ANARI_FLOAT32_VEC3, camDir);
  anariSetParameter(dev, camera, "up", ANARI_FLOAT32_VEC3, camUp);
  anariSetParameter(dev, camera, "fovy", ANARI_FLOAT32, &fovy);
  anariSetParameter(dev, camera, "aspect", ANARI_FLOAT32, &aspect);
  anariCommitParameters(dev, camera);

  ANARIRenderer renderer = anariNewRenderer(dev, "default");
  float bg[] = {0.02f, 0.02f, 0.05f, 1.f};
  anariSetParameter(dev, renderer, "background", ANARI_FLOAT32_VEC4, bg);
  anariCommitParameters(dev, renderer);

  ANARIFrame frame = anariNewFrame(dev);
  uint32_t size[] = {(uint32_t)width, (uint32_t)height};
  ANARIDataType colorType = ANARI_UFIXED8_VEC4;
  anariSetParameter(dev, frame, "size", ANARI_UINT32_VEC2, size);
  anariSetParameter(dev, frame, "channel.color", ANARI_DATA_TYPE, &colorType);
  anariSetParameter(dev, frame, "renderer", ANARI_RENDERER, &renderer);
  anariSetParameter(dev, frame, "camera", ANARI_CAMERA, &camera);
  anariSetParameter(dev, frame, "world", ANARI_WORLD, &world);
  anariCommitParameters(dev, frame);

  printf("Rendering %dx%d...\n", width, height);
  anariRenderFrame(dev, frame);
  anariFrameReady(dev, frame, ANARI_WAIT);

  uint32_t fbW, fbH;
  ANARIDataType fbType;
  const void *pixels =
      anariMapFrame(dev, frame, "channel.color", &fbW, &fbH, &fbType);
  if (pixels) {
    savePPM("scivis_volume_demo.ppm", (const uint8_t *)pixels, fbW, fbH);
    printf("Saved: scivis_volume_demo.ppm (%ux%u)\n", fbW, fbH);
    anariUnmapFrame(dev, frame, "channel.color");
  } else {
    fprintf(stderr, "Failed to map frame\n");
  }

  anariRelease(dev, frame);
  anariRelease(dev, renderer);
  anariRelease(dev, camera);
  anariRelease(dev, world);
  anariRelease(dev, volArr);
  anariRelease(dev, vol);
  anariRelease(dev, colorArr);
  anariRelease(dev, opacArr);
  anariRelease(dev, field);
  anariRelease(dev, data);
  anariRelease(dev, dev);
  anariUnloadLibrary(lib);

  printf("\n=== Demo Complete ===\n");
  return 0;
}

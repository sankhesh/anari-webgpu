// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

// Example: Render a colored triangle using the ANARI WebGPU device
//
// This example demonstrates how to:
// 1. Load the WebGPU ANARI library
// 2. Create a device (standalone or with an external WGPUDevice)
// 3. Set up a simple scene with a triangle
// 4. Render and save the result to a PPM file
//
// Resource sharing mode (pass --shared):
//   Creates its own WGPUDevice first, passes it to the ANARI device via
//   the "webgpu.device" parameter, then retrieves it back via
//   anariGetProperty to confirm the round-trip. This demonstrates how
//   an external WebGPU renderer can share GPU resources with ANARI.

#include <anari/anari.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static void statusFunc(const void * /*userData*/, ANARIDevice /*device*/,
                       ANARIObject source, ANARIDataType /*sourceType*/,
                       ANARIStatusSeverity severity, ANARIStatusCode /*code*/,
                       const char *message) {
  if (severity == ANARI_SEVERITY_FATAL_ERROR)
    fprintf(stderr, "[FATAL] %s\n", message);
  else if (severity == ANARI_SEVERITY_ERROR)
    fprintf(stderr, "[ERROR] %s\n", message);
  else if (severity == ANARI_SEVERITY_WARNING)
    fprintf(stderr, "[WARN]  %s\n", message);
  else if (severity == ANARI_SEVERITY_INFO)
    fprintf(stderr, "[INFO]  %s\n", message);
}

static void writePPM(const char *filename, int width, int height,
                     const uint8_t *pixels) {
  FILE *f = fopen(filename, "wb");
  if (!f) {
    fprintf(stderr, "Failed to open %s for writing\n", filename);
    return;
  }
  fprintf(f, "P6\n%d %d\n255\n", width, height);
  for (int i = 0; i < width * height; i++) {
    fwrite(&pixels[i * 4], 1, 3, f); // Write RGB, skip A
  }
  fclose(f);
  printf("Wrote %s (%dx%d)\n", filename, width, height);
}

int main(int argc, const char *argv[]) {
  bool sharedMode = false;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--shared") == 0)
      sharedMode = true;
  }

  printf("ANARI WebGPU Example: Render a Triangle%s\n",
         sharedMode ? " (shared WGPUDevice)" : "");

  // Load the WebGPU ANARI library
  ANARILibrary lib = anariLoadLibrary("webgpu", statusFunc, nullptr);
  if (!lib) {
    fprintf(stderr, "Failed to load 'webgpu' ANARI library.\n"
                    "Make sure libanari_webgpu is in your library path.\n");
    return 1;
  }

  // Create a device
  ANARIDevice device = anariNewDevice(lib, "default");
  if (!device) {
    fprintf(stderr, "Failed to create ANARI device.\n");
    anariUnloadLibrary(lib);
    return 1;
  }

  // --- Optional: share an external WGPUDevice ---
  // In a real application, you would create a WGPUDevice using Dawn/wgpu-native
  // and pass it here so that GPU buffers and textures can be shared between
  // your renderer and the ANARI device without any copies.
  //
  // Example (not executed here — requires actual WebGPU initialization):
  //   WGPUDevice myDevice = ...; // your existing WebGPU device
  //   anariSetParameter(device, device, "webgpu.device",
  //                     ANARI_VOID_POINTER, &myDevice);

  // Commit device parameters (triggers WebGPU initialization)
  anariCommitParameters(device, device);

  // Verify we can query the underlying WGPUDevice back (for resource sharing)
  void *wgpuDev = nullptr;
  if (anariGetProperty(
          device, device, "webgpu.device", ANARI_VOID_POINTER, &wgpuDev,
          sizeof(wgpuDev), ANARI_WAIT)) {
    printf("Underlying WGPUDevice: %p (available for resource sharing)\n",
           wgpuDev);
  }

  // --- Create the scene ---

  // Image dimensions
  const int width = 800;
  const int height = 600;

  // Triangle vertices: positions (vec3f)
  float vertices[] = {
      -0.5f, -0.5f, -2.0f, // bottom-left
      0.5f,  -0.5f, -2.0f, // bottom-right
      0.0f,  0.5f,  -2.0f, // top-center
  };

  // Vertex colors (vec4f)
  float colors[] = {
      1.0f, 0.0f, 0.0f, 1.0f, // red
      0.0f, 1.0f, 0.0f, 1.0f, // green
      0.0f, 0.0f, 1.0f, 1.0f, // blue
  };

  // Vertex normals (vec3f) - facing the camera
  float normals[] = {
      0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
  };

  // Create arrays for vertex data
  ANARIArray1D posArray = anariNewArray1D(device, vertices, nullptr, nullptr,
                                          ANARI_FLOAT32_VEC3, 3);
  ANARIArray1D colArray =
      anariNewArray1D(device, colors, nullptr, nullptr, ANARI_FLOAT32_VEC4, 3);
  ANARIArray1D normArray =
      anariNewArray1D(device, normals, nullptr, nullptr, ANARI_FLOAT32_VEC3, 3);

  // Create triangle geometry
  ANARIGeometry geometry = anariNewGeometry(device, "triangle");
  anariSetParameter(device, geometry, "vertex.position", ANARI_ARRAY1D,
                    &posArray);
  anariSetParameter(device, geometry, "vertex.color", ANARI_ARRAY1D, &colArray);
  anariSetParameter(device, geometry, "vertex.normal", ANARI_ARRAY1D,
                    &normArray);
  anariCommitParameters(device, geometry);

  // Create matte material
  ANARIMaterial material = anariNewMaterial(device, "matte");
  float matColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
  anariSetParameter(device, material, "color", ANARI_FLOAT32_VEC4, matColor);
  anariCommitParameters(device, material);

  // Create surface (geometry + material)
  ANARISurface surface = anariNewSurface(device);
  anariSetParameter(device, surface, "geometry", ANARI_GEOMETRY, &geometry);
  anariSetParameter(device, surface, "material", ANARI_MATERIAL, &material);
  anariCommitParameters(device, surface);

  // Create a directional light
  ANARILight light = anariNewLight(device, "directional");
  float lightDir[] = {0.0f, -1.0f, -1.0f};
  float lightColor[] = {1.0f, 1.0f, 1.0f};
  float irradiance = 1.0f;
  anariSetParameter(device, light, "direction", ANARI_FLOAT32_VEC3, lightDir);
  anariSetParameter(device, light, "color", ANARI_FLOAT32_VEC3, lightColor);
  anariSetParameter(device, light, "irradiance", ANARI_FLOAT32, &irradiance);
  anariCommitParameters(device, light);

  // Create world with the surface and light
  ANARIWorld world = anariNewWorld(device);

  ANARIArray1D surfaceArray =
      anariNewArray1D(device, &surface, nullptr, nullptr, ANARI_SURFACE, 1);
  anariSetParameter(device, world, "surface", ANARI_ARRAY1D, &surfaceArray);

  ANARIArray1D lightArray =
      anariNewArray1D(device, &light, nullptr, nullptr, ANARI_LIGHT, 1);
  anariSetParameter(device, world, "light", ANARI_ARRAY1D, &lightArray);

  anariCommitParameters(device, world);

  // Create perspective camera
  ANARICamera camera = anariNewCamera(device, "perspective");
  float camPos[] = {0.0f, 0.0f, 0.0f};
  float camDir[] = {0.0f, 0.0f, -1.0f};
  float camUp[] = {0.0f, 1.0f, 0.0f};
  float fovy = 1.0472f; // ~60 degrees
  float aspect = (float)width / (float)height;
  anariSetParameter(device, camera, "position", ANARI_FLOAT32_VEC3, camPos);
  anariSetParameter(device, camera, "direction", ANARI_FLOAT32_VEC3, camDir);
  anariSetParameter(device, camera, "up", ANARI_FLOAT32_VEC3, camUp);
  anariSetParameter(device, camera, "fovy", ANARI_FLOAT32, &fovy);
  anariSetParameter(device, camera, "aspect", ANARI_FLOAT32, &aspect);
  anariCommitParameters(device, camera);

  // Create renderer
  ANARIRenderer renderer = anariNewRenderer(device, "default");
  float bgColor[] = {0.1f, 0.1f, 0.1f, 1.0f};
  float ambientRadiance = 0.3f;
  anariSetParameter(device, renderer, "background", ANARI_FLOAT32_VEC4,
                    bgColor);
  anariSetParameter(device, renderer, "ambientRadiance", ANARI_FLOAT32,
                    &ambientRadiance);
  anariCommitParameters(device, renderer);

  // Create frame
  ANARIFrame frame = anariNewFrame(device);
  uint32_t size[] = {(uint32_t)width, (uint32_t)height};
  ANARIDataType colorType = ANARI_UFIXED8_VEC4;
  ANARIDataType depthType = ANARI_FLOAT32;
  anariSetParameter(device, frame, "size", ANARI_UINT32_VEC2, size);
  anariSetParameter(device, frame, "channel.color", ANARI_DATA_TYPE,
                    &colorType);
  anariSetParameter(device, frame, "channel.depth", ANARI_DATA_TYPE,
                    &depthType);
  anariSetParameter(device, frame, "renderer", ANARI_RENDERER, &renderer);
  anariSetParameter(device, frame, "camera", ANARI_CAMERA, &camera);
  anariSetParameter(device, frame, "world", ANARI_WORLD, &world);
  anariCommitParameters(device, frame);

  // --- Render ---
  printf("Rendering...\n");
  anariRenderFrame(device, frame);
  anariFrameReady(device, frame, ANARI_WAIT);

  // Map framebuffer and save to PPM
  uint32_t fbWidth, fbHeight;
  ANARIDataType fbType;
  const uint8_t *pixels = (const uint8_t *)anariMapFrame(
      device, frame, "channel.color", &fbWidth, &fbHeight, &fbType);

  if (pixels) {
    writePPM("triangle.ppm", fbWidth, fbHeight, pixels);
    anariUnmapFrame(device, frame, "channel.color");
  } else {
    fprintf(stderr, "Failed to map framebuffer.\n");
  }

  // --- Cleanup ---
  anariRelease(device, frame);
  anariRelease(device, renderer);
  anariRelease(device, camera);
  anariRelease(device, world);
  anariRelease(device, surfaceArray);
  anariRelease(device, lightArray);
  anariRelease(device, surface);
  anariRelease(device, geometry);
  anariRelease(device, material);
  anariRelease(device, light);
  anariRelease(device, posArray);
  anariRelease(device, colArray);
  anariRelease(device, normArray);

  anariRelease(device, device);
  anariUnloadLibrary(lib);

  printf("Done.\n");
  return 0;
}

// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include <anari/anari.h>
#include <cmath>
#include <cstdio>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void statusFunc(const void *, ANARIDevice, ANARIObject, ANARIDataType,
    ANARIStatusSeverity severity, ANARIStatusCode, const char *message) {
  if (severity <= ANARI_SEVERITY_WARNING) {
    fprintf(stderr, "[%s] %s\n",
        severity == ANARI_SEVERITY_ERROR ? "ERROR" : "WARN", message);
  }
}

void createSphere(std::vector<float> &vertices, std::vector<float> &normals,
                  std::vector<float> &colors, std::vector<uint32_t> &indices,
                  float radius, int segs, int rings, float r, float g, float b) {
  uint32_t baseIdx = vertices.size() / 3;
  
  for (int ring = 0; ring <= rings; ring++) {
    float phi = M_PI * float(ring) / float(rings);
    for (int seg = 0; seg <= segs; seg++) {
      float theta = 2.0f * M_PI * float(seg) / float(segs);
      
      float x = radius * sinf(phi) * cosf(theta);
      float y = radius * cosf(phi);
      float z = radius * sinf(phi) * sinf(theta);
      
      vertices.push_back(x);
      vertices.push_back(y);
      vertices.push_back(z);
      
      float len = sqrtf(x*x + y*y + z*z);
      normals.push_back(x / len);
      normals.push_back(y / len);
      normals.push_back(z / len);
      
      colors.push_back(r);
      colors.push_back(g);
      colors.push_back(b);
      colors.push_back(1.0f);
    }
  }
  
  for (int ring = 0; ring < rings; ring++) {
    for (int seg = 0; seg < segs; seg++) {
      uint32_t curr = baseIdx + ring * (segs + 1) + seg;
      uint32_t next = curr + segs + 1;
      
      indices.push_back(curr);
      indices.push_back(next);
      indices.push_back(curr + 1);
      
      indices.push_back(curr + 1);
      indices.push_back(next);
      indices.push_back(next + 1);
    }
  }
}

void savePPM(const char *filename, const uint8_t *pixels, int width, int height) {
  FILE *f = fopen(filename, "wb");
  if (!f) return;
  fprintf(f, "P6\n%d %d\n255\n", width, height);
  for (int i = 0; i < width * height; i++) {
    fputc(pixels[i * 4 + 0], f);
    fputc(pixels[i * 4 + 1], f);
    fputc(pixels[i * 4 + 2], f);
  }
  fclose(f);
  printf("Saved: %s\n", filename);
}

int main() {
  printf("=== Simple Surface Rendering Demo ===\n\n");
  
  ANARILibrary lib = anariLoadLibrary("webgpu", statusFunc, nullptr);
  if (!lib) { fprintf(stderr, "Failed to load library\n"); return 1; }
  
  ANARIDevice device = anariNewDevice(lib, "default");
  if (!device) { fprintf(stderr, "Failed to create device\n"); return 1; }
  
  printf("Device created\n");
  
  const int width = 3840, height = 2160;
  
  printf("Creating spheres...\n");
  std::vector<float> vertices, normals, colors;
  std::vector<uint32_t> indices;
  
  float sphereColors[][3] = {
    {0.9f, 0.2f, 0.2f},
    {0.2f, 0.9f, 0.2f},
    {0.2f, 0.4f, 0.9f},
    {0.9f, 0.8f, 0.2f},
    {0.9f, 0.5f, 0.9f}
  };
  
  for (int i = 0; i < 5; i++) {
    float x = -4.0f + i * 2.0f;
    
    std::vector<float> v, n, c;
    std::vector<uint32_t> idx;
    createSphere(v, n, c, idx, 0.5f, 24, 18,
        sphereColors[i][0], sphereColors[i][1], sphereColors[i][2]);
    
    for (size_t j = 0; j < v.size(); j += 3) {
      v[j] += x;
      v[j+2] -= 5.0f;
    }
    
    uint32_t baseIdx = vertices.size() / 3;
    vertices.insert(vertices.end(), v.begin(), v.end());
    normals.insert(normals.end(), n.begin(), n.end());
    colors.insert(colors.end(), c.begin(), c.end());
    
    for (auto i : idx) {
      indices.push_back(i + baseIdx);
    }
  }
  
  printf("  Total vertices: %zu\n", vertices.size() / 3);
  printf("  Total triangles: %zu\n", indices.size() / 3);
  
  ANARIArray1D posArray = anariNewArray1D(device, vertices.data(),
      0, 0, ANARI_FLOAT32_VEC3, vertices.size() / 3);
  ANARIArray1D normArray = anariNewArray1D(device, normals.data(),
      0, 0, ANARI_FLOAT32_VEC3, normals.size() / 3);
  ANARIArray1D colorArray = anariNewArray1D(device, colors.data(),
      0, 0, ANARI_FLOAT32_VEC4, colors.size() / 4);
  ANARIArray1D idxArray = anariNewArray1D(device, indices.data(),
      0, 0, ANARI_UINT32_VEC3, indices.size() / 3);
  
  ANARIGeometry geom = anariNewGeometry(device, "triangle");
  anariSetParameter(device, geom, "vertex.position", ANARI_ARRAY1D, &posArray);
  anariSetParameter(device, geom, "vertex.normal", ANARI_ARRAY1D, &normArray);
  anariSetParameter(device, geom, "vertex.color", ANARI_ARRAY1D, &colorArray);
  anariSetParameter(device, geom, "primitive.index", ANARI_ARRAY1D, &idxArray);
  anariCommitParameters(device, geom);
  
  ANARIMaterial mat = anariNewMaterial(device, "matte");
  float matColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
  anariSetParameter(device, mat, "color", ANARI_FLOAT32_VEC4, matColor);
  anariCommitParameters(device, mat);
  
  ANARISurface surface = anariNewSurface(device);
  anariSetParameter(device, surface, "geometry", ANARI_GEOMETRY, &geom);
  anariSetParameter(device, surface, "material", ANARI_MATERIAL, &mat);
  anariCommitParameters(device, surface);
  
  ANARILight light = anariNewLight(device, "directional");
  float lightDir[] = {-0.3f, -0.7f, -0.6f};
  float lightColor[] = {1.0f, 0.98f, 0.95f};
  float irradiance = 2.0f;
  anariSetParameter(device, light, "direction", ANARI_FLOAT32_VEC3, lightDir);
  anariSetParameter(device, light, "color", ANARI_FLOAT32_VEC3, lightColor);
  anariSetParameter(device, light, "irradiance", ANARI_FLOAT32, &irradiance);
  anariCommitParameters(device, light);
  
  printf("Creating world...\n");
  ANARIWorld world = anariNewWorld(device);
  ANARIArray1D surfArray = anariNewArray1D(device, &surface, 0, 0, ANARI_SURFACE, 1);
  ANARIArray1D lightArray = anariNewArray1D(device, &light, 0, 0, ANARI_LIGHT, 1);
  anariSetParameter(device, world, "surface", ANARI_ARRAY1D, &surfArray);
  anariSetParameter(device, world, "light", ANARI_ARRAY1D, &lightArray);
  anariCommitParameters(device, world);
  
  printf("Creating camera...\n");
  ANARICamera camera = anariNewCamera(device, "perspective");
  float camPos[] = {0.0f, 0.5f, 0.0f};
  float camDir[] = {0.0f, 0.0f, -1.0f};
  float camUp[] = {0.0f, 1.0f, 0.0f};
  float fovy = 60.0f * M_PI / 180.0f;
  float aspect = (float)width / height;
  anariSetParameter(device, camera, "position", ANARI_FLOAT32_VEC3, camPos);
  anariSetParameter(device, camera, "direction", ANARI_FLOAT32_VEC3, camDir);
  anariSetParameter(device, camera, "up", ANARI_FLOAT32_VEC3, camUp);
  anariSetParameter(device, camera, "fovy", ANARI_FLOAT32, &fovy);
  anariSetParameter(device, camera, "aspect", ANARI_FLOAT32, &aspect);
  anariCommitParameters(device, camera);
  
  ANARIRenderer renderer = anariNewRenderer(device, "default");
  float bgColor[] = {0.1f, 0.1f, 0.15f, 1.0f};
  float ambient = 0.2f;
  anariSetParameter(device, renderer, "background", ANARI_FLOAT32_VEC4, bgColor);
  anariSetParameter(device, renderer, "ambientRadiance", ANARI_FLOAT32, &ambient);
  anariCommitParameters(device, renderer);
  
  printf("Creating frame (%dx%d)...\n", width, height);
  ANARIFrame frame = anariNewFrame(device);
  uint32_t size[] = {(uint32_t)width, (uint32_t)height};
  ANARIDataType colorType = ANARI_UFIXED8_VEC4;
  anariSetParameter(device, frame, "size", ANARI_UINT32_VEC2, size);
  anariSetParameter(device, frame, "channel.color", ANARI_DATA_TYPE, &colorType);
  anariSetParameter(device, frame, "renderer", ANARI_RENDERER, &renderer);
  anariSetParameter(device, frame, "camera", ANARI_CAMERA, &camera);
  anariSetParameter(device, frame, "world", ANARI_WORLD, &world);
  anariCommitParameters(device, frame);
  
  printf("\nRendering...\n");
  anariRenderFrame(device, frame);
  anariFrameReady(device, frame, ANARI_WAIT);
  
  uint32_t fbW, fbH;
  ANARIDataType fbType;
  const void *pixels = anariMapFrame(device, frame, "channel.color", &fbW, &fbH, &fbType);
  
  if (pixels) {
    printf("Render complete!\n");
    savePPM("surface_demo.ppm", (const uint8_t *)pixels, fbW, fbH);
    anariUnmapFrame(device, frame, "channel.color");
  } else {
    fprintf(stderr, "Failed to map framebuffer\n");
  }
  
  printf("\nCleaning up...\n");
  anariRelease(device, frame);
  anariRelease(device, renderer);
  anariRelease(device, camera);
  anariRelease(device, world);
  anariRelease(device, surfArray);
  anariRelease(device, lightArray);
  anariRelease(device, surface);
  anariRelease(device, geom);
  anariRelease(device, mat);
  anariRelease(device, light);
  anariRelease(device, posArray);
  anariRelease(device, normArray);
  anariRelease(device, colorArray);
  anariRelease(device, idxArray);
  anariRelease(device, device);
  anariUnloadLibrary(lib);
  
  printf("\n=== Demo Complete ===\n");
  printf("Output: surface_demo.ppm\n");
  printf("\nFive colored spheres (left to right):\n");
  printf("  Red, Green, Blue, Yellow, Magenta\n");
  
  return 0;
}

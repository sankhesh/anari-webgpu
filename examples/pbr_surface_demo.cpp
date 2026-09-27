// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include <anari/anari.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void statusFunc(const void *, ANARIDevice, ANARIObject, ANARIDataType,
    ANARIStatusSeverity severity, ANARIStatusCode, const char *message) {
  if (severity == ANARI_SEVERITY_ERROR) {
    fprintf(stderr, "[ERROR] %s\n", message);
  } else if (severity == ANARI_SEVERITY_WARNING) {
    fprintf(stderr, "[WARN] %s\n", message);
  } else if (severity == ANARI_SEVERITY_INFO) {
    fprintf(stdout, "[INFO] %s\n", message);
  }
}

struct Mesh {
  std::vector<float> vertices;
  std::vector<float> normals;
  std::vector<uint32_t> indices;
};

Mesh createSphere(float radius, int segments, int rings) {
  Mesh mesh;
  
  for (int ring = 0; ring <= rings; ring++) {
    float phi = M_PI * float(ring) / float(rings);
    for (int seg = 0; seg <= segments; seg++) {
      float theta = 2.0f * M_PI * float(seg) / float(segments);
      
      float x = radius * sinf(phi) * cosf(theta);
      float y = radius * cosf(phi);
      float z = radius * sinf(phi) * sinf(theta);
      
      mesh.vertices.push_back(x);
      mesh.vertices.push_back(y);
      mesh.vertices.push_back(z);
      
      float len = sqrtf(x*x + y*y + z*z);
      mesh.normals.push_back(x / len);
      mesh.normals.push_back(y / len);
      mesh.normals.push_back(z / len);
    }
  }
  
  for (int ring = 0; ring < rings; ring++) {
    for (int seg = 0; seg < segments; seg++) {
      int curr = ring * (segments + 1) + seg;
      int next = curr + segments + 1;
      
      mesh.indices.push_back(curr);
      mesh.indices.push_back(next);
      mesh.indices.push_back(curr + 1);
      
      mesh.indices.push_back(curr + 1);
      mesh.indices.push_back(next);
      mesh.indices.push_back(next + 1);
    }
  }
  
  return mesh;
}

Mesh createPlane(float size, int divisions) {
  Mesh mesh;
  float step = size / divisions;
  float halfSize = size * 0.5f;
  
  for (int z = 0; z <= divisions; z++) {
    for (int x = 0; x <= divisions; x++) {
      float px = -halfSize + x * step;
      float pz = -halfSize + z * step;
      
      mesh.vertices.push_back(px);
      mesh.vertices.push_back(-1.0f);
      mesh.vertices.push_back(pz);
      
      mesh.normals.push_back(0.0f);
      mesh.normals.push_back(1.0f);
      mesh.normals.push_back(0.0f);
    }
  }
  
  for (int z = 0; z < divisions; z++) {
    for (int x = 0; x < divisions; x++) {
      int curr = z * (divisions + 1) + x;
      int next = curr + divisions + 1;
      
      mesh.indices.push_back(curr);
      mesh.indices.push_back(next);
      mesh.indices.push_back(curr + 1);
      
      mesh.indices.push_back(curr + 1);
      mesh.indices.push_back(next);
      mesh.indices.push_back(next + 1);
    }
  }
  
  return mesh;
}

void savePPM(const char *filename, const uint8_t *pixels, int width, int height) {
  FILE *file = fopen(filename, "wb");
  if (!file) {
    fprintf(stderr, "Failed to open %s for writing\n", filename);
    return;
  }
  
  fprintf(file, "P6\n%d %d\n255\n", width, height);
  for (int i = 0; i < width * height; i++) {
    fputc(pixels[i * 4 + 0], file);
    fputc(pixels[i * 4 + 1], file);
    fputc(pixels[i * 4 + 2], file);
  }
  
  fclose(file);
  printf("Saved image: %s\n", filename);
}

int main(int argc, char **argv) {
  printf("=== ANARI WebGPU PBR Surface Rendering Demo ===\n\n");
  
  ANARILibrary lib = anariLoadLibrary("webgpu", statusFunc, nullptr);
  if (!lib) {
    fprintf(stderr, "Failed to load webgpu library\n");
    return 1;
  }
  
  ANARIDevice device = anariNewDevice(lib, "default");
  if (!device) {
    fprintf(stderr, "Failed to create device\n");
    anariUnloadLibrary(lib);
    return 1;
  }
  
  printf("Device created successfully\n");
  
  const int width = 3840;
  const int height = 2160;
  
  printf("Creating geometries...\n");
  
  const int numSpheres = 5;
  std::vector<ANARISurface> surfaces;
  
  Mesh sphereMesh = createSphere(0.5f, 32, 24);

  std::vector<std::vector<float>> sphereVerts(numSpheres);

  for (int i = 0; i < numSpheres; i++) {
    float xPos = -4.0f + i * 2.0f;
    sphereVerts[i] = sphereMesh.vertices;
    for (size_t j = 0; j + 2 < sphereVerts[i].size(); j += 3) {
      sphereVerts[i][j]     += xPos;
      sphereVerts[i][j + 2] -= 5.0f;
    }

    ANARIArray1D positions = anariNewArray1D(device, sphereVerts[i].data(),
        0, 0, ANARI_FLOAT32_VEC3, sphereVerts[i].size() / 3);
    ANARIArray1D normals = anariNewArray1D(device, sphereMesh.normals.data(),
        0, 0, ANARI_FLOAT32_VEC3, sphereMesh.normals.size() / 3);
    ANARIArray1D indices = anariNewArray1D(device, sphereMesh.indices.data(),
        0, 0, ANARI_UINT32_VEC3, sphereMesh.indices.size() / 3);
    
    ANARIGeometry geom = anariNewGeometry(device, "triangle");
    anariSetParameter(device, geom, "vertex.position", ANARI_ARRAY1D, &positions);
    anariSetParameter(device, geom, "vertex.normal", ANARI_ARRAY1D, &normals);
    anariSetParameter(device, geom, "primitive.index", ANARI_ARRAY1D, &indices);
    anariCommitParameters(device, geom);
    
    ANARIMaterial mat = anariNewMaterial(device, "physicallyBased");
    
    float baseColors[][4] = {
      {0.8f, 0.1f, 0.1f, 1.0f},
      {0.1f, 0.8f, 0.1f, 1.0f},
      {0.1f, 0.3f, 0.9f, 1.0f},
      {1.0f, 0.86f, 0.14f, 1.0f},
      {0.95f, 0.95f, 0.98f, 1.0f}
    };
    
    float metallic = (float)i / (float)(numSpheres - 1);
    
    float roughness = 0.3f;
    
    anariSetParameter(device, mat, "baseColor", ANARI_FLOAT32_VEC3, baseColors[i]);
    anariSetParameter(device, mat, "metallic", ANARI_FLOAT32, &metallic);
    anariSetParameter(device, mat, "roughness", ANARI_FLOAT32, &roughness);
    anariCommitParameters(device, mat);
    
    ANARISurface surface = anariNewSurface(device);
    anariSetParameter(device, surface, "geometry", ANARI_GEOMETRY, &geom);
    anariSetParameter(device, surface, "material", ANARI_MATERIAL, &mat);
    
    anariCommitParameters(device, surface);
    
    surfaces.push_back(surface);
    
    anariRelease(device, positions);
    anariRelease(device, normals);
    anariRelease(device, indices);
    anariRelease(device, geom);
    anariRelease(device, mat);
    
    printf("  Sphere %d: metallic=%.2f, color=%.2f,%.2f,%.2f\n",
        i, metallic, baseColors[i][0], baseColors[i][1], baseColors[i][2]);
  }
  
  printf("Creating ground plane...\n");
  Mesh planeMesh = createPlane(20.0f, 20);

  for (size_t j = 0; j + 2 < planeMesh.vertices.size(); j += 3) {
    planeMesh.vertices[j + 1] -= 0.5f;
    planeMesh.vertices[j + 2] -= 5.0f;
  }

  ANARIArray1D planePos = anariNewArray1D(device, planeMesh.vertices.data(),
      0, 0, ANARI_FLOAT32_VEC3, planeMesh.vertices.size() / 3);
  ANARIArray1D planeNorm = anariNewArray1D(device, planeMesh.normals.data(),
      0, 0, ANARI_FLOAT32_VEC3, planeMesh.normals.size() / 3);
  ANARIArray1D planeIdx = anariNewArray1D(device, planeMesh.indices.data(),
      0, 0, ANARI_UINT32_VEC3, planeMesh.indices.size() / 3);
  
  ANARIGeometry planeGeom = anariNewGeometry(device, "triangle");
  anariSetParameter(device, planeGeom, "vertex.position", ANARI_ARRAY1D, &planePos);
  anariSetParameter(device, planeGeom, "vertex.normal", ANARI_ARRAY1D, &planeNorm);
  anariSetParameter(device, planeGeom, "primitive.index", ANARI_ARRAY1D, &planeIdx);
  anariCommitParameters(device, planeGeom);
  
  ANARIMaterial planeMat = anariNewMaterial(device, "matte");
  float planeColor[] = {0.5f, 0.5f, 0.5f, 1.0f};
  anariSetParameter(device, planeMat, "color", ANARI_FLOAT32_VEC4, planeColor);
  anariCommitParameters(device, planeMat);
  
  ANARISurface planeSurface = anariNewSurface(device);
  anariSetParameter(device, planeSurface, "geometry", ANARI_GEOMETRY, &planeGeom);
  anariSetParameter(device, planeSurface, "material", ANARI_MATERIAL, &planeMat);
  anariCommitParameters(device, planeSurface);
  
  surfaces.push_back(planeSurface);
  
  printf("Creating lights...\n");
  
  ANARILight keyLight = anariNewLight(device, "directional");
  float keyDir[] = {-0.3f, -0.7f, -0.6f};
  float keyColor[] = {1.0f, 0.98f, 0.95f};
  float keyIntensity = 2.0f;
  anariSetParameter(device, keyLight, "direction", ANARI_FLOAT32_VEC3, keyDir);
  anariSetParameter(device, keyLight, "color", ANARI_FLOAT32_VEC3, keyColor);
  anariSetParameter(device, keyLight, "irradiance", ANARI_FLOAT32, &keyIntensity);
  anariCommitParameters(device, keyLight);
  
  ANARILight fillLight = anariNewLight(device, "directional");
  float fillDir[] = {0.8f, -0.3f, -0.5f};
  float fillColor[] = {0.6f, 0.7f, 0.9f};
  float fillIntensity = 0.5f;
  anariSetParameter(device, fillLight, "direction", ANARI_FLOAT32_VEC3, fillDir);
  anariSetParameter(device, fillLight, "color", ANARI_FLOAT32_VEC3, fillColor);
  anariSetParameter(device, fillLight, "irradiance", ANARI_FLOAT32, &fillIntensity);
  anariCommitParameters(device, fillLight);
  
  ANARILight rimLight = anariNewLight(device, "directional");
  float rimDir[] = {0.0f, 0.3f, 1.0f};
  float rimColor[] = {1.0f, 0.9f, 0.8f};
  float rimIntensity = 1.0f;
  anariSetParameter(device, rimLight, "direction", ANARI_FLOAT32_VEC3, rimDir);
  anariSetParameter(device, rimLight, "color", ANARI_FLOAT32_VEC3, rimColor);
  anariSetParameter(device, rimLight, "irradiance", ANARI_FLOAT32, &rimIntensity);
  anariCommitParameters(device, rimLight);
  
  ANARILight lights[] = {keyLight, fillLight, rimLight};
  
  printf("Creating world...\n");
  ANARIWorld world = anariNewWorld(device);
  
  ANARIArray1D surfaceArray = anariNewArray1D(device, surfaces.data(),
      0, 0, ANARI_SURFACE, surfaces.size());
  ANARIArray1D lightArray = anariNewArray1D(device, lights, 0, 0, ANARI_LIGHT, 3);
  
  anariSetParameter(device, world, "surface", ANARI_ARRAY1D, &surfaceArray);
  anariSetParameter(device, world, "light", ANARI_ARRAY1D, &lightArray);
  anariCommitParameters(device, world);
  
  printf("Creating camera...\n");
  ANARICamera camera = anariNewCamera(device, "perspective");
  float camPos[] = {0.0f, 1.5f, 2.0f};
  float camDir[] = {0.0f, -0.3f, -1.0f};
  float camUp[] = {0.0f, 1.0f, 0.0f};
  float fovy = 60.0f * M_PI / 180.0f;
  float aspect = (float)width / (float)height;
  
  anariSetParameter(device, camera, "position", ANARI_FLOAT32_VEC3, camPos);
  anariSetParameter(device, camera, "direction", ANARI_FLOAT32_VEC3, camDir);
  anariSetParameter(device, camera, "up", ANARI_FLOAT32_VEC3, camUp);
  anariSetParameter(device, camera, "fovy", ANARI_FLOAT32, &fovy);
  anariSetParameter(device, camera, "aspect", ANARI_FLOAT32, &aspect);
  anariCommitParameters(device, camera);
  
  printf("Creating renderer...\n");
  ANARIRenderer renderer = anariNewRenderer(device, "default");
  float bgColor[] = {0.02f, 0.02f, 0.05f, 1.0f};
  float ambient = 0.15f;
  anariSetParameter(device, renderer, "background", ANARI_FLOAT32_VEC4, bgColor);
  anariSetParameter(device, renderer, "ambientRadiance", ANARI_FLOAT32, &ambient);
  anariCommitParameters(device, renderer);
  
  printf("Creating frame (%dx%d)...\n", width, height);
  ANARIFrame frame = anariNewFrame(device);
  uint32_t size[] = {(uint32_t)width, (uint32_t)height};
  ANARIDataType colorType = ANARI_UFIXED8_RGBA_SRGB;
  
  anariSetParameter(device, frame, "size", ANARI_UINT32_VEC2, size);
  anariSetParameter(device, frame, "channel.color", ANARI_DATA_TYPE, &colorType);
  anariSetParameter(device, frame, "renderer", ANARI_RENDERER, &renderer);
  anariSetParameter(device, frame, "camera", ANARI_CAMERA, &camera);
  anariSetParameter(device, frame, "world", ANARI_WORLD, &world);
  anariCommitParameters(device, frame);
  
  printf("\nRendering...\n");
  anariRenderFrame(device, frame);
  anariFrameReady(device, frame, ANARI_WAIT);
  
  uint32_t fbWidth, fbHeight;
  ANARIDataType fbType;
  const void *pixels = anariMapFrame(device, frame, "channel.color",
      &fbWidth, &fbHeight, &fbType);
  
  if (pixels) {
    printf("Render complete! (%dx%d)\n", fbWidth, fbHeight);
    savePPM("pbr_surface_demo.ppm", (const uint8_t *)pixels, fbWidth, fbHeight);
    anariUnmapFrame(device, frame, "channel.color");
  } else {
    fprintf(stderr, "Failed to map framebuffer\n");
  }
  
  printf("\nCleaning up...\n");
  anariRelease(device, frame);
  anariRelease(device, renderer);
  anariRelease(device, camera);
  anariRelease(device, world);
  anariRelease(device, surfaceArray);
  anariRelease(device, lightArray);
  
  for (auto surf : surfaces) {
    anariRelease(device, surf);
  }
  
  anariRelease(device, keyLight);
  anariRelease(device, fillLight);
  anariRelease(device, rimLight);
  anariRelease(device, planePos);
  anariRelease(device, planeNorm);
  anariRelease(device, planeIdx);
  anariRelease(device, planeGeom);
  anariRelease(device, planeMat);
  
  anariRelease(device, device);
  anariUnloadLibrary(lib);
  
  printf("\n=== Demo Complete ===\n");
  printf("Output saved to: pbr_surface_demo.ppm\n");
  printf("\nMaterial progression (left to right):\n");
  printf("  1. Red dielectric (metallic=0.00)\n");
  printf("  2. Green (metallic=0.25)\n");
  printf("  3. Blue (metallic=0.50)\n");
  printf("  4. Gold (metallic=0.75)\n");
  printf("  5. Silver (metallic=1.00)\n");
  
  return 0;
}

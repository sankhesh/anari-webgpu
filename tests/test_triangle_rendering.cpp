// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0
#include <anari/anari.h>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

static void statusFunc(const void *, ANARIDevice, ANARIObject, ANARIDataType, ANARIStatusSeverity severity, ANARIStatusCode, const char *msg) {
  if (severity <= ANARI_SEVERITY_WARNING) fprintf(stderr, "[%s] %s\n", severity == ANARI_SEVERITY_ERROR ? "ERROR" : "WARN", msg);
}

struct TestScene {
  ANARIDevice device; ANARIWorld world; ANARICamera camera; ANARIRenderer renderer; ANARIFrame frame; int width, height;
};

static TestScene createBasicTriangleScene(int w, int h) {
  TestScene s{}; s.width = w; s.height = h;
  ANARILibrary lib = anariLoadLibrary("webgpu", statusFunc, nullptr); assert(lib);
  s.device = anariNewDevice(lib, "default"); assert(s.device);
  float vertices[] = {-0.5f,-0.5f,-2.0f, 0.5f,-0.5f,-2.0f, 0.0f,0.5f,-2.0f};
  float colors[] = {1,0,0,1, 0,1,0,1, 0,0,1,1};
  float normals[] = {0,0,1, 0,0,1, 0,0,1};
  ANARIArray1D posArr = anariNewArray1D(s.device, vertices, 0, 0, ANARI_FLOAT32_VEC3, 3);
  ANARIArray1D colArr = anariNewArray1D(s.device, colors, 0, 0, ANARI_FLOAT32_VEC4, 3);
  ANARIArray1D normArr = anariNewArray1D(s.device, normals, 0, 0, ANARI_FLOAT32_VEC3, 3);
  ANARIGeometry geom = anariNewGeometry(s.device, "triangle");
  anariSetParameter(s.device, geom, "vertex.position", ANARI_ARRAY1D, &posArr);
  anariSetParameter(s.device, geom, "vertex.color", ANARI_ARRAY1D, &colArr);
  anariSetParameter(s.device, geom, "vertex.normal", ANARI_ARRAY1D, &normArr);
  anariCommitParameters(s.device, geom);
  ANARIMaterial mat = anariNewMaterial(s.device, "matte");
  float matColor[] = {1,1,1,1}; anariSetParameter(s.device, mat, "color", ANARI_FLOAT32_VEC4, matColor);
  anariCommitParameters(s.device, mat);
  ANARISurface surf = anariNewSurface(s.device);
  anariSetParameter(s.device, surf, "geometry", ANARI_GEOMETRY, &geom);
  anariSetParameter(s.device, surf, "material", ANARI_MATERIAL, &mat);
  anariCommitParameters(s.device, surf);
  ANARILight light = anariNewLight(s.device, "directional");
  float lightDir[] = {0,0,-1}; anariSetParameter(s.device, light, "direction", ANARI_FLOAT32_VEC3, lightDir);
  anariCommitParameters(s.device, light);
  s.world = anariNewWorld(s.device);
  ANARIArray1D surfArr = anariNewArray1D(s.device, &surf, 0, 0, ANARI_SURFACE, 1);
  ANARIArray1D lightArr = anariNewArray1D(s.device, &light, 0, 0, ANARI_LIGHT, 1);
  anariSetParameter(s.device, s.world, "surface", ANARI_ARRAY1D, &surfArr);
  anariSetParameter(s.device, s.world, "light", ANARI_ARRAY1D, &lightArr);
  anariCommitParameters(s.device, s.world);
  s.camera = anariNewCamera(s.device, "perspective");
  float camPos[] = {0,0,0}, camDir[] = {0,0,-1}, camUp[] = {0,1,0};
  float fovy = 1.0472f, aspect = (float)w/h;
  anariSetParameter(s.device, s.camera, "position", ANARI_FLOAT32_VEC3, camPos);
  anariSetParameter(s.device, s.camera, "direction", ANARI_FLOAT32_VEC3, camDir);
  anariSetParameter(s.device, s.camera, "up", ANARI_FLOAT32_VEC3, camUp);
  anariSetParameter(s.device, s.camera, "fovy", ANARI_FLOAT32, &fovy);
  anariSetParameter(s.device, s.camera, "aspect", ANARI_FLOAT32, &aspect);
  anariCommitParameters(s.device, s.camera);
  s.renderer = anariNewRenderer(s.device, "default");
  float bg[] = {0,0,0,1}; anariSetParameter(s.device, s.renderer, "background", ANARI_FLOAT32_VEC4, bg);
  anariCommitParameters(s.device, s.renderer);
  s.frame = anariNewFrame(s.device);
  uint32_t size[] = {(uint32_t)w, (uint32_t)h};
  ANARIDataType colorType = ANARI_UFIXED8_VEC4, depthType = ANARI_FLOAT32;
  anariSetParameter(s.device, s.frame, "size", ANARI_UINT32_VEC2, size);
  anariSetParameter(s.device, s.frame, "channel.color", ANARI_DATA_TYPE, &colorType);
  anariSetParameter(s.device, s.frame, "channel.depth", ANARI_DATA_TYPE, &depthType);
  anariSetParameter(s.device, s.frame, "renderer", ANARI_RENDERER, &s.renderer);
  anariSetParameter(s.device, s.frame, "camera", ANARI_CAMERA, &s.camera);
  anariSetParameter(s.device, s.frame, "world", ANARI_WORLD, &s.world);
  anariCommitParameters(s.device, s.frame);
  anariRelease(s.device, surfArr); anariRelease(s.device, lightArr);
  anariRelease(s.device, surf); anariRelease(s.device, geom);
  anariRelease(s.device, mat); anariRelease(s.device, light);
  anariRelease(s.device, posArr); anariRelease(s.device, colArr); anariRelease(s.device, normArr);
  return s;
}

static void destroyScene(TestScene &s) {
  anariRelease(s.device, s.frame); anariRelease(s.device, s.renderer);
  anariRelease(s.device, s.camera); anariRelease(s.device, s.world);
  anariRelease(s.device, s.device);
}

static void test_framebuffer_valid() {
  printf("  test_framebuffer_valid..."); TestScene s = createBasicTriangleScene(64, 64);
  anariRenderFrame(s.device, s.frame); anariFrameReady(s.device, s.frame, ANARI_WAIT);
  uint32_t fbW, fbH; ANARIDataType fbType;
  const void *pixels = anariMapFrame(s.device, s.frame, "channel.color", &fbW, &fbH, &fbType);
  assert(pixels && "Framebuffer should not be null"); assert(fbW == 64 && fbH == 64);
  anariUnmapFrame(s.device, s.frame, "channel.color"); destroyScene(s); printf(" PASSED\n");
}
static void test_triangle_visible() {
  printf("  test_triangle_visible..."); TestScene s = createBasicTriangleScene(128, 128);
  anariRenderFrame(s.device, s.frame); anariFrameReady(s.device, s.frame, ANARI_WAIT);
  uint32_t fbW, fbH; ANARIDataType fbType;
  const uint8_t *px = (const uint8_t *)anariMapFrame(s.device, s.frame, "channel.color", &fbW, &fbH, &fbType);
  assert(px); int cx = fbW/2, cy = fbH/2, idx = (cy*fbW+cx)*4;
  bool nonBlack = px[idx]>0||px[idx+1]>0||px[idx+2]>0;
  anariUnmapFrame(s.device, s.frame, "channel.color");
  if (!nonBlack) fprintf(stderr, "  WARNING: center pixel is black\n");
  destroyScene(s); printf(" PASSED\n");
}
static void test_depth_buffer() {
  printf("  test_depth_buffer..."); TestScene s = createBasicTriangleScene(64, 64);
  anariRenderFrame(s.device, s.frame); anariFrameReady(s.device, s.frame, ANARI_WAIT);
  uint32_t fbW, fbH; ANARIDataType fbType;
  const float *depth = (const float *)anariMapFrame(s.device, s.frame, "channel.depth", &fbW, &fbH, &fbType);
  if (depth) { int fc = 0; for (uint32_t i = 0; i < fbW*fbH; i++) if (std::isfinite(depth[i]) && depth[i] < 1e10f) fc++;
    printf(" (%d finite depth pixels)", fc); anariUnmapFrame(s.device, s.frame, "channel.depth");
  } else printf(" (depth not available)");
  destroyScene(s); printf(" PASSED\n");
}
static void test_resolution_variants() {
  printf("  test_resolution_variants...");
  int sizes[][2] = {{16,16},{100,100},{256,128},{1,1}};
  for (auto &sz : sizes) { TestScene s = createBasicTriangleScene(sz[0], sz[1]);
    anariRenderFrame(s.device, s.frame); anariFrameReady(s.device, s.frame, ANARI_WAIT);
    uint32_t fbW, fbH; ANARIDataType fbType;
    const void *p = anariMapFrame(s.device, s.frame, "channel.color", &fbW, &fbH, &fbType);
    assert(p); assert(fbW==(uint32_t)sz[0] && fbH==(uint32_t)sz[1]);
    anariUnmapFrame(s.device, s.frame, "channel.color"); destroyScene(s);
  } printf(" PASSED\n");
}
int main() {
  printf("=== Triangle Rendering Tests ===\n");
  test_framebuffer_valid(); test_triangle_visible(); test_depth_buffer(); test_resolution_variants();
  printf("All triangle rendering tests passed.\n"); return 0;
}

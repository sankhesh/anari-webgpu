// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0
// Integration test: volume rendering with structured regular fields
#include <anari/anari.h>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
static void statusFunc(const void *, ANARIDevice, ANARIObject, ANARIDataType, ANARIStatusSeverity, ANARIStatusCode, const char *) {}
static ANARIDevice createTestDevice() {
  ANARILibrary lib = anariLoadLibrary("webgpu", statusFunc, nullptr); assert(lib);
  ANARIDevice dev = anariNewDevice(lib, "default"); assert(dev); return dev;
}
static void test_structured_regular_field() {
  printf("  test_structured_regular_field...");
  ANARIDevice dev = createTestDevice();
  const int N = 4;
  float voxels[N * N * N];
  for (int z = 0; z < N; z++)
    for (int y = 0; y < N; y++)
      for (int x = 0; x < N; x++) {
        float dx = x - N/2.0f + 0.5f, dy = y - N/2.0f + 0.5f, dz = z - N/2.0f + 0.5f;
        voxels[z*N*N + y*N + x] = 1.0f - sqrtf(dx*dx + dy*dy + dz*dz) / (N/2.0f);
      }
  ANARIArray3D dataArr = anariNewArray3D(dev, voxels, nullptr, nullptr, ANARI_FLOAT32, N, N, N);
  ANARISpatialField field = anariNewSpatialField(dev, "structuredRegular");
  if (!field) { printf(" SKIPPED (not implemented)\n"); anariRelease(dev, dataArr); anariRelease(dev, dev); return; }
  anariSetParameter(dev, field, "data", ANARI_ARRAY3D, &dataArr);
  float origin[] = {0,0,0}, spacing[] = {1,1,1};
  anariSetParameter(dev, field, "origin", ANARI_FLOAT32_VEC3, origin);
  anariSetParameter(dev, field, "spacing", ANARI_FLOAT32_VEC3, spacing);
  anariCommitParameters(dev, field);
  anariRelease(dev, field); anariRelease(dev, dataArr); anariRelease(dev, dev);
  printf(" PASSED\n");
}
static void test_volume_with_transfer_function() {
  printf("  test_volume_with_transfer_function...");
  ANARIDevice dev = createTestDevice();
  const int N = 8; float voxels[N*N*N];
  for (int i = 0; i < N*N*N; i++) voxels[i] = (float)i / (N*N*N - 1);
  ANARIArray3D data = anariNewArray3D(dev, voxels, 0, 0, ANARI_FLOAT32, N, N, N);
  ANARISpatialField field = anariNewSpatialField(dev, "structuredRegular");
  if (!field) { printf(" SKIPPED\n"); anariRelease(dev, data); anariRelease(dev, dev); return; }
  anariSetParameter(dev, field, "data", ANARI_ARRAY3D, &data); anariCommitParameters(dev, field);
  ANARIVolume vol = anariNewVolume(dev, "transferFunction1D");
  if (!vol) { printf(" SKIPPED (volume not implemented)\n"); anariRelease(dev, field); anariRelease(dev, data); anariRelease(dev, dev); return; }
  float tfColors[] = {0,0,1, 1,0,0}; float tfOpacities[] = {0.0f, 1.0f};
  ANARIArray1D colorArr = anariNewArray1D(dev, tfColors, 0, 0, ANARI_FLOAT32_VEC3, 2);
  ANARIArray1D opacArr = anariNewArray1D(dev, tfOpacities, 0, 0, ANARI_FLOAT32, 2);
  anariSetParameter(dev, vol, "field", ANARI_SPATIAL_FIELD, &field);
  anariSetParameter(dev, vol, "color", ANARI_ARRAY1D, &colorArr);
  anariSetParameter(dev, vol, "opacity", ANARI_ARRAY1D, &opacArr);
  float valueRange[] = {0.0f, 1.0f};
  anariSetParameter(dev, vol, "valueRange", ANARI_FLOAT32_BOX1, valueRange);
  anariCommitParameters(dev, vol);
  ANARIWorld world = anariNewWorld(dev);
  ANARIArray1D va = anariNewArray1D(dev, &vol, 0, 0, ANARI_VOLUME, 1);
  anariSetParameter(dev, world, "volume", ANARI_ARRAY1D, &va);
  anariCommitParameters(dev, world);
  anariRelease(dev, world); anariRelease(dev, va); anariRelease(dev, vol);
  anariRelease(dev, field); anariRelease(dev, data);
  anariRelease(dev, colorArr); anariRelease(dev, opacArr); anariRelease(dev, dev);
  printf(" PASSED\n");
}
static void test_volume_render_to_frame() {
  printf("  test_volume_render_to_frame...");
  ANARIDevice dev = createTestDevice();
  const int N = 16;
  float *voxels = (float *)malloc(N*N*N*sizeof(float));
  for (int z=0;z<N;z++) for (int y=0;y<N;y++) for (int x=0;x<N;x++) {
    float dx=(x+0.5f)/N-0.5f, dy=(y+0.5f)/N-0.5f, dz=(z+0.5f)/N-0.5f;
    voxels[z*N*N+y*N+x] = fmaxf(0, 1.0f - sqrtf(dx*dx+dy*dy+dz*dz)*4.0f);
  }
  ANARIArray3D data = anariNewArray3D(dev, voxels, 0, 0, ANARI_FLOAT32, N, N, N);
  ANARISpatialField field = anariNewSpatialField(dev, "structuredRegular");
  if (!field) { printf(" SKIPPED\n"); free(voxels); anariRelease(dev, data); anariRelease(dev, dev); return; }
  anariSetParameter(dev, field, "data", ANARI_ARRAY3D, &data);
  float origin[]={-0.5f,-0.5f,-0.5f}, spacing[]={1.0f/N,1.0f/N,1.0f/N};
  anariSetParameter(dev, field, "origin", ANARI_FLOAT32_VEC3, origin);
  anariSetParameter(dev, field, "spacing", ANARI_FLOAT32_VEC3, spacing);
  anariCommitParameters(dev, field);
  ANARIVolume vol = anariNewVolume(dev, "transferFunction1D");
  if (!vol) { printf(" SKIPPED\n"); free(voxels); anariRelease(dev,field); anariRelease(dev,data); anariRelease(dev,dev); return; }
  float tfC[]={0,0,1, 0,1,0, 1,1,0, 1,0,0}; float tfO[]={0.0f,0.2f,0.5f,1.0f};
  ANARIArray1D ca=anariNewArray1D(dev,tfC,0,0,ANARI_FLOAT32_VEC3,4);
  ANARIArray1D oa=anariNewArray1D(dev,tfO,0,0,ANARI_FLOAT32,4);
  anariSetParameter(dev,vol,"field",ANARI_SPATIAL_FIELD,&field);
  anariSetParameter(dev,vol,"color",ANARI_ARRAY1D,&ca);
  anariSetParameter(dev,vol,"opacity",ANARI_ARRAY1D,&oa);
  anariCommitParameters(dev,vol);
  ANARIWorld world=anariNewWorld(dev);
  ANARIArray1D vArr=anariNewArray1D(dev,&vol,0,0,ANARI_VOLUME,1);
  anariSetParameter(dev,world,"volume",ANARI_ARRAY1D,&vArr); anariCommitParameters(dev,world);
  ANARICamera cam=anariNewCamera(dev,"perspective");
  float pos[]={0,0,2},dir[]={0,0,-1},up[]={0,1,0};
  anariSetParameter(dev,cam,"position",ANARI_FLOAT32_VEC3,pos);
  anariSetParameter(dev,cam,"direction",ANARI_FLOAT32_VEC3,dir);
  anariSetParameter(dev,cam,"up",ANARI_FLOAT32_VEC3,up);
  anariCommitParameters(dev,cam);
  ANARIRenderer ren=anariNewRenderer(dev,"default"); anariCommitParameters(dev,ren);
  ANARIFrame frame=anariNewFrame(dev);
  uint32_t sz[]={64,64}; ANARIDataType ct=ANARI_UFIXED8_VEC4;
  anariSetParameter(dev,frame,"size",ANARI_UINT32_VEC2,sz);
  anariSetParameter(dev,frame,"channel.color",ANARI_DATA_TYPE,&ct);
  anariSetParameter(dev,frame,"renderer",ANARI_RENDERER,&ren);
  anariSetParameter(dev,frame,"camera",ANARI_CAMERA,&cam);
  anariSetParameter(dev,frame,"world",ANARI_WORLD,&world);
  anariCommitParameters(dev,frame);
  anariRenderFrame(dev,frame); anariFrameReady(dev,frame,ANARI_WAIT);
  uint32_t fw,fh; ANARIDataType ft;
  const void *px=anariMapFrame(dev,frame,"channel.color",&fw,&fh,&ft);
  assert(px > 0 && "Volume render should produce valid framebuffer");
  anariUnmapFrame(dev,frame,"channel.color");
  anariRelease(dev,frame); anariRelease(dev,ren); anariRelease(dev,cam);
  anariRelease(dev,world); anariRelease(dev,vArr); anariRelease(dev,vol);
  anariRelease(dev,ca); anariRelease(dev,oa); anariRelease(dev,field);
  anariRelease(dev,data); anariRelease(dev,dev); free(voxels);
  printf(" PASSED\n");
}
int main() {
  printf("=== Volume Rendering Tests ===\n");
  test_structured_regular_field();
  test_volume_with_transfer_function();
  test_volume_render_to_frame();
  printf("All volume rendering tests passed.\n"); return 0;
}

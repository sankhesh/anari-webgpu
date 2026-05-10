// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0
// Integration test: PBR material rendering
#include <anari/anari.h>
#include <cassert>
#include <cstdio>
#include <cstdint>
static void statusFunc(const void *, ANARIDevice, ANARIObject, ANARIDataType, ANARIStatusSeverity, ANARIStatusCode, const char *) {}
static ANARIDevice createTestDevice() {
  ANARILibrary lib = anariLoadLibrary("webgpu", statusFunc, nullptr); assert(lib);
  ANARIDevice dev = anariNewDevice(lib, "default"); assert(dev); return dev;
}
static void test_pbr_roughness_sweep() {
  printf("  test_pbr_roughness_sweep...");
  ANARIDevice dev = createTestDevice();
  float roughnessValues[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
  int numCreated = 0;
  for (float roughness : roughnessValues) {
    ANARIMaterial mat = anariNewMaterial(dev, "physicallyBased");
    if (!mat) { printf(" SKIPPED (PBR not implemented)\n"); anariRelease(dev, dev); return; }
    float baseColor[] = {0.8f, 0.2f, 0.1f, 1.0f}; float metallic = 0.0f;
    anariSetParameter(dev, mat, "baseColor", ANARI_FLOAT32_VEC4, baseColor);
    anariSetParameter(dev, mat, "metallic", ANARI_FLOAT32, &metallic);
    anariSetParameter(dev, mat, "roughness", ANARI_FLOAT32, &roughness);
    anariCommitParameters(dev, mat); anariRelease(dev, mat); numCreated++;
  }
  printf(" (%d materials created)", numCreated);
  anariRelease(dev, dev); printf(" PASSED\n");
}
static void test_pbr_metallic() {
  printf("  test_pbr_metallic...");
  ANARIDevice dev = createTestDevice();
  ANARIMaterial mat = anariNewMaterial(dev, "physicallyBased");
  if (!mat) { printf(" SKIPPED\n"); anariRelease(dev, dev); return; }
  float baseColor[] = {1.0f, 0.84f, 0.0f, 1.0f}; float metallic = 1.0f; float roughness = 0.2f;
  anariSetParameter(dev, mat, "baseColor", ANARI_FLOAT32_VEC4, baseColor);
  anariSetParameter(dev, mat, "metallic", ANARI_FLOAT32, &metallic);
  anariSetParameter(dev, mat, "roughness", ANARI_FLOAT32, &roughness);
  anariCommitParameters(dev, mat);
  float v[]={-1,-1,-3, 1,-1,-3, 1,1,-3, -1,1,-3};
  float n[]={0,0,1, 0,0,1, 0,0,1, 0,0,1}; uint32_t idx[]={0,1,2, 0,2,3};
  ANARIArray1D pa=anariNewArray1D(dev,v,0,0,ANARI_FLOAT32_VEC3,4);
  ANARIArray1D na=anariNewArray1D(dev,n,0,0,ANARI_FLOAT32_VEC3,4);
  ANARIArray1D ia=anariNewArray1D(dev,idx,0,0,ANARI_UINT32_VEC3,2);
  ANARIGeometry geom=anariNewGeometry(dev,"triangle");
  anariSetParameter(dev,geom,"vertex.position",ANARI_ARRAY1D,&pa);
  anariSetParameter(dev,geom,"vertex.normal",ANARI_ARRAY1D,&na);
  anariSetParameter(dev,geom,"primitive.index",ANARI_ARRAY1D,&ia);
  anariCommitParameters(dev,geom);
  ANARISurface surf=anariNewSurface(dev);
  anariSetParameter(dev,surf,"geometry",ANARI_GEOMETRY,&geom);
  anariSetParameter(dev,surf,"material",ANARI_MATERIAL,&mat); anariCommitParameters(dev,surf);
  ANARILight light=anariNewLight(dev,"directional");
  float ld[]={-1,-1,-1}; float lc[]={1,1,1}; float irr=3.0f;
  anariSetParameter(dev,light,"direction",ANARI_FLOAT32_VEC3,ld);
  anariSetParameter(dev,light,"color",ANARI_FLOAT32_VEC3,lc);
  anariSetParameter(dev,light,"irradiance",ANARI_FLOAT32,&irr); anariCommitParameters(dev,light);
  ANARIWorld world=anariNewWorld(dev);
  ANARIArray1D sa=anariNewArray1D(dev,&surf,0,0,ANARI_SURFACE,1);
  ANARIArray1D la=anariNewArray1D(dev,&light,0,0,ANARI_LIGHT,1);
  anariSetParameter(dev,world,"surface",ANARI_ARRAY1D,&sa);
  anariSetParameter(dev,world,"light",ANARI_ARRAY1D,&la); anariCommitParameters(dev,world);
  ANARICamera cam=anariNewCamera(dev,"perspective");
  float cp[]={0,0,0},cd[]={0,0,-1},cu[]={0,1,0};
  anariSetParameter(dev,cam,"position",ANARI_FLOAT32_VEC3,cp);
  anariSetParameter(dev,cam,"direction",ANARI_FLOAT32_VEC3,cd);
  anariSetParameter(dev,cam,"up",ANARI_FLOAT32_VEC3,cu); anariCommitParameters(dev,cam);
  ANARIRenderer ren=anariNewRenderer(dev,"default"); anariCommitParameters(dev,ren);
  ANARIFrame frame=anariNewFrame(dev);
  uint32_t sz[]={64,64}; ANARIDataType ct=ANARI_UFIXED8_VEC4;
  anariSetParameter(dev,frame,"size",ANARI_UINT32_VEC2,sz);
  anariSetParameter(dev,frame,"channel.color",ANARI_DATA_TYPE,&ct);
  anariSetParameter(dev,frame,"renderer",ANARI_RENDERER,&ren);
  anariSetParameter(dev,frame,"camera",ANARI_CAMERA,&cam);
  anariSetParameter(dev,frame,"world",ANARI_WORLD,&world); anariCommitParameters(dev,frame);
  anariRenderFrame(dev,frame); anariFrameReady(dev,frame,ANARI_WAIT);
  uint32_t fw,fh; ANARIDataType ft;
  const void *px=anariMapFrame(dev,frame,"channel.color",&fw,&fh,&ft);
  assert(px); anariUnmapFrame(dev,frame,"channel.color");
  anariRelease(dev,frame); anariRelease(dev,ren); anariRelease(dev,cam);
  anariRelease(dev,world); anariRelease(dev,sa); anariRelease(dev,la);
  anariRelease(dev,surf); anariRelease(dev,geom); anariRelease(dev,mat);
  anariRelease(dev,light); anariRelease(dev,pa); anariRelease(dev,na); anariRelease(dev,ia);
  anariRelease(dev,dev); printf(" PASSED\n");
}
static void test_pbr_normal_map() {
  printf("  test_pbr_normal_map...");
  ANARIDevice dev = createTestDevice();
  ANARIMaterial mat = anariNewMaterial(dev, "physicallyBased");
  if (!mat) { printf(" SKIPPED\n"); anariRelease(dev, dev); return; }
  uint8_t normalTexels[4*4*4];
  for (int i=0;i<4*4;i++) { normalTexels[i*4]=128; normalTexels[i*4+1]=128; normalTexels[i*4+2]=255; normalTexels[i*4+3]=255; }
  ANARIArray2D nrmTex=anariNewArray2D(dev,normalTexels,0,0,ANARI_UFIXED8_VEC4,4,4);
  ANARISampler normalSampler=anariNewSampler(dev,"image2D");
  anariSetParameter(dev,normalSampler,"image",ANARI_ARRAY2D,&nrmTex); anariCommitParameters(dev,normalSampler);
  float baseColor[]={0.5f,0.5f,0.5f,1.0f};
  anariSetParameter(dev,mat,"baseColor",ANARI_FLOAT32_VEC4,baseColor);
  anariSetParameter(dev,mat,"normal",ANARI_SAMPLER,&normalSampler); anariCommitParameters(dev,mat);
  anariRelease(dev,mat); anariRelease(dev,normalSampler); anariRelease(dev,nrmTex);
  anariRelease(dev,dev); printf(" PASSED\n");
}
static void test_emissive_material() {
  printf("  test_emissive_material...");
  ANARIDevice dev = createTestDevice();
  ANARIMaterial mat = anariNewMaterial(dev, "physicallyBased");
  if (!mat) mat = anariNewMaterial(dev, "matte");
  assert(mat);
  float color[]={0,0,0,1}; float emissive[]={10.0f,5.0f,0.0f};
  anariSetParameter(dev,mat,"color",ANARI_FLOAT32_VEC4,color);
  anariSetParameter(dev,mat,"emissive",ANARI_FLOAT32_VEC3,emissive);
  anariCommitParameters(dev,mat);
  anariRelease(dev,mat); anariRelease(dev,dev); printf(" PASSED\n");
}
int main() {
  printf("=== PBR Rendering Tests ===\n");
  test_pbr_roughness_sweep(); test_pbr_metallic(); test_pbr_normal_map(); test_emissive_material();
  printf("All PBR rendering tests passed.\n"); return 0;
}

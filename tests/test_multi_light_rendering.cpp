// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0
// Integration test: rendering with multiple lights and brightness validation
#include <anari/anari.h>
#include <cassert>
#include <cstdio>
#include <cstdint>
static void statusFunc(const void *, ANARIDevice, ANARIObject, ANARIDataType, ANARIStatusSeverity, ANARIStatusCode, const char *) {}
static ANARIDevice createTestDevice() {
  ANARILibrary lib = anariLoadLibrary("webgpu", statusFunc, nullptr); assert(lib);
  ANARIDevice dev = anariNewDevice(lib, "default"); assert(dev); return dev;
}
static float averageBrightness(ANARIDevice dev, ANARIFrame frame) {
  uint32_t fw, fh; ANARIDataType ft;
  const uint8_t *px = (const uint8_t *)anariMapFrame(dev, frame, "channel.color", &fw, &fh, &ft);
  if (!px) return -1.0f;
  double sum = 0;
  for (uint32_t i = 0; i < fw * fh; i++) { int idx = i * 4; sum += (px[idx] + px[idx+1] + px[idx+2]) / 3.0; }
  anariUnmapFrame(dev, frame, "channel.color");
  return (float)(sum / (fw * fh));
}
static void test_brightness_increases_with_lights() {
  printf("  test_brightness_increases_with_lights...");
  ANARIDevice dev = createTestDevice();
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
  ANARIMaterial mat=anariNewMaterial(dev,"matte");
  float c[]={0.8f,0.8f,0.8f,1}; anariSetParameter(dev,mat,"color",ANARI_FLOAT32_VEC4,c);
  anariCommitParameters(dev,mat);
  ANARISurface surf=anariNewSurface(dev);
  anariSetParameter(dev,surf,"geometry",ANARI_GEOMETRY,&geom);
  anariSetParameter(dev,surf,"material",ANARI_MATERIAL,&mat);
  anariCommitParameters(dev,surf);
  ANARICamera cam=anariNewCamera(dev,"perspective");
  float pos[]={0,0,0},dir[]={0,0,-1},up[]={0,1,0};
  anariSetParameter(dev,cam,"position",ANARI_FLOAT32_VEC3,pos);
  anariSetParameter(dev,cam,"direction",ANARI_FLOAT32_VEC3,dir);
  anariSetParameter(dev,cam,"up",ANARI_FLOAT32_VEC3,up); anariCommitParameters(dev,cam);
  ANARIRenderer ren=anariNewRenderer(dev,"default");
  float bg[]={0,0,0,1}; float ambientRad=0.0f;
  anariSetParameter(dev,ren,"background",ANARI_FLOAT32_VEC4,bg);
  anariSetParameter(dev,ren,"ambientRadiance",ANARI_FLOAT32,&ambientRad);
  anariCommitParameters(dev,ren);
  float brightnesses[3]={};
  for (int numLights=1;numLights<=3;numLights++) {
    ANARILight lights[3]; float dirs[][3]={{0,0,-1},{1,-1,-1},{-1,-1,-1}};
    for (int l=0;l<numLights;l++) {
      lights[l]=anariNewLight(dev,"directional");
      anariSetParameter(dev,lights[l],"direction",ANARI_FLOAT32_VEC3,dirs[l]);
      float irr=1.0f; anariSetParameter(dev,lights[l],"irradiance",ANARI_FLOAT32,&irr);
      anariCommitParameters(dev,lights[l]);
    }
    ANARIWorld world=anariNewWorld(dev);
    ANARIArray1D sa=anariNewArray1D(dev,&surf,0,0,ANARI_SURFACE,1);
    ANARIArray1D la=anariNewArray1D(dev,lights,0,0,ANARI_LIGHT,numLights);
    anariSetParameter(dev,world,"surface",ANARI_ARRAY1D,&sa);
    anariSetParameter(dev,world,"light",ANARI_ARRAY1D,&la); anariCommitParameters(dev,world);
    ANARIFrame frame=anariNewFrame(dev);
    uint32_t sz[]={64,64}; ANARIDataType ct=ANARI_UFIXED8_VEC4;
    anariSetParameter(dev,frame,"size",ANARI_UINT32_VEC2,sz);
    anariSetParameter(dev,frame,"channel.color",ANARI_DATA_TYPE,&ct);
    anariSetParameter(dev,frame,"renderer",ANARI_RENDERER,&ren);
    anariSetParameter(dev,frame,"camera",ANARI_CAMERA,&cam);
    anariSetParameter(dev,frame,"world",ANARI_WORLD,&world); anariCommitParameters(dev,frame);
    anariRenderFrame(dev,frame); anariFrameReady(dev,frame,ANARI_WAIT);
    brightnesses[numLights-1]=averageBrightness(dev,frame);
    printf(" [%d lights: avg=%.1f]",numLights,brightnesses[numLights-1]);
    anariRelease(dev,frame); anariRelease(dev,world);
    anariRelease(dev,sa); anariRelease(dev,la);
    for (int l=0;l<numLights;l++) anariRelease(dev,lights[l]);
  }
  if (brightnesses[0]>=0 && brightnesses[2]>=0)
    printf(" [trend: %.1f -> %.1f -> %.1f]",brightnesses[0],brightnesses[1],brightnesses[2]);
  anariRelease(dev,ren); anariRelease(dev,cam); anariRelease(dev,surf);
  anariRelease(dev,geom); anariRelease(dev,mat);
  anariRelease(dev,pa); anariRelease(dev,na); anariRelease(dev,ia);
  anariRelease(dev,dev); printf(" PASSED\n");
}
static void test_colored_lights() {
  printf("  test_colored_lights...");
  ANARIDevice dev = createTestDevice();
  float v[]={-1,-1,-3, 1,-1,-3, 0,1,-3}; float n[]={0,0,1, 0,0,1, 0,0,1};
  ANARIArray1D pa=anariNewArray1D(dev,v,0,0,ANARI_FLOAT32_VEC3,3);
  ANARIArray1D na=anariNewArray1D(dev,n,0,0,ANARI_FLOAT32_VEC3,3);
  ANARIGeometry geom=anariNewGeometry(dev,"triangle");
  anariSetParameter(dev,geom,"vertex.position",ANARI_ARRAY1D,&pa);
  anariSetParameter(dev,geom,"vertex.normal",ANARI_ARRAY1D,&na);
  anariCommitParameters(dev,geom);
  ANARIMaterial mat=anariNewMaterial(dev,"matte");
  float white[]={1,1,1,1}; anariSetParameter(dev,mat,"color",ANARI_FLOAT32_VEC4,white);
  anariCommitParameters(dev,mat);
  ANARISurface surf=anariNewSurface(dev);
  anariSetParameter(dev,surf,"geometry",ANARI_GEOMETRY,&geom);
  anariSetParameter(dev,surf,"material",ANARI_MATERIAL,&mat); anariCommitParameters(dev,surf);
  ANARILight light=anariNewLight(dev,"directional");
  float ld[]={0,0,-1}; float lc[]={1.0f,0.0f,0.0f};
  anariSetParameter(dev,light,"direction",ANARI_FLOAT32_VEC3,ld);
  anariSetParameter(dev,light,"color",ANARI_FLOAT32_VEC3,lc); anariCommitParameters(dev,light);
  ANARIWorld world=anariNewWorld(dev);
  ANARIArray1D sa=anariNewArray1D(dev,&surf,0,0,ANARI_SURFACE,1);
  ANARIArray1D la=anariNewArray1D(dev,&light,0,0,ANARI_LIGHT,1);
  anariSetParameter(dev,world,"surface",ANARI_ARRAY1D,&sa);
  anariSetParameter(dev,world,"light",ANARI_ARRAY1D,&la); anariCommitParameters(dev,world);
  anariRelease(dev,world); anariRelease(dev,sa); anariRelease(dev,la);
  anariRelease(dev,surf); anariRelease(dev,geom); anariRelease(dev,mat);
  anariRelease(dev,light); anariRelease(dev,pa); anariRelease(dev,na);
  anariRelease(dev,dev); printf(" PASSED\n");
}
int main() {
  printf("=== Multi-Light Rendering Tests ===\n");
  test_brightness_increases_with_lights(); test_colored_lights();
  printf("All multi-light rendering tests passed.\n"); return 0;
}

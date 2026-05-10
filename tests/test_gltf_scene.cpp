// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0
// Integration test: glTF-stle scene setup with instancing and multi-material
#include <anari/anari.h>
#include <cassert>
#include <cmath>
#include <cstdio>
static void statusFunc(const void *, ANARIDevice, ANARIObject, ANARIDataType, ANARIStatusSeverity, ANARIStatusCode, const char *) {}
static ANARIDevice createTestDevice() {
  ANARILibrary lib = anariLoadLibrary("webgpu", statusFunc, nullptr); assert(lib);
  ANARIDevice dev = anariNewDevice(lib, "default"); assert(dev); return dev;
}
static ANARISurface createTriangleSurface(ANARIDevice dev, float r, float g, float b) {
  float v[]={-0.3f,-0.3f,0, 0.3f,-0.3f,0, 0,0.3f,0};
  float n[]={0,0,1, 0,0,1, 0,0,1};
  ANARIArray1D pa=anariNewArray1D(dev,v,0,0,ANARI_FLOAT32_VEC3,3);
  ANARIArray1D na=anariNewArray1D(dev,n,0,0,ANARI_FLOAT32_VEC3,3);
  ANARIGeometry geom=anariNewGeometry(dev,"triangle");
  anariSetParameter(dev,geom,"vertex.position",ANARI_ARRAY1D,&pa);
  anariSetParameter(dev,geom,"vertex.normal",ANARI_ARRAY1D,&na);
  anariCommitParameters(dev,geom);
  ANARIMaterial mat=anariNewMaterial(dev,"matte");
  float color[]={r,g,b,1.0f}; anariSetParameter(dev,mat,"color",ANARI_FLOAT32_VEC4,color);
  anariCommitParameters(dev,mat);
  ANARISurface surf=anariNewSurface(dev);
  anariSetParameter(dev,surf,"geometry",ANARI_GEOMETRY,&geom);
  anariSetParameter(dev,surf,"material",ANARI_MATERIAL,&mat);
  anariCommitParameters(dev,surf);
  anariRelease(dev,pa); anariRelease(dev,na); anariRelease(dev,geom); anariRelease(dev,mat);
  return surf;
}
static void test_instanced_geometry() {
  printf("  test_instanced_geometry...");
  ANARIDevice dev = createTestDevice();
  ANARISurface surf = createTriangleSurface(dev, 0.8f, 0.2f, 0.2f);
  ANARIGroup group = anariNewGroup(dev);
  ANARIArray1D surfArr = anariNewArray1D(dev, &surf, 0, 0, ANARI_SURFACE, 1);
  anariSetParameter(dev, group, "surface", ANARI_ARRAY1D, &surfArr);
  anariCommitParameters(dev, group);
  const int numInst = 4; ANARIInstance instances[4];
  for (int i = 0; i < numInst; i++) {
    instances[i] = anariNewInstance(dev, "transform");
    anariSetParameter(dev, instances[i], "group", ANARI_GROUP, &group);
    float xform[] = {1,0,0,0, 0,1,0,0, 0,0,1,0, (float)(i-1.5)*0.8f,0,-3,1};
    anariSetParameter(dev, instances[i], "transform", ANARI_FLOAT32_MAT4, xform);
    anariCommitParameters(dev, instances[i]);
  }
  ANARIWorld world = anariNewWorld(dev);
  ANARIArray1D instArr = anariNewArray1D(dev, instances, 0, 0, ANARI_INSTANCE, numInst);
  anariSetParameter(dev, world, "instance", ANARI_ARRAY1D, &instArr);
  anariCommitParameters(dev, world);
  ANARICamera cam=anariNewCamera(dev,"perspective");
  float pos[]={0,0,0},dir[]={0,0,-1},up[]={0,1,0};
  anariSetParameter(dev,cam,"position",ANARI_FLOAT32_VEC3,pos);
  anariSetParameter(dev,cam,"direction",ANARI_FLOAT32_VEC3,dir);
  anariSetParameter(dev,cam,"up",ANARI_FLOAT32_VEC3,up); anariCommitParameters(dev,cam);
  ANARIRenderer ren=anariNewRenderer(dev,"default"); anariCommitParameters(dev,ren);
  ANARIFrame frame=anariNewFrame(dev);
  uint32_t sz[]={128,64}; ANARIDataType ct=ANARI_UFIXED8_VEC4;
  anariSetParameter(dev,frame,"size",ANARI_UINT32_VEC2,sz);
  anariSetParameter(dev,frame,"channel.color",ANARI_DATA_TYPE,&ct);
  anariSetParameter(dev,frame,"renderer",ANARI_RENDERER,&ren);
  anariSetParameter(dev,frame,"camera",ANARI_CAMERA,&cam);
  anariSetParameter(dev,frame,"world",ANARI_WORLD,&world); anariCommitParameters(dev,frame);
  anariRenderFrame(dev,frame); anariFrameReady(dev,frame,ANARI_WAIT);
  uint32_t fw,fh; ANARIDataType ft;
  const void *px=anariMapFrame(dev,frame,"channel.color",&fw,&fh,&ft);
  assert(px && "Instanced scene should render"); anariUnmapFrame(dev,frame,"channel.color");
  anariRelease(dev,frame); anariRelease(dev,ren); anariRelease(dev,cam);
  anariRelease(dev,world); anariRelease(dev,instArr);
  for (int i=0;i<numInst;i++) anariRelease(dev,instances[i]);
  anariRelease(dev,group); anariRelease(dev,surfArr); anariRelease(dev,surf);
  anariRelease(dev,dev); printf(" PASSED\n");
}
static void test_multi_material_scene() {
  printf("  test_multi_material_scene...");
  ANARIDevice dev = createTestDevice();
  ANARISurface surfaces[3];
  surfaces[0]=createTriangleSurface(dev,1,0,0);
  surfaces[1]=createTriangleSurface(dev,0,1,0);
  surfaces[2]=createTriangleSurface(dev,0,0,1);
  ANARIGroup group=anariNewGroup(dev);
  ANARIArray1D sa=anariNewArray1D(dev,surfaces,0,0,ANARI_SURFACE,3);
  anariSetParameter(dev,group,"surface",ANARI_ARRAY1D,&sa); anariCommitParameters(dev,group);
  ANARIInstance inst=anariNewInstance(dev,"transform");
  anariSetParameter(dev,inst,"group",ANARI_GROUP,&group);
  float xform[]={1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,-3,1};
  anariSetParameter(dev,inst,"transform",ANARI_FLOAT32_MAT4,xform);
  anariCommitParameters(dev,inst);
  ANARIWorld world=anariNewWorld(dev);
  ANARIArray1D ia=anariNewArray1D(dev,&inst,0,0,ANARI_INSTANCE,1);
  anariSetParameter(dev,world,"instance",ANARI_ARRAY1D,&ia);
  ANARILight light=anariNewLight(dev,"directional");
  float ld[]={0,-1,-1}; anariSetParameter(dev,light,"direction",ANARI_FLOAT32_VEC3,ld);
  anariCommitParameters(dev,light);
  ANARIArray1D la=anariNewArray1D(dev,&light,0,0,ANARI_LIGHT,1);
  anariSetParameter(dev,world,"light",ANARI_ARRAY1D,&la); anariCommitParameters(dev,world);
  anariRelease(dev,world); anariRelease(dev,ia); anariRelease(dev,la);
  anariRelease(dev,inst); anariRelease(dev,group); anariRelease(dev,sa);
  anariRelease(dev,light);
  for (int i=0;i<3;i++) anariRelease(dev,surfaces[i]);
  anariRelease(dev,dev); printf(" PASSED\n");
}
static void test_transform_hierarchy() {
  printf("  test_transform_hierarchy...");
  ANARIDevice dev = createTestDevice();
  ANARISurface surf = createTriangleSurface(dev, 1, 1, 1);
  ANARIGroup g1=anariNewGroup(dev);
  ANARIArray1D sa=anariNewArray1D(dev,&surf,0,0,ANARI_SURFACE,1);
  anariSetParameter(dev,g1,"surface",ANARI_ARRAY1D,&sa); anariCommitParameters(dev,g1);
  ANARIInstance inst1=anariNewInstance(dev,"transform");
  anariSetParameter(dev,inst1,"group",ANARI_GROUP,&g1);
  float c=cosf(0.785f),s=sinf(0.785f);
  float rot[]={c,-s,0,0, s,c,0,0, 0,0,1,0, 0,0,-3,1};
  anariSetParameter(dev,inst1,"transform",ANARI_FLOAT32_MAT4,rot);
  anariCommitParameters(dev,inst1);
  ANARIInstance inst2=anariNewInstance(dev,"transform");
  anariSetParameter(dev,inst2,"group",ANARI_GROUP,&g1);
  float scale[]={2,0,0,0, 0,2,0,0, 0,0,2,0, 1,0,-3,1};
  anariSetParameter(dev,inst2,"transform",ANARI_FLOAT32_MAT4,scale);
  anariCommitParameters(dev,inst2);
  ANARIWorld world=anariNewWorld(dev);
  ANARIInstance insts[]={inst1,inst2};
  ANARIArray1D ia=anariNewArray1D(dev,insts,0,0,ANARI_INSTANCE,2);
  anariSetParameter(dev,world,"instance",ANARI_ARRAY1D,&ia); anariCommitParameters(dev,world);
  anariRelease(dev,world); anariRelease(dev,ia);
  anariRelease(dev,inst1); anariRelease(dev,inst2);
  anariRelease(dev,g1); anariRelease(dev,sa); anariRelease(dev,surf);
  anariRelease(dev,dev); printf(" PASSED\n");
}
int main() {
  printf("=== glTF-style Scene Tests ===\n");
  test_instanced_geometry(); test_multi_material_scene(); test_transform_hierarchy();
  printf("All glTF scene tests passed.\n"); return 0;
}

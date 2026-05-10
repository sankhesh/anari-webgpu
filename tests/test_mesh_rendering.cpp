// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0
// Integration test: polygonal meshes (quad, cube, indexed mesh)
#include <anari/anari.h>
#include <cassert>
#include <cstdio>
#include <cstring>
static void statusFunc(const void *, ANARIDevice, ANARIObject, ANARIDataType, ANARIStatusSeverity, ANARIStatusCode, const char *) {}
static ANARIDevice createTestDevice() {
  ANARILibrary lib = anariLoadLibrary("webgpu", statusFunc, nullptr); assert(lib);
  ANARIDevice dev = anariNewDevice(lib, "default"); assert(dev); return dev;
}
static bool renderAndValidate(ANARIDevice dev, ANARIWorld world, int w, int h) {
  ANARICamera cam = anariNewCamera(dev, "perspective");
  float pos[]={0,0,3}, dir[]={0,0,-1}, up[]={0,1,0}; float fovy=1.0472f, aspect=(float)w/h;
  anariSetParameter(dev, cam, "position", ANARI_FLOAT32_VEC3, pos);
  anariSetParameter(dev, cam, "direction", ANARI_FLOAT32_VEC3, dir);
  anariSetParameter(dev, cam, "up", ANARI_FLOAT32_VEC3, up);
  anariSetParameter(dev, cam, "fovy", ANARI_FLOAT32, &fovy);
  anariSetParameter(dev, cam, "aspect", ANARI_FLOAT32, &aspect);
  anariCommitParameters(dev, cam);
  ANARIRenderer ren = anariNewRenderer(dev, "default");
  float bg[]={0,0,0,1}; anariSetParameter(dev, ren, "background", ANARI_FLOAT32_VEC4, bg);
  anariCommitParameters(dev, ren);
  ANARIFrame frame = anariNewFrame(dev);
  uint32_t sz[]={(uint32_t)w,(uint32_t)h}; ANARIDataType ct=ANARI_UFIXED8_VEC4;
  anariSetParameter(dev, frame, "size", ANARI_UINT32_VEC2, sz);
  anariSetParameter(dev, frame, "channel.color", ANARI_DATA_TYPE, &ct);
  anariSetParameter(dev, frame, "renderer", ANARI_RENDERER, &ren);
  anariSetParameter(dev, frame, "camera", ANARI_CAMERA, &cam);
  anariSetParameter(dev, frame, "world", ANARI_WORLD, &world);
  anariCommitParameters(dev, frame);
  anariRenderFrame(dev, frame); anariFrameReady(dev, frame, ANARI_WAIT);
  uint32_t fbW, fbH; ANARIDataType fbType;
  const uint8_t *px = (const uint8_t *)anariMapFrame(dev, frame, "channel.color", &fbW, &fbH, &fbType);
  bool valid = (px != nullptr) && (fbW == (uint32_t)w) && (fbH == (uint32_t)h);
  if (px) anariUnmapFrame(dev, frame, "channel.color");
  anariRelease(dev, frame); anariRelease(dev, ren); anariRelease(dev, cam);
  return valid;
}
static void test_quad_mesh() {
  printf("  test_quad_mesh..."); ANARIDevice dev = createTestDevice();
  float verts[]={-0.5f,-0.5f,-2, 0.5f,-0.5f,-2, 0.5f,0.5f,-2, -0.5f,0.5f,-2};
  uint32_t indices[]={0,1,2, 0,2,3}; float normals[]={0,0,1, 0,0,1, 0,0,1, 0,0,1};
  ANARIArray1D pa=anariNewArray1D(dev,verts,0,0,ANARI_FLOAT32_VEC3,4);
  ANARIArray1D ia=anariNewArray1D(dev,indices,0,0,ANARI_UINT32_VEC3,2);
  ANARIArray1D na=anariNewArray1D(dev,normals,0,0,ANARI_FLOAT32_VEC3,4);
  ANARIGeometry geom=anariNewGeometry(dev,"triangle");
  anariSetParameter(dev,geom,"vertex.position",ANARI_ARRAY1D,&pa);
  anariSetParameter(dev,geom,"primitive.index",ANARI_ARRAY1D,&ia);
  anariSetParameter(dev,geom,"vertex.normal",ANARI_ARRAY1D,&na);
  anariCommitParameters(dev,geom);
  ANARIMaterial mat=anariNewMaterial(dev,"matte");
  float c[]={0.2f,0.6f,1.0f,1.0f}; anariSetParameter(dev,mat,"color",ANARI_FLOAT32_VEC4,c);
  anariCommitParameters(dev,mat);
  ANARISurface surf=anariNewSurface(dev);
  anariSetParameter(dev,surf,"geometry",ANARI_GEOMETRY,&geom);
  anariSetParameter(dev,surf,"material",ANARI_MATERIAL,&mat);
  anariCommitParameters(dev,surf);
  ANARIWorld world=anariNewWorld(dev);
  ANARIArray1D sa=anariNewArray1D(dev,&surf,0,0,ANARI_SURFACE,1);
  anariSetParameter(dev,world,"surface",ANARI_ARRAY1D,&sa);
  anariCommitParameters(dev,world);
  assert(renderAndValidate(dev,world,64,64));
  anariRelease(dev,world); anariRelease(dev,sa); anariRelease(dev,surf);
  anariRelease(dev,geom); anariRelease(dev,mat);
  anariRelease(dev,pa); anariRelease(dev,ia); anariRelease(dev,na);
  anariRelease(dev,dev); printf(" PASSED\n");
}
static void test_cube_mesh() {
  printf("  test_cube_mesh..."); ANARIDevice dev = createTestDevice();
  float verts[]={-0.5f,-0.5f,-0.5f, 0.5f,-0.5f,-0.5f, 0.5f,0.5f,-0.5f, -0.5f,0.5f,-0.5f,
    -0.5f,-0.5f,0.5f, 0.5f,-0.5f,0.5f, 0.5f,0.5f,0.5f, -0.5f,0.5f,0.5f};
  uint32_t idx[]={0,1,2,0,2,3, 4,6,5,4,7,6, 0,4,5,0,5,1, 2,6,7,2,7,3, 0,3,7,0,7,4, 1,5,6,1,6,2};
  ANARIArray1D pa=anariNewArray1D(dev,verts,0,0,ANARI_FLOAT32_VEC3,8);
  ANARIArray1D ia=anariNewArray1D(dev,idx,0,0,ANARI_UINT32_VEC3,12);
  ANARIGeometry geom=anariNewGeometry(dev,"triangle");
  anariSetParameter(dev,geom,"vertex.position",ANARI_ARRAY1D,&pa);
  anariSetParameter(dev,geom,"primitive.index",ANARI_ARRAY1D,&ia);
  anariCommitParameters(dev,geom);
  ANARIMaterial mat=anariNewMaterial(dev,"matte");
  float c[]={0.8f,0.3f,0.1f,1}; anariSetParameter(dev,mat,"color",ANARI_FLOAT32_VEC4,c);
  anariCommitParameters(dev,mat);
  ANARISurface surf=anariNewSurface(dev);
  anariSetParameter(dev,surf,"geometry",ANARI_GEOMETRY,&geom);
  anariSetParameter(dev,surf,"material",ANARI_MATERIAL,&mat);
  anariCommitParameters(dev,surf);
  ANARILight light=anariNewLight(dev,"directional");
  float ld[]={-1,-1,-1}; anariSetParameter(dev,light,"direction",ANARI_FLOAT32_VEC3,ld);
  anariCommitParameters(dev,light);
  ANARIWorld world=anariNewWorld(dev);
  ANARIArray1D sa=anariNewArray1D(dev,&surf,0,0,ANARI_SURFACE,1);
  ANARIArray1D la=anariNewArray1D(dev,&light,0,0,ANARI_LIGHT,1);
  anariSetParameter(dev,world,"surface",ANARI_ARRAY1D,&sa);
  anariSetParameter(dev,world,"light",ANARI_ARRAY1D,&la);
  anariCommitParameters(dev,world);
  assert(renderAndValidate(dev,world,128,128));
  anariRelease(dev,world); anariRelease(dev,sa); anariRelease(dev,la);
  anariRelease(dev,surf); anariRelease(dev,geom); anariRelease(dev,mat);
  anariRelease(dev,light); anariRelease(dev,pa); anariRelease(dev,ia);
  anariRelease(dev,dev); printf(" PASSED\n");
}
static void test_vertex_colored_mesh() {
  printf("  test_vertex_colored_mesh..."); ANARIDevice dev = createTestDevice();
  float verts[]={-1,-1,-3, 1,-1,-3, 1,1,-3, -1,1,-3};
  float colors[]={1,0,0,1, 0,1,0,1, 0,0,1,1, 1,1,0,1};
  uint32_t idx[]={0,1,2, 0,2,3};
  ANARIArray1D pa=anariNewArray1D(dev,verts,0,0,ANARI_FLOAT32_VEC3,4);
  ANARIArray1D ca=anariNewArray1D(dev,colors,0,0,ANARI_FLOAT32_VEC4,4);
  ANARIArray1D ia=anariNewArray1D(dev,idx,0,0,ANARI_UINT32_VEC3,2);
  ANARIGeometry geom=anariNewGeometry(dev,"triangle");
  anariSetParameter(dev,geom,"vertex.position",ANARI_ARRAY1D,&pa);
  anariSetParameter(dev,geom,"vertex.color",ANARI_ARRAY1D,&ca);
  anariSetParameter(dev,geom,"primitive.index",ANARI_ARRAY1D,&ia);
  anariCommitParameters(dev,geom);
  ANARIMaterial mat=anariNewMaterial(dev,"matte"); anariCommitParameters(dev,mat);
  ANARISurface surf=anariNewSurface(dev);
  anariSetParameter(dev,surf,"geometry",ANARI_GEOMETRY,&geom);
  anariSetParameter(dev,surf,"material",ANARI_MATERIAL,&mat);
  anariCommitParameters(dev,surf);
  ANARIWorld world=anariNewWorld(dev);
  ANARIArray1D sa=anariNewArray1D(dev,&surf,0,0,ANARI_SURFACE,1);
  anariSetParameter(dev,world,"surface",ANARI_ARRAY1D,&sa);
  anariCommitParameters(dev,world);
  assert(renderAndValidate(dev,world,64,64));
  anariRelease(dev,world); anariRelease(dev,sa); anariRelease(dev,surf);
  anariRelease(dev,geom); anariRelease(dev,mat);
  anariRelease(dev,pa); anariRelease(dev,ca); anariRelease(dev,ia);
  anariRelease(dev,dev); printf(" PASSED\n");
}
int main() {
  printf("=== Polygonal Mesh Rendering Tests ===\n");
  test_quad_mesh(); test_cube_mesh(); test_vertex_colored_mesh();
  printf("All mesh rendering tests passed.\n"); return 0;
}

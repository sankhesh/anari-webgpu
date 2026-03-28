// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0
// Integration test: cubemap environment textures and 2D image samplers
#include <anari/anari.h>
#include <cassert>
#include <cstdio>
#include <cstdint>
static void statusFunc(const void *, ANARIDevice, ANARIObject, ANARIDataType, ANARIStatusSeverity, ANARIStatusCode, const char *) {}
static ANARIDevice createTestDevice() {
  ANARILibrary lib = anariLoadLibrary("webgpu", statusFunc, nullptr); assert(lib);
  ANARIDevice dev = anariNewDevice(lib, "default"); assert(dev); return dev;
}
static void test_image2d_sampler() {
  printf("  test_image2d_sampler...");
  ANARIDevice dev = createTestDevice();
  uint8_t texels[4*4*4];
  for (int i=0;i<4*4;i++) { texels[i*4]=(i*17)&0xFF; texels[i*4+1]=(i*37)&0xFF; texels[i*4+2]=(i*59)&0xFF; texels[i*4+3]=255; }
  ANARIArray2D tex = anariNewArray2D(dev, texels, 0, 0, ANARI_UFIXED8_VEC4, 4, 4);
  ANARISampler sampler = anariNewSampler(dev, "image2D"); assert(sampler);
  anariSetParameter(dev, sampler, "image", ANARI_ARRAY2D, &tex);
  anariCommitParameters(dev, sampler);
  anariRelease(dev, sampler); anariRelease(dev, tex); anariRelease(dev, dev);
  printf(" PASSED\n");
}
static void test_cubemap_faces() {
  printf("  test_cubemap_faces...");
  ANARIDevice dev = createTestDevice();
  const int faceSize = 8;
  uint8_t faceColors[][3] = {{255,0,0},{0,255,255},{0,255,0},{255,0,255},{0,0,255},{255,255,0}};
  ANARISampler samplers[6]={}; ANARIArray2D texArrays[6]={};
  for (int face=0;face<6;face++) {
    uint8_t *texels = new uint8_t[faceSize*faceSize*4];
    for (int i=0;i<faceSize*faceSize;i++) {
      texels[i*4]=faceColors[face][0]; texels[i*4+1]=faceColors[face][1];
      texels[i*4+2]=faceColors[face][2]; texels[i*4+3]=255;
    }
    texArrays[face]=anariNewArray2D(dev,texels,0,0,ANARI_UFIXED8_VEC4,faceSize,faceSize);
    samplers[face]=anariNewSampler(dev,"image2D");
    anariSetParameter(dev,samplers[face],"image",ANARI_ARRAY2D,&texArrays[face]);
    anariCommitParameters(dev,samplers[face]); delete[] texels;
  }
  printf(" (created 6 face samplers)");
  for (int i=0;i<6;i++) { anariRelease(dev,samplers[i]); anariRelease(dev,texArrays[i]); }
  anariRelease(dev, dev); printf(" PASSED\n");
}
static void test_textured_surface() {
  printf("  test_textured_surface...");
  ANARIDevice dev = createTestDevice();
  const int texW=16,texH=16;
  uint8_t *texels = new uint8_t[texW*texH*4];
  for (int y=0;y<texH;y++) for (int x=0;x<texW;x++) {
    int i=(y*texW+x)*4; texels[i]=(uint8_t)(255*x/(texW-1)); texels[i+1]=(uint8_t)(255*y/(texH-1)); texels[i+2]=128; texels[i+3]=255;
  }
  ANARIArray2D texArr=anariNewArray2D(dev,texels,0,0,ANARI_UFIXED8_VEC4,texW,texH);
  ANARISampler sampler=anariNewSampler(dev,"image2D");
  anariSetParameter(dev,sampler,"image",ANARI_ARRAY2D,&texArr); anariCommitParameters(dev,sampler);
  ANARIMaterial mat=anariNewMaterial(dev,"matte");
  anariSetParameter(dev,mat,"color",ANARI_SAMPLER,&sampler); anariCommitParameters(dev,mat);
  float verts[]={-1,-1,-3, 1,-1,-3, 1,1,-3, -1,1,-3};
  float texcoords[]={0,0, 1,0, 1,1, 0,1}; uint32_t idx[]={0,1,2, 0,2,3};
  ANARIArray1D pa=anariNewArray1D(dev,verts,0,0,ANARI_FLOAT32_VEC3,4);
  ANARIArray1D ta=anariNewArray1D(dev,texcoords,0,0,ANARI_FLOAT32_VEC2,4);
  ANARIArray1D ia=anariNewArray1D(dev,idx,0,0,ANARI_UINT32_VEC3,2);
  ANARIGeometry geom=anariNewGeometry(dev,"triangle");
  anariSetParameter(dev,geom,"vertex.position",ANARI_ARRAY1D,&pa);
  anariSetParameter(dev,geom,"vertex.attribute0",ANARI_ARRAY1D,&ta);
  anariSetParameter(dev,geom,"primitive.index",ANARI_ARRAY1D,&ia);
  anariCommitParameters(dev,geom);
  ANARISurface surf=anariNewSurface(dev);
  anariSetParameter(dev,surf,"geometry",ANARI_GEOMETRY,&geom);
  anariSetParameter(dev,surf,"material",ANARI_MATERIAL,&mat);
  anariCommitParameters(dev,surf);
  ANARIWorld world=anariNewWorld(dev);
  ANARIArray1D sa=anariNewArray1D(dev,&surf,0,0,ANARI_SURFACE,1);
  anariSetParameter(dev,world,"surface",ANARI_ARRAY1D,&sa); anariCommitParameters(dev,world);
  ANARICamera cam=anariNewCamera(dev,"perspective");
  float pos[]={0,0,0},dir[]={0,0,-1},up[]={0,1,0};
  anariSetParameter(dev,cam,"position",ANARI_FLOAT32_VEC3,pos);
  anariSetParameter(dev,cam,"direction",ANARI_FLOAT32_VEC3,dir);
  anariSetParameter(dev,cam,"up",ANARI_FLOAT32_VEC3,up); anariCommitParameters(dev,cam);
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
  anariRelease(dev,world); anariRelease(dev,sa); anariRelease(dev,surf);
  anariRelease(dev,geom); anariRelease(dev,mat); anariRelease(dev,sampler);
  anariRelease(dev,texArr); anariRelease(dev,pa); anariRelease(dev,ta);
  anariRelease(dev,ia); anariRelease(dev,dev); delete[] texels;
  printf(" PASSED\n");
}
int main() {
  printf("=== Cubemap & Texture Tests ===\n");
  test_image2d_sampler(); test_cubemap_faces(); test_textured_surface();
  printf("All cubemap/texture tests passed.\n"); return 0;
}

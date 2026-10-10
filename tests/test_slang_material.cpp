// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include <anari/anari.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

static std::string g_errors;

static void statusFunc(const void *, ANARIDevice, ANARIObject, ANARIDataType,
    ANARIStatusSeverity severity, ANARIStatusCode, const char *msg)
{
  if (severity <= ANARI_SEVERITY_ERROR)
    g_errors += msg;
  if (severity <= ANARI_SEVERITY_WARNING)
    fprintf(stderr, "[%s] %s\n", severity == ANARI_SEVERITY_ERROR ? "ERROR" : "WARN", msg);
}

static const char *kModule = R"(
module test_tint;
import vtkslang_shading;
import vtkslang_hooks;
public struct Tinted : IMaterial {
  float4 tint;
  float3x3 colorMatrix;
  public MaterialSample evaluate(SurfaceSample s) {
    MaterialSample m = HostDrivenMaterial().evaluate(s);
    m.baseColor = mul(colorMatrix, tint.rgb);
    return m;
  }
};
public struct Swapped : IMaterial {
  float4 tint;
  float2x2 swap;
  public MaterialSample evaluate(SurfaceSample s) {
    MaterialSample m = HostDrivenMaterial().evaluate(s);
    m.baseColor = float3(mul(swap, tint.rg), tint.b);
    return m;
  }
};
public struct Invert : ISurfaceOutput {
  public bool keep(SurfaceSample s) { return true; }
  public float4 finish(float4 c, MaterialSample m, SurfaceSample s) { return float4(1.f - c.rgb, c.a); }
  public float depth(float d, SurfaceSample s) { return d; }
};
public struct Erase : ISurfaceOutput {
  public bool keep(SurfaceSample s) { return false; }
  public float4 finish(float4 c, MaterialSample m, SurfaceSample s) { return c; }
  public float depth(float d, SurfaceSample s) { return d; }
};
public struct Paint : IShaderHooks {
  float3 tint;
  public override void output(inout FragmentContext f) { f.color = float4(tint, 1.f); }
};
public struct Halve : IShaderHooks {
  float amount;
  public override void lit(inout FragmentContext f) { f.color.rgb *= amount; }
};
public struct Shift : IVertexModifier {
  float offset;
  public VertexSample modify(VertexSample v) {
    v.position.x += offset;
    return v;
  }
};
)";

static constexpr int kSize = 64;

struct Scene
{
  ANARIDevice device;
  ANARIMaterial material;
  ANARIFrame frame;
  ANARIWorld world;
  ANARICamera camera;
  ANARIRenderer renderer;
};

static Scene createScene()
{
  Scene s{};
  ANARILibrary lib = anariLoadLibrary("webgpu", statusFunc, nullptr);
  s.device = anariNewDevice(lib, "default");
  float vertices[] = {-1.f, -1.f, -2.f, 1.f, -1.f, -2.f, 0.f, 1.f, -2.f};
  float normals[] = {0, 0, 1, 0, 0, 1, 0, 0, 1};
  ANARIArray1D posArr = anariNewArray1D(s.device, vertices, 0, 0, ANARI_FLOAT32_VEC3, 3);
  ANARIArray1D normArr = anariNewArray1D(s.device, normals, 0, 0, ANARI_FLOAT32_VEC3, 3);
  ANARIGeometry geom = anariNewGeometry(s.device, "triangle");
  anariSetParameter(s.device, geom, "vertex.position", ANARI_ARRAY1D, &posArr);
  anariSetParameter(s.device, geom, "vertex.normal", ANARI_ARRAY1D, &normArr);
  anariCommitParameters(s.device, geom);
  s.material = anariNewMaterial(s.device, "matte");
  float white[] = {1, 1, 1, 1};
  anariSetParameter(s.device, s.material, "color", ANARI_FLOAT32_VEC4, white);
  anariCommitParameters(s.device, s.material);
  ANARISurface surf = anariNewSurface(s.device);
  anariSetParameter(s.device, surf, "geometry", ANARI_GEOMETRY, &geom);
  anariSetParameter(s.device, surf, "material", ANARI_MATERIAL, &s.material);
  anariCommitParameters(s.device, surf);
  ANARILight light = anariNewLight(s.device, "directional");
  float lightDir[] = {0, 0, -1};
  anariSetParameter(s.device, light, "direction", ANARI_FLOAT32_VEC3, lightDir);
  anariCommitParameters(s.device, light);
  s.world = anariNewWorld(s.device);
  ANARIArray1D surfArr = anariNewArray1D(s.device, &surf, 0, 0, ANARI_SURFACE, 1);
  ANARIArray1D lightArr = anariNewArray1D(s.device, &light, 0, 0, ANARI_LIGHT, 1);
  anariSetParameter(s.device, s.world, "surface", ANARI_ARRAY1D, &surfArr);
  anariSetParameter(s.device, s.world, "light", ANARI_ARRAY1D, &lightArr);
  anariCommitParameters(s.device, s.world);
  s.camera = anariNewCamera(s.device, "perspective");
  float camPos[] = {0, 0, 0}, camDir[] = {0, 0, -1}, camUp[] = {0, 1, 0};
  float fovy = 1.0472f, aspect = 1.f;
  anariSetParameter(s.device, s.camera, "position", ANARI_FLOAT32_VEC3, camPos);
  anariSetParameter(s.device, s.camera, "direction", ANARI_FLOAT32_VEC3, camDir);
  anariSetParameter(s.device, s.camera, "up", ANARI_FLOAT32_VEC3, camUp);
  anariSetParameter(s.device, s.camera, "fovy", ANARI_FLOAT32, &fovy);
  anariSetParameter(s.device, s.camera, "aspect", ANARI_FLOAT32, &aspect);
  anariCommitParameters(s.device, s.camera);
  s.renderer = anariNewRenderer(s.device, "default");
  float bg[] = {0, 0, 0, 1};
  anariSetParameter(s.device, s.renderer, "background", ANARI_FLOAT32_VEC4, bg);
  anariCommitParameters(s.device, s.renderer);
  s.frame = anariNewFrame(s.device);
  uint32_t size[] = {kSize, kSize};
  ANARIDataType colorType = ANARI_UFIXED8_VEC4;
  anariSetParameter(s.device, s.frame, "size", ANARI_UINT32_VEC2, size);
  anariSetParameter(s.device, s.frame, "channel.color", ANARI_DATA_TYPE, &colorType);
  anariSetParameter(s.device, s.frame, "renderer", ANARI_RENDERER, &s.renderer);
  anariSetParameter(s.device, s.frame, "camera", ANARI_CAMERA, &s.camera);
  anariSetParameter(s.device, s.frame, "world", ANARI_WORLD, &s.world);
  anariCommitParameters(s.device, s.frame);
  anariRelease(s.device, surfArr);
  anariRelease(s.device, lightArr);
  anariRelease(s.device, surf);
  anariRelease(s.device, geom);
  anariRelease(s.device, light);
  anariRelease(s.device, posArr);
  anariRelease(s.device, normArr);
  return s;
}

static void render(Scene &s, int rgb[3])
{
  anariRenderFrame(s.device, s.frame);
  anariFrameReady(s.device, s.frame, ANARI_WAIT);
  uint32_t w, h;
  ANARIDataType type;
  const auto *px = static_cast<const uint8_t *>(
      anariMapFrame(s.device, s.frame, "channel.color", &w, &h, &type));
  const int i = ((h / 2) * w + w / 2) * 4;
  for (int c = 0; c < 3; ++c)
    rgb[c] = px[i + c];
  anariUnmapFrame(s.device, s.frame, "channel.color");
}

static int dominant(const int rgb[3])
{
  for (int c = 0; c < 3; ++c)
    if (rgb[c] > 60 && rgb[c] > 3 * rgb[(c + 1) % 3] && rgb[c] > 3 * rgb[(c + 2) % 3])
      return c;
  return -1;
}

static bool same(const int a[3], const int b[3])
{
  for (int c = 0; c < 3; ++c)
    if (std::abs(a[c] - b[c]) > 1)
      return false;
  return true;
}

static int fail(const char *what, const int rgb[3])
{
  fprintf(stderr, "FAILED: %s (center pixel %d, %d, %d)\n", what, rgb[0], rgb[1], rgb[2]);
  return 1;
}

int main()
{
  printf("=== Slang Material Override Tests ===\n");
  Scene s = createScene();
  int defaults[3], rgb[3];
  render(s, defaults);
  void *gpu = nullptr;
  anariGetProperty(s.device, s.device, "webgpu.device", ANARI_VOID_POINTER, &gpu,
      sizeof(gpu), ANARI_WAIT);
  if (!gpu) {
    printf("No WebGPU device; skipping.\n");
    return 77;
  }

  const char *materialType = "Tinted";
  float green[] = {0, 1, 0, 1};
  float identity[] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
  anariSetParameter(s.device, s.material, "slang.source", ANARI_STRING, kModule);
  anariSetParameter(s.device, s.material, "slang.material", ANARI_STRING, materialType);
  anariSetParameter(s.device, s.material, "slang.material.tint", ANARI_FLOAT32_VEC4, green);
  anariSetParameter(s.device, s.material, "slang.material.colorMatrix", ANARI_FLOAT32_MAT3, identity);
  anariCommitParameters(s.device, s.material);
  render(s, rgb);
  if (g_errors.find("Slang is disabled") != std::string::npos
      || g_errors.find("built without Slang") != std::string::npos) {
    printf("Slang is unavailable; skipping.\n");
    return 77;
  }
  printf("  tint...");
  if (dominant(rgb) != 1)
    return fail("the tint parameter was not applied (expected green)", rgb);
  printf(" PASSED\n");

  printf("  value update (mat3)...");
  float swapGB[] = {1, 0, 0, 0, 0, 1, 0, 1, 0};
  anariSetParameter(s.device, s.material, "slang.material.colorMatrix", ANARI_FLOAT32_MAT3, swapGB);
  anariCommitParameters(s.device, s.material);
  render(s, rgb);
  if (dominant(rgb) != 2)
    return fail("the colorMatrix parameter was not applied as columns (expected blue)", rgb);
  printf(" PASSED\n");

  printf("  value update (mat2)...");
  float swapRG[] = {0, 1, 1, 0};
  anariSetParameter(s.device, s.material, "slang.material", ANARI_STRING, "Swapped");
  anariSetParameter(s.device, s.material, "slang.material.swap", ANARI_FLOAT32_MAT2, swapRG);
  anariCommitParameters(s.device, s.material);
  render(s, rgb);
  if (dominant(rgb) != 0)
    return fail("the float2x2 parameter was not packed as 16-byte columns (expected red)", rgb);
  anariUnsetParameter(s.device, s.material, "slang.material.swap");
  anariSetParameter(s.device, s.material, "slang.material", ANARI_STRING, materialType);
  anariCommitParameters(s.device, s.material);
  printf(" PASSED\n");

  printf("  surface output (finish, keep)...");
  anariSetParameter(s.device, s.material, "slang.output", ANARI_STRING, "Invert");
  anariCommitParameters(s.device, s.material);
  render(s, rgb);
  if (!(rgb[0] > 100 && rgb[1] > 100 && rgb[2] < rgb[0] / 3))
    return fail("ISurfaceOutput::finish was not applied (expected yellow)", rgb);
  anariSetParameter(s.device, s.material, "slang.output", ANARI_STRING, "Erase");
  anariCommitParameters(s.device, s.material);
  render(s, rgb);
  if (rgb[0] + rgb[1] + rgb[2] != 0)
    return fail("ISurfaceOutput::keep did not discard (expected the background)", rgb);
  anariUnsetParameter(s.device, s.material, "slang.output");
  printf(" PASSED\n");

  printf("  vertex modifier...");
  float away = 10.f, none = 0.f;
  anariSetParameter(s.device, s.material, "slang.vertex", ANARI_STRING, "Shift");
  anariSetParameter(s.device, s.material, "slang.vertex.offset", ANARI_FLOAT32, &away);
  anariCommitParameters(s.device, s.material);
  render(s, rgb);
  if (rgb[0] + rgb[1] + rgb[2] != 0)
    return fail("IVertexModifier did not move the triangle away (expected the background)", rgb);
  anariSetParameter(s.device, s.material, "slang.vertex.offset", ANARI_FLOAT32, &none);
  anariCommitParameters(s.device, s.material);
  render(s, rgb);
  if (dominant(rgb) != 2)
    return fail("IVertexModifier with no offset changed the image (expected blue)", rgb);
  anariUnsetParameter(s.device, s.material, "slang.vertex");
  anariUnsetParameter(s.device, s.material, "slang.vertex.offset");
  printf(" PASSED\n");

  printf("  shader hooks...");
  float magenta[] = {1, 0, 1};
  anariUnsetParameter(s.device, s.material, "slang.material");
  anariSetParameter(s.device, s.material, "slang.hooks", ANARI_STRING, "Paint");
  anariSetParameter(s.device, s.material, "slang.hooks.tint", ANARI_FLOAT32_VEC3, magenta);
  anariCommitParameters(s.device, s.material);
  render(s, rgb);
  if (!(rgb[0] > 200 && rgb[2] > 200 && rgb[1] < 40))
    return fail("IShaderHooks::output() alone was not applied (expected magenta)", rgb);
  int full[3], halved[3];
  float half = 0.5f, whole = 1.f;
  anariSetParameter(s.device, s.material, "slang.material", ANARI_STRING, materialType);
  anariSetParameter(s.device, s.material, "slang.hooks", ANARI_STRING, "Halve");
  anariSetParameter(s.device, s.material, "slang.hooks.amount", ANARI_FLOAT32, &whole);
  anariCommitParameters(s.device, s.material);
  render(s, full);
  anariSetParameter(s.device, s.material, "slang.hooks.amount", ANARI_FLOAT32, &half);
  anariCommitParameters(s.device, s.material);
  render(s, halved);
  if (dominant(full) != 2 || std::abs(halved[2] * 2 - full[2]) > 3)
    return fail("IShaderHooks::lit() with a material override did not halve the color", halved);
  anariUnsetParameter(s.device, s.material, "slang.hooks");
  anariUnsetParameter(s.device, s.material, "slang.hooks.amount");
  anariUnsetParameter(s.device, s.material, "slang.hooks.tint");
  anariCommitParameters(s.device, s.material);
  printf(" PASSED\n");

  printf("  broken override...");
  const char *missing = "NoSuchMaterial";
  g_errors.clear();
  anariSetParameter(s.device, s.material, "slang.material", ANARI_STRING, missing);
  anariCommitParameters(s.device, s.material);
  render(s, rgb);
  if (g_errors.find("WEBGPU_MATERIAL_SLANG") == std::string::npos)
    return fail("an override that does not compile was not reported", rgb);
  if (!same(rgb, defaults))
    return fail("an override that does not compile did not fall back to default shading", rgb);
  printf(" PASSED\n");

  printf("  removal...");
  anariUnsetParameter(s.device, s.material, "slang.material");
  anariCommitParameters(s.device, s.material);
  render(s, rgb);
  if (!same(rgb, defaults))
    return fail("removing the override did not restore default shading", rgb);
  printf(" PASSED\n");

  anariRelease(s.device, s.material);
  anariRelease(s.device, s.frame);
  anariRelease(s.device, s.renderer);
  anariRelease(s.device, s.camera);
  anariRelease(s.device, s.world);
  anariRelease(s.device, s.device);
  printf("All Slang material tests passed.\n");
  return 0;
}

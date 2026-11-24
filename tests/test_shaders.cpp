// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "frame/ShaderPrograms.h"

#include <cstdio>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using slangkit::BindingKind;

static int g_failures = 0;

static void check(bool ok, const std::string &what)
{
  std::printf("%s: %s\n", ok ? "PASS" : "FAIL", what.c_str());
  if (!ok)
    ++g_failures;
}

static std::string readFile(const std::string &path)
{
  std::ifstream in(path, std::ios::binary);
  std::stringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

static std::string stripGeneratedHeader(const std::string &wgsl)
{
  size_t pos = wgsl.find("\n\n");
  return pos == std::string::npos ? wgsl : wgsl.substr(pos + 2);
}

static const slangkit::ReflectedBinding *byName(
    const slangkit::Artifact &a, const std::string &name)
{
  for (const auto &b : a.bindings)
    if (b.name == name)
      return &b;
  return nullptr;
}

static const slangkit::UniformField *field(
    const slangkit::ReflectedBinding *b, const std::string &name)
{
  if (b)
    for (const auto &f : b->fields)
      if (f.name == name)
        return &f;
  return nullptr;
}

static std::set<std::string> wgslBindings(const std::string &wgsl)
{
  std::set<std::string> out;
  std::regex decl(
      R"(@binding\((\d+)\) @group\((\d+)\) var(<[^>]*>)? \w+ : ([\w_]+))");
  for (std::sregex_iterator it(wgsl.begin(), wgsl.end(), decl), end; it != end;
       ++it) {
    const std::string access = (*it)[3];
    const std::string type = (*it)[4];
    std::string kind = "other";
    if (access == "<uniform>")
      kind = "uniform";
    else if (access == "<storage, read>")
      kind = "ro-storage";
    else if (access == "<storage, read_write>")
      kind = "storage";
    else if (type.rfind("texture_storage", 0) == 0)
      kind = "storage-texture";
    else if (type.rfind("texture", 0) == 0)
      kind = "texture";
    else if (type.rfind("sampler", 0) == 0)
      kind = "sampler";
    out.insert((*it)[2].str() + ":" + (*it)[1].str() + ":" + kind);
  }
  return out;
}

static std::set<std::string> reflectedBindings(const slangkit::Artifact &a)
{
  std::set<std::string> out;
  for (const auto &b : a.bindings) {
    if (!b.used)
      continue;
    const char *kind = "other";
    switch (b.kind) {
    case BindingKind::UniformBuffer:
      kind = "uniform";
      break;
    case BindingKind::ReadOnlyStorageBuffer:
      kind = "ro-storage";
      break;
    case BindingKind::StorageBuffer:
      kind = "storage";
      break;
    case BindingKind::SampledTexture:
      kind = "texture";
      break;
    case BindingKind::StorageTexture:
      kind = "storage-texture";
      break;
    case BindingKind::Sampler:
      kind = "sampler";
      break;
    default:
      break;
    }
    out.insert(std::to_string(b.group) + ":" + std::to_string(b.binding) + ":"
        + kind);
  }
  return out;
}

struct ProgramCase
{
  const char *name;
  std::vector<const char *> entries;
};

static const ProgramCase kCases[] = {
    {"clear", {"main"}},
    {"raster", {"depth_main", "main"}},
    {"pbr", {"pbr_depth_main", "pbr_main"}},
    {"volume", {"vol_classify", "vol_main"}},
};

static void testPrecompiled(slangkit::Compiler &compiler)
{
  check(anari_webgpu::shaderProgramNames().size() == 4,
      "the device owns four shader programs");
  for (const ProgramCase &c : kCases) {
    slangkit::Artifact a =
        compiler.compileProgram(anari_webgpu::shaderProgram(c.name));
    bool entriesOk = true;
    for (const char *e : c.entries)
      entriesOk = entriesOk
          && a.wgsl.find(std::string("fn ") + e + "(") != std::string::npos;
    check(a && a.source == slangkit::ArtifactSource::Precompiled && entriesOk,
        std::string("precompiled WGSL for ") + c.name
            + ", with every entry point");
  }
  check(!compiler.compileProgram(anari_webgpu::shaderProgram("no_such_shader")),
      "an unknown program fails");
}

static void testRuntimeMatchesBuildTime(slangkit::Compiler &compiler)
{
  slangkit::CompileOptions viaSlang;
  viaSlang.reflect = true;
  for (const ProgramCase &c : kCases) {
    const slangkit::Program program = anari_webgpu::shaderProgram(c.name);
    slangkit::Artifact built = compiler.compileProgram(program);
    slangkit::Artifact runtime = compiler.compileProgram(program, viaSlang);
    const std::string expected = stripGeneratedHeader(readFile(
        std::string(ANARI_WEBGPU_SOURCE_DIR "/generated/") + c.name + ".wgsl"));
    check(runtime && runtime.source == slangkit::ArtifactSource::Slang
            && runtime.wgsl == expected
            && stripGeneratedHeader(built.wgsl) == expected,
        std::string("runtime Slang output matches generated/") + c.name
            + ".wgsl");
    if (!runtime)
      std::printf("%s", runtime.diagnostics.c_str());
  }
}

static void testOverrides(slangkit::Compiler &compiler)
{
  const char *stripes = R"(
module test_stripes;
import slangkit_shading;
public struct StripedMaterial : IMaterial {
  public float4 stripeColor;
  public float  frequency;
  public MaterialSample evaluate(SurfaceSample s) {
    MaterialSample m = HostDrivenMaterial().evaluate(s);
    if (frac(s.position.y * frequency) < 0.5f)
      m.baseColor = stripeColor.rgb;
    return m;
  }
};
public struct ToonLighting : ILightingModel {
  public float3 direct(MaterialSample m, SurfaceSample s, LightSample l) {
    float n = max(dot(s.normal, l.direction), 0.f);
    return m.baseColor * l.radiance * (n > 0.5f ? 1.f : 0.3f);
  }
  public float3 ambient(MaterialSample m, SurfaceSample s, float3 a) { return m.baseColor * a; }
  public float3 resolve(float3 r, MaterialSample m) { return saturate(r); }
};
)";
  const slangkit::Program raster = anari_webgpu::shaderProgram("raster");
  const slangkit::Program pbr = anari_webgpu::shaderProgram("pbr");

  slangkit::CompileOptions striped;
  striped.modules = {stripes};
  striped.specializations = {{"IMaterial", "StripedMaterial"}};
  slangkit::Artifact r = compiler.compileProgram(raster, striped);
  const auto *block = byName(r, "shading");
  bool group0Binding2 = false;
  size_t group0 = 0;
  for (const auto &b : r.bindings) {
    group0 += b.group == 0 ? 1 : 0;
    group0Binding2 = group0Binding2 || (b.group == 0 && b.binding == 2);
  }
  check(r && r.source == slangkit::ArtifactSource::Slang && block
          && block->group == 1 && block->kind == BindingKind::UniformBuffer
          && field(block, "material.stripeColor")
          && field(block, "material.frequency") && group0Binding2
          && reflectedBindings(r) == wgslBindings(r.wgsl),
      "raster takes a consumer IMaterial; its uniforms land in group 1, group 0 is unchanged");
  if (!r)
    std::printf("%s", r.diagnostics.c_str());
  check(group0 == 8, "the override leaves the host's 8 group-0 bindings intact");

  slangkit::CompileOptions toon;
  toon.modules = {stripes};
  toon.specializations = {{"ILightingModel", "ToonLighting"}};
  slangkit::Artifact p = compiler.compileProgram(pbr, toon);
  check(p && p.wgsl != compiler.compileProgram(pbr).wgsl && !byName(p, "shading"),
      "pbr takes a consumer ILightingModel; a field-less override adds no bindings");
  if (!p)
    std::printf("%s", p.diagnostics.c_str());

  slangkit::CompileOptions both = toon;
  both.specializations.push_back({"IMaterial", "StripedMaterial"});
  check(static_cast<bool>(compiler.compileProgram(pbr, both)),
      "both interfaces at once");

  slangkit::CompileOptions phong;
  phong.specializations = {{"ILightingModel", "PhongLighting"}};
  check(static_cast<bool>(compiler.compileProgram(raster, phong)),
      "the library's PhongLighting can replace a shader's default");
}

static void testHooks(slangkit::Compiler &compiler)
{
  const char *hooks = R"(
module builtin_hooks;
import slangkit_hooks;
import slangkit_shading;
public struct Fade : IShaderHooks {
  float amount;
  public override void lit(inout FragmentContext f) { f.color.rgb *= amount; }
};
public struct Tint : IMaterial {
  float3 tint;
  public MaterialSample evaluate(SurfaceSample s) {
    MaterialSample m = HostDrivenMaterial().evaluate(s);
    m.baseColor *= tint;
    return m;
  }
};
)";
  for (const char *shader : {"raster", "pbr"}) {
    const slangkit::Program program = anari_webgpu::shaderProgram(shader);
    slangkit::CompileOptions hooksOnly;
    hooksOnly.modules = {hooks};
    hooksOnly.specializations = {{"IShaderHooks", "Fade"}};
    slangkit::Artifact a = compiler.compileProgram(program, hooksOnly);
    const auto *h = byName(a, "hookBlock");

    slangkit::CompileOptions both = hooksOnly;
    both.specializations.push_back({"IMaterial", "Tint"});
    slangkit::Artifact b = compiler.compileProgram(program, both);
    const auto *s = byName(b, "shading");
    const auto *g = byName(b, "hookBlock");
    check(a && h && h->kind == BindingKind::UniformBuffer
            && field(h, "hooks.amount") && b && s && g && s->group == 1
            && g->group == 2 && field(s, "material.tint")
            && reflectedBindings(b) == wgslBindings(b.wgsl),
        std::string(shader)
            + " offers the shader hooks; their block follows the shading block");
    if (!a)
      std::printf("%s", a.diagnostics.c_str());
  }
}

int main()
{
  slangkit::Compiler compiler;
  testPrecompiled(compiler);
  if (slangkit::Compiler::slangAvailable()) {
    testRuntimeMatchesBuildTime(compiler);
    testOverrides(compiler);
    testHooks(compiler);
  } else {
    std::printf("SKIP: Slang unavailable\n");
  }
  std::printf("%d failure(s)\n", g_failures);
  return g_failures == 0 ? 0 : 1;
}

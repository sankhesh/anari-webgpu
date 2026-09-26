// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "frame/ShaderPrograms.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

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

struct ProgramCase
{
  const char *name;
  std::vector<const char *> entries;
};

static const ProgramCase kCases[] = {
    {"clear", {"main"}},
    {"raster", {"main"}},
    {"pbr", {"pbr_main"}},
    {"volume", {"vol_main"}},
};

static void testPrecompiled(vtkslang::vtkSlangCompiler &compiler)
{
  check(anari_webgpu::shaderProgramNames().size() == 4,
      "the device owns four shader programs");
  for (const ProgramCase &c : kCases) {
    vtkslang::Artifact a =
        compiler.compileProgram(anari_webgpu::shaderProgram(c.name));
    bool entriesOk = true;
    for (const char *e : c.entries)
      entriesOk = entriesOk
          && a.wgsl.find(std::string("fn ") + e + "(") != std::string::npos;
    check(a && a.source == vtkslang::ArtifactSource::Precompiled && entriesOk,
        std::string("precompiled WGSL for ") + c.name
            + ", with every entry point");
  }
  check(!compiler.compileProgram(anari_webgpu::shaderProgram("no_such_shader")),
      "an unknown program fails");
}

static void testRuntimeMatchesBuildTime(vtkslang::vtkSlangCompiler &compiler)
{
  vtkslang::CompileOptions viaSlang;
  viaSlang.reflect = true;
  for (const ProgramCase &c : kCases) {
    const vtkslang::Program program = anari_webgpu::shaderProgram(c.name);
    vtkslang::Artifact built = compiler.compileProgram(program);
    vtkslang::Artifact runtime = compiler.compileProgram(program, viaSlang);
    const std::string expected = stripGeneratedHeader(readFile(
        std::string(ANARI_WEBGPU_SOURCE_DIR "/generated/") + c.name + ".wgsl"));
    check(runtime && runtime.source == vtkslang::ArtifactSource::Slang
            && runtime.wgsl == expected
            && stripGeneratedHeader(built.wgsl) == expected,
        std::string("runtime Slang output matches generated/") + c.name
            + ".wgsl");
    if (!runtime)
      std::printf("%s", runtime.diagnostics.c_str());
  }
}

int main()
{
  vtkslang::vtkSlangCompiler compiler;
  testPrecompiled(compiler);
  if (vtkslang::vtkSlangCompiler::slangAvailable()) {
    testRuntimeMatchesBuildTime(compiler);
  } else {
    std::printf("SKIP: Slang unavailable\n");
  }
  std::printf("%d failure(s)\n", g_failures);
  return g_failures == 0 ? 0 : 1;
}

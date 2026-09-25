// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "frame/ShaderPrograms.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string readFile(const std::string &path)
{
  std::ifstream in(path, std::ios::binary);
  std::stringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

} // namespace

int main(int argc, char **argv)
{
  if (argc < 2) {
    std::fprintf(stderr, "usage: %s <out-dir> [--check <dir>]\n", argv[0]);
    return 2;
  }
  const std::string outDir = argv[1];
  std::string checkDir;
  for (int i = 2; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--check" && i + 1 < argc)
      checkDir = argv[++i];
    else {
      std::fprintf(stderr, "unknown argument '%s'\n", arg.c_str());
      return 2;
    }
  }

  if (!vtkslang::vtkSlangCompiler::slangAvailable()) {
    std::fprintf(stderr, "Slang is unavailable; the WGSL cannot be regenerated\n");
    return 77;
  }

  vtkslang::vtkSlangCompiler compiler;
  int failures = 0;
  for (const std::string &name : anari_webgpu::shaderProgramNames()) {
    vtkslang::Program program = anari_webgpu::shaderProgram(name);
    program.precompiled.clear();
    const vtkslang::Artifact a = compiler.compileProgram(program);
    if (!a) {
      std::fprintf(stderr,
          "%s.slang failed to compile:\n%s",
          name.c_str(),
          a.diagnostics.c_str());
      ++failures;
      continue;
    }
    const std::string text = "// Generated from shaders/" + name
        + ".slang with its default specializations\n// by vtkSlang (Slang "
        + vtkslang::vtkSlangCompiler::slangVersion()
        + "). Do not edit; regenerate with the\n// anari_webgpu_update_wgsl "
          "target.\n\n"
        + a.wgsl;

    const std::string path = outDir + "/" + name + ".wgsl";
    {
      std::ofstream out(path, std::ios::binary | std::ios::trunc);
      out << text;
      if (!out) {
        std::fprintf(stderr, "cannot write %s\n", path.c_str());
        ++failures;
        continue;
      }
    }
    if (!checkDir.empty() && readFile(checkDir + "/" + name + ".wgsl") != text) {
      std::fprintf(stderr,
          "generated/%s.wgsl is stale; run the anari_webgpu_update_wgsl target "
          "and commit generated/\n",
          name.c_str());
      ++failures;
    }
  }
  return failures == 0 ? 0 : 1;
}

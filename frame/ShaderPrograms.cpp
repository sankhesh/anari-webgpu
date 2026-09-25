// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include "ShaderPrograms.h"

#include "anariShaderSources.h"
#include "anariShaderWGSL.h"

namespace anari_webgpu {

namespace {

struct ProgramEntry
{
  const char *name;
  std::vector<const char *> modules;
};

const ProgramEntry kPrograms[] = {
    {"clear", {}},
    {"raster", {"anari_raster_common"}},
    {"pbr", {"anari_raster_common"}},
    {"volume", {"anari_scivis"}},
};

} // namespace

std::vector<std::string> shaderProgramNames()
{
  std::vector<std::string> names;
  for (const auto &entry : kPrograms)
    names.push_back(entry.name);
  return names;
}

vtkslang::Program shaderProgram(const std::string &name)
{
  vtkslang::Program program;
  for (const auto &entry : kPrograms) {
    if (name != entry.name)
      continue;
    program.name = entry.name;
    program.source = anariShaderSources::get(entry.name);
    for (const char *module : entry.modules)
      program.modules.push_back(anariShaderSources::get(module));
    if (const char *wgsl = anariShaderWGSL::get(entry.name)) {
      vtkslang::Artifact precompiled;
      precompiled.target = vtkslang::Target::WGSL;
      precompiled.wgsl = wgsl;
      program.precompiled.push_back(precompiled);
    }
  }
  return program;
}

} // namespace anari_webgpu

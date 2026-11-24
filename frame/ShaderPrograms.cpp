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
    {"volume", {}},
};

} // namespace

std::vector<std::string> shaderProgramNames()
{
  std::vector<std::string> names;
  for (const auto &entry : kPrograms)
    names.push_back(entry.name);
  return names;
}

slangkit::Program shaderProgram(const std::string &name)
{
  slangkit::Program program;
  for (const auto &entry : kPrograms) {
    if (name != entry.name)
      continue;
    program.name = entry.name;
    program.source = anariShaderSources::get(entry.name);
    for (const char *module : entry.modules)
      program.modules.push_back(anariShaderSources::get(module));
    if (const char *wgsl = anariShaderWGSL::get(entry.name)) {
      slangkit::Artifact precompiled;
      precompiled.target = slangkit::Target::WGSL;
      precompiled.wgsl = wgsl;
      program.precompiled.push_back(precompiled);
    }
  }
  return program;
}

} // namespace anari_webgpu

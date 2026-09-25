// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "vtkSlangCompiler.h"

#include <string>
#include <vector>

namespace anari_webgpu {

std::vector<std::string> shaderProgramNames();

vtkslang::Program shaderProgram(const std::string &name);

} // namespace anari_webgpu

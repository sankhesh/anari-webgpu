# Copyright 2026 The anari-webgpu Authors SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.16)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_LIST_DIR})
  set(STANDALONE_SOURCE_TREE TRUE)
else()
  set(STANDALONE_SOURCE_TREE FALSE)
endif()

if(STANDALONE_SOURCE_TREE)
  message(STATUS "CMake version: ${CMAKE_VERSION}")
endif()

# Language setup

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_BUILD_TYPE_INIT Release)

set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_INSTALL_MESSAGE LAZY)

# Establish project

project(anari_library_webgpu VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(GNUInstallDirs)

# Dependencies

find_package(anari REQUIRED)

# WebGPU backend - user must provide a wgpu-native or Dawn installation

# * Option 1: wgpu-native (default)
# * Option 2: Dawn
# * Option 3: Emscripten (auto-detected)

option(WEBGPU_BACKEND "WebGPU backend implementation: wgpu-native, dawn, or emscripten" "")

if(EMSCRIPTEN)
  message(STATUS "Building for Emscripten - using built-in WebGPU support")
  set(WEBGPU_BACKEND "emscripten")
endif()
message(STATUS "WebGPU backend: ${WEBGPU_BACKEND}")

# Try to find webgpu headers and library
if(NOT
   WEBGPU_BACKEND
   STREQUAL
   "emscripten")
  if(WEBGPU_BACKEND STREQUAL "dawn")
    # If backend is dawn, use Dawn CMake package
    find_package(Dawn REQUIRED)
    find_path(WEBGPU_INCLUDE_DIR NAMES webgpu/webgpu.h HINTS ${DAWN_DIR}/include
                                                             $ENV{DAWN_DIR}/include)
  else()
    # Look for wgpu-native first
    find_library(
      WGPU_NATIVE_LIBRARY
      NAMES wgpu_native wgpu
      HINTS ${WGPU_NATIVE_DIR}/lib
            ${WGPU_NATIVE_DIR}
            $ENV{WGPU_NATIVE_DIR}/lib
            $ENV{WGPU_NATIVE_DIR})

    find_path(
      WEBGPU_INCLUDE_DIR
      NAMES webgpu/webgpu.h
      HINTS ${WGPU_NATIVE_DIR}/include
            ${WGPU_NATIVE_DIR}
            $ENV{WGPU_NATIVE_DIR}/include
            $ENV{WGPU_NATIVE_DIR})

    if(NOT WGPU_NATIVE_LIBRARY AND NOT DAWN_DIR)
      # Try to find Dawn if wgpu-native is not found
      find_library(DAWN_LIBRARY NAMES Dawn webgpu_dawn HINTS ${DAWN_DIR}/lib $ENV{DAWN_DIR}/lib)
    endif()
  endif()
endif()

find_package(vtkSlang 0.1 QUIET)
if(NOT vtkSlang_FOUND)
  include(FetchContent)
  FetchContent_Declare(vtkSlang
    GIT_REPOSITORY https://github.com/sankhesh/vtkSlang.git
    GIT_TAG 1dda0b547a9a48553ab1a907e992b30597aaca56)
  FetchContent_MakeAvailable(vtkSlang)
endif()

# Shaders

set(_shader_dir "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(_generated_dir "${CMAKE_CURRENT_SOURCE_DIR}/generated")
set(_sources_header "${CMAKE_CURRENT_BINARY_DIR}/anariShaderSources.h")
set(_wgsl_header "${CMAKE_CURRENT_BINARY_DIR}/anariShaderWGSL.h")
file(GLOB _shader_files CONFIGURE_DEPENDS "${_shader_dir}/*.slang")
file(GLOB _generated_files CONFIGURE_DEPENDS "${_generated_dir}/*.wgsl")

add_custom_command(
  OUTPUT "${_sources_header}"
  COMMAND "${CMAKE_COMMAND}" -DSHADER_DIR=${_shader_dir} -DOUT_HEADER=${_sources_header}
          -DNAMESPACE=anariShaderSources -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShaders.cmake"
  DEPENDS ${_shader_files} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShaders.cmake"
  COMMENT "Embedding Slang shader sources into anariShaderSources.h"
  VERBATIM)

add_custom_command(
  OUTPUT "${_wgsl_header}"
  COMMAND "${CMAKE_COMMAND}" -DSHADER_DIR=${_generated_dir} -DOUT_HEADER=${_wgsl_header}
          -DPATTERN=*.wgsl -DNAMESPACE=anariShaderWGSL
          -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShaders.cmake"
  DEPENDS ${_generated_files} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShaders.cmake"
  COMMENT "Embedding generated WGSL into anariShaderWGSL.h"
  VERBATIM)

add_library(anari_webgpu_shaders STATIC frame/ShaderPrograms.cpp "${_sources_header}"
                                        "${_wgsl_header}")
target_include_directories(anari_webgpu_shaders PUBLIC "${CMAKE_CURRENT_LIST_DIR}"
                                                PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(anari_webgpu_shaders PUBLIC vtkSlang::vtkSlang)

add_executable(anari_webgpu_generate_wgsl tools/GenerateWGSL.cpp)
target_link_libraries(anari_webgpu_generate_wgsl PRIVATE anari_webgpu_shaders)
add_custom_target(anari_webgpu_update_wgsl
  COMMAND anari_webgpu_generate_wgsl "${_generated_dir}"
  COMMENT "Regenerating generated/*.wgsl from shaders/*.slang"
  VERBATIM)

# Build device target

add_library(
  ${PROJECT_NAME} SHARED
  WebGPUDevice.cpp
  WebGPUDeviceGlobalState.cpp
  WebGPULibrary.cpp
  Object.cpp
  camera/Camera.cpp
  frame/Frame.cpp
  geometry/Geometry.cpp
  light/Light.cpp
  material/Material.cpp
  renderer/Renderer.cpp
  sampler/Sampler.cpp
  spatial_field/SpatialField.cpp
  surface/Surface.cpp
  volume/Volume.cpp
  world/Group.cpp
  world/Instance.cpp
  world/World.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE anari::helium vtkSlang::vtkSlang anari_webgpu_shaders)

# Link WebGPU
if(EMSCRIPTEN)
  target_compile_options(${PROJECT_NAME} PRIVATE "-sUSE_WEBGPU=1")
elseif(WEBGPU_BACKEND STREQUAL "dawn")
  message(STATUS "Using Dawn via CMake package")
  target_link_libraries(${PROJECT_NAME} PRIVATE dawn::webgpu_dawn)
  if(WEBGPU_INCLUDE_DIR)
    target_include_directories(${PROJECT_NAME} PRIVATE ${WEBGPU_INCLUDE_DIR})
  endif()
elseif(WGPU_NATIVE_LIBRARY)
  message(STATUS "Using wgpu-native: ${WGPU_NATIVE_LIBRARY}")
  target_link_libraries(${PROJECT_NAME} PRIVATE ${WGPU_NATIVE_LIBRARY})
  if(WEBGPU_INCLUDE_DIR)
    target_include_directories(${PROJECT_NAME} PRIVATE ${WEBGPU_INCLUDE_DIR})
  endif()
else()
  message(
    WARNING "No WebGPU implementation found. The device will build but fall back to "
            "software rasterization at runtime.\n"
            "Set WGPU_NATIVE_DIR or DAWN_DIR to point to your WebGPU installation.\n"
            "Example: cmake -DWGPU_NATIVE_DIR=/path/to/wgpu-native ..")
  # Still build - the device will use software fallback
  if(WEBGPU_INCLUDE_DIR)
    target_include_directories(${PROJECT_NAME} PRIVATE ${WEBGPU_INCLUDE_DIR})
  else()
    # Use bundled minimal webgpu header stub
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external)
  endif()
endif()

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} EXPORT_MACRO_NAME "WEBGPU_EXPORT")

target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
                                                   $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

# Code generation

anari_generate_queries(
  DEVICE_TARGET
  ${PROJECT_NAME}
  CPP_NAMESPACE
  anari_webgpu
  JSON_DEFINITIONS_FILE
  ${CMAKE_CURRENT_SOURCE_DIR}/WebGPUDefinitions.json)

# Install


# Tests
option(BUILD_TESTS "Build test executables" ON)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

install(
  TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Example application

option(BUILD_WEBGPU_EXAMPLE "Build the example application" ON)

if(BUILD_WEBGPU_EXAMPLE)
  add_executable(anari_webgpu_example examples/render_triangle.cpp)
  target_link_libraries(anari_webgpu_example PRIVATE anari::anari)
  target_include_directories(anari_webgpu_example
                             PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)
  
  add_executable(pbr_surface_demo examples/pbr_surface_demo.cpp)
  target_link_libraries(pbr_surface_demo PRIVATE anari::anari)
  target_include_directories(pbr_surface_demo
                             PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)

  add_executable(surface_demo_simple examples/surface_demo_simple.cpp)
  target_link_libraries(surface_demo_simple PRIVATE anari::anari)
  target_include_directories(surface_demo_simple
                             PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)
endif()

# Copyright 2026 The anari-webgpu Authors
# SPDX-License-Identifier: Apache-2.0

find_package(anari REQUIRED)

set(TEST_SOURCES
  test_object_lifecycle.cpp
  test_materials.cpp
  test_lights.cpp
  test_triangle_rendering.cpp
  test_mesh_rendering.cpp
  test_volume_rendering.cpp
  test_cubemap.cpp
  test_gltf_scene.cpp
  test_multi_light_rendering.cpp
  test_pbr_rendering.cpp
)

foreach(test_src ${TEST_SOURCES})
  get_filename_component(test_name ${test_src} NAME_WE)
  add_executable(${test_name} ${test_src})
  target_link_libraries(${test_name} PRIVATE anari::anari)
  target_compile_features(${test_name} PRIVATE cxx_std_17)
  add_test(NAME ${test_name} COMMAND ${test_name})
  set_tests_properties(${test_name} PROPERTIES
    ENVIRONMENT "DYLD_LIBRARY_PATH=${CMAKE_BINARY_DIR}:$ENV{DYLD_LIBRARY_PATH};ANARI_WEBGPU_DISABLE=1"
  )
endforeach()

add_executable(test_shaders test_shaders.cpp)
target_link_libraries(test_shaders PRIVATE anari_webgpu_shaders)
target_compile_features(test_shaders PRIVATE cxx_std_17)
target_compile_definitions(test_shaders PRIVATE
  ANARI_WEBGPU_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
add_test(NAME test_shaders COMMAND test_shaders)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
add_test(NAME test_generated_wgsl_up_to_date
  COMMAND anari_webgpu_generate_wgsl "${CMAKE_CURRENT_BINARY_DIR}/generated"
    --check "${PROJECT_SOURCE_DIR}/generated")
set_tests_properties(test_generated_wgsl_up_to_date PROPERTIES SKIP_RETURN_CODE 77)

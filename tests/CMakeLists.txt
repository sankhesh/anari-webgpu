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
)

foreach(test_src ${TEST_SOURCES})
  get_filename_component(test_name ${test_src} NAME_WE)
  add_executable(${test_name} ${test_src})
  target_link_libraries(${test_name} PRIVATE anari::anari)
  target_compile_features(${test_name} PRIVATE cxx_std_17)
  add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

Standard CMake build. Build output goes to a build directory of your choice.

```bash
# Configure (choose one)
cmake --preset default        # Release, software-only (no WebGPU library required)
cmake --preset debug          # Debug, software-only
cmake --preset wgpu-native    # Release + wgpu-native backend (set $WGPU_NATIVE_DIR first)
cmake --preset dawn           # Debug + Google Dawn backend

# Build
cmake --build ../bld/default

# Install (optional)
cmake --install ../bld/default --prefix /desired/install/path
```

Set `anari_DIR` to point to your ANARI SDK install (e.g. `-Danari_DIR=~/Projects/anari/anari-sdk/install/lib/cmake/anari-0.16.0`).

Build outputs:
- `libanari_webgpu.{so,dylib,dll}` — the ANARI device library
- `anari_webgpu_example` — renders a triangle to `triangle.ppm`

Run the example:
```bash
export DYLD_LIBRARY_PATH=../bld/default:$DYLD_LIBRARY_PATH  # macOS
./anari_webgpu_example
```

Tests live in `tests/`. Build with `-DBUILD_TESTS=ON` (the default) and run via `ctest`.

## Architecture

This is an [ANARI](https://www.khronos.org/anari/) device implementation backed by WebGPU, following the **helium** framework pattern from the ANARI SDK. The library is loaded at runtime via `anariLoadLibrary("webgpu", ...)`.

### Layer overview

```
WebGPULibrary        (anari::LibraryImpl)  — entry point, creates devices
WebGPUDevice         (helium::BaseDevice)  — implements all anariNew* factory calls
WebGPUDeviceGlobalState                   — shared WebGPU handles (instance/adapter/device/queue)
Object               (helium::BaseObject)  — base for all scene objects
```

All code lives in the `anari_webgpu` namespace.

### Object hierarchy

Every ANARI scene object inherits from `Object` (which inherits `helium::BaseObject`). The object types map directly to ANARI concepts:

- `camera/` — `Camera`, `PerspectiveCamera`
- `frame/` — `Frame` — orchestrates rendering (see below)
- `geometry/` — `Geometry`, `TriangleGeometry`
- `light/` — `Light`, `DirectionalLight`
- `material/` — `Material`, `MatteMaterial`
- `renderer/` — `Renderer` (background color, ambient radiance)
- `surface/` — `Surface` (geometry + material pair)
- `world/` — `World`, `Group`, `Instance`
- `array/` — Array wrappers (delegates to helium arrays)
- `sampler/`, `spatial_field/`, `volume/` — stub implementations

### Parameter lifecycle (helium pattern)

ANARI parameters flow through two phases triggered by `anariCommitParameters()`:

1. `commitParameters()` — reads params from the helium store (e.g. `getParam<T>(...)`, `getParamObject<T>(...)`)
2. `finalize()` — validates state, allocates GPU resources

`ChangeObserverPtr<T>` tracks whether referenced objects have changed. `IntrusivePtr<T>` provides ref-counted ownership.

### Rendering pipeline (`Frame::renderFrame`)

1. Flushes the commit buffer so all pending parameter changes propagate.
2. Fills the pixel buffer with the background color.
3. **GPU path** (when `state->wgpuDevice != nullptr`): lazily initializes WebGPU resources (`initWebGPUResources`), encodes a render pass, submits, and copies the render texture back to CPU via `wgpuBufferMapAsync` + polling `wgpuInstanceProcessEvents`.
4. **Software fallback** (`softwareRasterize`): CPU barycentric rasterizer using the same view/projection math.

The WGSL shader is embedded as a string literal in `frame/Frame.cpp`. The vertex buffer layout is interleaved: `position(vec3f) | normal(vec3f) | color(vec4f)` = 10 floats per vertex.

### WebGPU header strategy

`WebGPUDeviceGlobalState.h` forward-declares the WGPUDevice/Instance/Adapter/Queue opaque pointer types so the full `webgpu.h` is **not** pulled into every translation unit. Only files that call WebGPU API functions include the actual header. When no WebGPU backend is found, `external/webgpu/` provides a minimal stub header so the project still compiles.

### Code generation

`WebGPUDefinitions.json` is the authoritative source of supported object subtypes and their parameters. CMake's `anari_generate_queries()` call (at the bottom of `CMakeLists.txt`) generates C++ query-function bodies into the build directory. Edit `WebGPUDefinitions.json` when adding new parameters or object subtypes.

### Math

`WebGPUMath.h` re-exports `anari::math` and `helium::math` (backed by `linalg`). Matrices are **column-major** in C++ (`mat4[col][row]`), but WebGPU/WGSL expects row-major, so the uniform-buffer upload in `Frame.cpp` transposes: `uniforms.mvp[r*4+c] = finalMVP[c][r]`.

### Type registration macros

Every `Object` subtype uses two macros:
- `WEBGPU_ANARI_TYPEFOR_SPECIALIZATION(Type*, ANARI_TYPE)` in the `.h` — tells ANARI's type system the C++ ↔ ANARI mapping.
- `WEBGPU_ANARI_TYPEFOR_DEFINITION(Type*)` in the `.cpp` — emits the definition (must appear in exactly one TU).

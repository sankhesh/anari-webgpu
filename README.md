# ANARI WebGPU Device

An [ANARI](https://www.khronos.org/anari/) device implementation that uses
[WebGPU](https://www.w3.org/TR/webgpu/) as its rendering backend. The device
follows the same helium-based architecture as the example devices in the ANARI
SDK and can be loaded at runtime by any ANARI application via the library name
`"webgpu"`.

When a WebGPU-capable GPU is available (through **wgpu-native** or **Dawn**),
geometry is rasterized on the GPU. If no WebGPU implementation is found at build
time or at runtime, the device automatically falls back to a built-in software
rasterizer so that scenes can still be rendered without a GPU.

## Supported ANARI object types

| Object         | Subtypes                 |
|----------------|--------------------------|
| Camera         | `perspective`            |
| Geometry       | `triangle`               |
| Material       | `matte`                  |
| Light          | `directional`            |
| Renderer       | `default`                |
| Instance       | `transform`              |

## Dependencies

The following must be available before building.

| Dependency | Minimum version | Notes |
|------------|-----------------|-------|
| CMake      | 3.16            |       |
| C++17 compiler | GCC 9+, Clang 10+, MSVC 2019+ | |
| ANARI SDK  | 0.16.0          | Provides the `anari` CMake package (headers, helium framework, and the `anari_generate_queries` CMake function) |
| **One of:**                             |||
| wgpu-native | 0.19+          | Preferred native WebGPU backend. Download from https://github.com/gfx-rs/wgpu-native/releases |
| Dawn       | latest          | Google's WebGPU implementation. Build from https://dawn.googlesource.com/dawn |
| Emscripten | 3.1+            | For browser/WASM builds; WebGPU support is built-in |

> **Note:** If neither wgpu-native nor Dawn is available, the project still
> compiles and links using a bundled header stub. The device will work in
> software-rasterization mode only.

## Building

### 1. Install the ANARI SDK

If you have already built and installed the ANARI SDK, make sure the install
prefix is in your `CMAKE_PREFIX_PATH`, or pass `-Danari_DIR=<path>` when
configuring.

### 2. Configure with CMake

```bash
# --- Option A: software-only (no WebGPU library required) ---
cmake --preset default

# --- Option B: with wgpu-native ---
export WGPU_NATIVE_DIR=/path/to/wgpu-native
cmake --preset wgpu-native

# --- Option C: with Dawn ---
export DAWN_DIR=/path/to/dawn/install
cmake --preset dawn

# --- Option D: manual configuration ---
cmake -B ../bld/manual -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DWGPU_NATIVE_DIR=/path/to/wgpu-native \
  -Danari_DIR=/path/to/anari-sdk/install/lib/cmake/anari-0.16.0
```

### 3. Build

```bash
cmake --build ../bld/default      # or whichever preset you chose
```

The build produces:

- `libanari_webgpu.so` (Linux) / `libanari_webgpu.dylib` (macOS) / `anari_webgpu.dll` (Windows) — the ANARI device library
- `anari_webgpu_example` — a small demo that renders a colored triangle to a PPM file

### 4. Install (optional)

```bash
cmake --install ../bld/default --prefix /desired/install/path
```

## Running the example

```bash
# Make sure the device library is discoverable
export LD_LIBRARY_PATH=../bld/default:$LD_LIBRARY_PATH   # Linux
# or
export DYLD_LIBRARY_PATH=../bld/default:$DYLD_LIBRARY_PATH  # macOS

./anari_webgpu_example
# Writes triangle.ppm in the current directory
```

## Using the device in your application

```c
#include <anari/anari.h>

// Status callback (optional but recommended)
void statusFunc(const void *userData, ANARIDevice device,
    ANARIObject source, ANARIDataType sourceType,
    ANARISeverity severity, ANARIStatusCode code,
    const char *message) {
  fprintf(stderr, "[ANARI] %s\n", message);
}

int main() {
  // Load the library - ANARI looks for libanari_webgpu on the library path
  ANARILibrary lib = anariLoadLibrary("webgpu", statusFunc, NULL);
  ANARIDevice device = anariNewDevice(lib, "default");
  anariCommitParameters(device, device);

  // --- Build a scene ---

  // Geometry
  float positions[] = {
    -0.5f, -0.5f, -2.0f,
     0.5f, -0.5f, -2.0f,
     0.0f,  0.5f, -2.0f,
  };
  ANARIArray1D posArr = anariNewArray1D(
      device, positions, NULL, NULL, ANARI_FLOAT32_VEC3, 3);

  ANARIGeometry geom = anariNewGeometry(device, "triangle");
  anariSetParameter(device, geom, "vertex.position", ANARI_ARRAY1D, &posArr);
  anariCommitParameters(device, geom);

  // Material
  ANARIMaterial mat = anariNewMaterial(device, "matte");
  float color[] = {0.9f, 0.6f, 0.2f, 1.0f};
  anariSetParameter(device, mat, "color", ANARI_FLOAT32_VEC4, color);
  anariCommitParameters(device, mat);

  // Surface = geometry + material
  ANARISurface surf = anariNewSurface(device);
  anariSetParameter(device, surf, "geometry", ANARI_GEOMETRY, &geom);
  anariSetParameter(device, surf, "material", ANARI_MATERIAL, &mat);
  anariCommitParameters(device, surf);

  // World
  ANARIWorld world = anariNewWorld(device);
  ANARIArray1D surfArr = anariNewArray1D(
      device, &surf, NULL, NULL, ANARI_SURFACE, 1);
  anariSetParameter(device, world, "surface", ANARI_ARRAY1D, &surfArr);
  anariCommitParameters(device, world);

  // Camera
  ANARICamera cam = anariNewCamera(device, "perspective");
  float pos[] = {0, 0, 0}, dir[] = {0, 0, -1}, up[] = {0, 1, 0};
  float fovy = 1.047f, aspect = 800.0f / 600.0f;
  anariSetParameter(device, cam, "position",  ANARI_FLOAT32_VEC3, pos);
  anariSetParameter(device, cam, "direction", ANARI_FLOAT32_VEC3, dir);
  anariSetParameter(device, cam, "up",        ANARI_FLOAT32_VEC3, up);
  anariSetParameter(device, cam, "fovy",      ANARI_FLOAT32, &fovy);
  anariSetParameter(device, cam, "aspect",    ANARI_FLOAT32, &aspect);
  anariCommitParameters(device, cam);

  // Renderer
  ANARIRenderer ren = anariNewRenderer(device, "default");
  float bg[] = {0.1f, 0.1f, 0.1f, 1.0f};
  anariSetParameter(device, ren, "background", ANARI_FLOAT32_VEC4, bg);
  anariCommitParameters(device, ren);

  // Frame
  ANARIFrame frame = anariNewFrame(device);
  uint32_t size[] = {800, 600};
  ANARIDataType colorType = ANARI_UFIXED8_VEC4;
  anariSetParameter(device, frame, "size", ANARI_UINT32_VEC2, size);
  anariSetParameter(device, frame, "channel.color", ANARI_DATA_TYPE, &colorType);
  anariSetParameter(device, frame, "renderer", ANARI_RENDERER, &ren);
  anariSetParameter(device, frame, "camera",   ANARI_CAMERA,   &cam);
  anariSetParameter(device, frame, "world",    ANARI_WORLD,    &world);
  anariCommitParameters(device, frame);

  // Render
  anariRenderFrame(device, frame);
  anariFrameReady(device, frame, ANARI_WAIT);

  // Read back pixels
  uint32_t w, h;
  ANARIDataType pType;
  const void *pixels = anariMapFrame(
      device, frame, "channel.color", &w, &h, &pType);
  // ... use pixel data ...
  anariUnmapFrame(device, frame, "channel.color");

  // Cleanup
  anariRelease(device, frame);
  anariRelease(device, ren);
  anariRelease(device, cam);
  anariRelease(device, world);
  anariRelease(device, surfArr);
  anariRelease(device, surf);
  anariRelease(device, geom);
  anariRelease(device, mat);
  anariRelease(device, posArr);
  anariRelease(device, device);
  anariUnloadLibrary(lib);

  return 0;
}
```

## Architecture

The device is structured following the **helium** framework from the ANARI SDK:

```
WebGPULibrary          (anari::LibraryImpl)
 └─ WebGPUDevice       (helium::BaseDevice)
     ├─ Frame           (helium::BaseFrame)  ← orchestrates rendering
     ├─ World           (Object)
     │   ├─ Instance    (Object)  ← holds transform + Group
     │   └─ Group       (Object)  ← holds Surfaces, Volumes, Lights
     ├─ Surface         (Object)  ← Geometry + Material pair
     ├─ Geometry        (Object)  ← triangle mesh vertex data
     ├─ Material        (Object)  ← matte diffuse
     ├─ Camera          (Object)  ← perspective
     ├─ Light           (Object)  ← directional
     ├─ Renderer        (Object)  ← background color, ambient
     └─ Array1D/2D/3D  (helium arrays)
```

When `renderFrame()` is called, the `Frame` object:

1. Attempts to use WebGPU GPU rasterization if a `WGPUDevice` is available.
2. Falls back to a built-in software rasterizer otherwise.
3. Writes RGBA pixel data into a CPU-side buffer that the application can map.

## WebGPU rendering pipeline

The GPU path creates a single render pipeline with WGSL shaders that support:

- Per-vertex positions, normals, and colors (interleaved vertex buffer)
- A uniform buffer carrying the MVP matrix, material color, light direction,
  and ambient radiance
- Depth testing (`Depth24Plus`, less-than compare)
- Readback via `CopyTextureToBuffer` + `MapAsync`

## License

Apache-2.0. See [LICENSE](LICENSE) for details.

# ANARI WebGPU PBR Surface Rendering Demo

This example demonstrates physically-based rendering (PBR) using the ANARI WebGPU device implementation.

## Features

- **5 spheres** with varying metallic properties (0.0 to 1.0)
- **PBR materials** using the `physicallyBased` material type
- **Three-point lighting** setup (key, fill, and rim lights)
- **Ground plane** for context and reflections
- **High-quality output** at 1920x1080 resolution

## Material Progression

The demo showcases how metallic values affect appearance:

| Sphere | Color  | Metallic | Description |
|--------|--------|----------|-------------|
| 1      | Red    | 0.00     | Pure dielectric (non-metallic) |
| 2      | Green  | 0.25     | Mostly dielectric |
| 3      | Blue   | 0.50     | Half metal, half dielectric |
| 4      | Gold   | 0.75     | Mostly metallic |
| 5      | Silver | 1.00     | Pure metal |

All spheres use a roughness value of 0.3 to showcase visible reflections while maintaining material characteristics.

## Lighting Setup

**Key Light**: Main illumination from upper right (warm white, high intensity)
- Direction: (-0.3, -0.7, -0.6)
- Color: Warm white (1.0, 0.98, 0.95)
- Intensity: 2.0

**Fill Light**: Soft light from left (cool tone, medium intensity)
- Direction: (0.8, -0.3, -0.5)
- Color: Cool blue (0.6, 0.7, 0.9)
- Intensity: 0.5

**Rim Light**: Backlight for edge definition (warm, medium intensity)
- Direction: (0.0, 0.3, 1.0)
- Color: Warm (1.0, 0.9, 0.8)
- Intensity: 1.0

## Building

```bash
cd build
cmake --build . --target pbr_surface_demo
```

## Running

```bash
# On macOS
export DYLD_LIBRARY_PATH=../bld:$DYLD_LIBRARY_PATH
./pbr_surface_demo

# On Linux
export LD_LIBRARY_PATH=../bld:$LD_LIBRARY_PATH
./pbr_surface_demo
```

## Output

The demo generates `pbr_surface_demo.ppm` in the current directory, which can be converted to PNG:

```bash
# Using ImageMagick
convert pbr_surface_demo.ppm pbr_surface_demo.png

# Using macOS sips
sips -s format png pbr_surface_demo.ppm --out pbr_surface_demo.png
```

## Code Highlights

### PBR Material Creation

```cpp
ANARIMaterial mat = anariNewMaterial(device, "physicallyBased");

float baseColor[] = {1.0f, 0.86f, 0.14f, 1.0f}; // Gold
float metallic = 0.75f;
float roughness = 0.3f;

anariSetParameter(device, mat, "baseColor", ANARI_FLOAT32_VEC3, baseColor);
anariSetParameter(device, mat, "metallic", ANARI_FLOAT32, &metallic);
anariSetParameter(device, mat, "roughness", ANARI_FLOAT32, &roughness);
anariCommitParameters(device, mat);
```

### Sphere Mesh Generation

The demo includes a parametric sphere generator:

```cpp
Mesh createSphere(float radius, int segments, int rings);
```

This creates a UV sphere with proper normals for realistic lighting.

### Three-Point Lighting

Professional 3-point lighting is achieved using three `directional` lights with different colors and intensities to create depth and visual interest.

## Technical Details

- **Geometry**: Triangle meshes with vertex normals
- **Materials**: Cook-Torrance PBR model
- **Lighting**: Multiple directional lights with color and intensity control
- **Camera**: Perspective projection with configurable FOV
- **Resolution**: 1920x1080 (Full HD)
- **Output Format**: PPM (Portable Pixmap)

## Customization

You can modify various parameters in the code:

- **Sphere count and positions**: Adjust the loop in main()
- **Material properties**: Change metallic, roughness, baseColor
- **Lighting**: Add/remove lights, change directions and colors
- **Camera**: Modify position, direction, FOV
- **Resolution**: Change width and height variables

## Notes

- The demo uses software rasterization by default (no WebGPU required)
- To enable WebGPU hardware acceleration, ensure WebGPU is properly configured
- The PBR shader implements Cook-Torrance microfacet BRDF
- Ambient lighting is set to 0.15 for subtle fill in shadowed areas

## See Also

- `render_triangle.cpp` - Simple triangle rendering example
- [ANARI Specification](https://www.khronos.org/anari/) - Official ANARI documentation
- WebGPU PBR implementation in `material/Material.cpp`

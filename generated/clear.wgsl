// Generated from shaders/clear.slang with its default specializations
// by vtkSlang (Slang 2026.13.1). Do not edit; regenerate with the
// anari_webgpu_update_wgsl target.

struct ClearUniforms_std140_0
{
    @align(16) bgColor_0 : vec4<f32>,
    @align(16) width_0 : u32,
    @align(4) height_0 : u32,
    @align(8) _pad_0 : vec2<u32>,
};

@binding(2) @group(0) var<uniform> params_0 : ClearUniforms_std140_0;
@binding(0) @group(0) var<storage, read_write> colorBuf_0 : array<vec4<f32>>;

@binding(1) @group(0) var<storage, read_write> depthBuf_0 : array<f32>;

@compute
@workgroup_size(256, 1, 1)
fn main(@builtin(global_invocation_id) gid_0 : vec3<u32>)
{
    var idx_0 : u32 = gid_0.x;
    if(idx_0 >= (params_0.width_0 * params_0.height_0))
    {
        return;
    }
    colorBuf_0[idx_0] = params_0.bgColor_0;
    depthBuf_0[idx_0] = 1.0f;
    return;
}


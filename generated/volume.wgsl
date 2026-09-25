// Generated from shaders/volume.slang with its default specializations
// by vtkSlang (Slang 2026.13.1). Do not edit; regenerate with the
// anari_webgpu_update_wgsl target.

struct _MatrixStorage_float4x4_ColMajorstd140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct VolumeUniforms_std140_0
{
    @align(16) viewInv_0 : _MatrixStorage_float4x4_ColMajorstd140_0,
    @align(16) projInv_0 : _MatrixStorage_float4x4_ColMajorstd140_0,
    @align(16) volumeMin_0 : vec3<f32>,
    @align(4) stepSize_0 : f32,
    @align(16) volumeMax_0 : vec3<f32>,
    @align(4) densityScale_0 : f32,
    @align(16) dims_0 : vec3<u32>,
    @align(4) tfSize_0 : u32,
    @align(16) valueRangeMin_0 : f32,
    @align(4) valueRangeMax_0 : f32,
    @align(8) width_0 : u32,
    @align(4) height_0 : u32,
};

@binding(2) @group(0) var<uniform> params_0 : VolumeUniforms_std140_0;
@binding(3) @group(0) var<storage, read> volData_0 : array<f32>;

@binding(4) @group(0) var<storage, read> tfTable_0 : array<vec4<f32>>;

@binding(0) @group(0) var<storage, read_write> colorBuf_0 : array<vec4<f32>>;

fn IntersectAABB_0( rayOrigin_0 : vec3<f32>,  rayDir_0 : vec3<f32>,  boxMin_0 : vec3<f32>,  boxMax_0 : vec3<f32>) -> vec2<f32>
{
    var invDir_0 : vec3<f32> = vec3<f32>(1.0f) / rayDir_0;
    var t0_0 : vec3<f32> = (boxMin_0 - rayOrigin_0) * invDir_0;
    var t1_0 : vec3<f32> = (boxMax_0 - rayOrigin_0) * invDir_0;
    var tSmall_0 : vec3<f32> = min(t0_0, t1_0);
    var tLarge_0 : vec3<f32> = max(t0_0, t1_0);
    return vec2<f32>(max(max(tSmall_0.x, tSmall_0.y), tSmall_0.z), min(min(tLarge_0.x, tLarge_0.y), tLarge_0.z));
}

fn SampleVolumeTrilinear_0( _S1 : vec3<u32>,  _S2 : vec3<f32>) -> f32
{
    var _S3 : bool;
    if((any((_S2 < vec3<f32>(0.0f, 0.0f, 0.0f)))))
    {
        _S3 = true;
    }
    else
    {
        _S3 = (any((_S2 > vec3<f32>(1.0f, 1.0f, 1.0f))));
    }
    if(_S3)
    {
        return 0.0f;
    }
    const _S4 : vec3<u32> = vec3<u32>(u32(1), u32(1), u32(1));
    var _S5 : vec3<u32> = _S1 - _S4;
    var coord_0 : vec3<f32> = _S2 * vec3<f32>(_S5);
    var _S6 : vec3<u32> = vec3<u32>(coord_0);
    var c1_0 : vec3<u32> = min(_S6 + _S4, _S5);
    var fr_0 : vec3<f32> = fract(coord_0);
    var _S7 : u32 = _S1.x;
    var dxy_0 : u32 = _S7 * _S1.y;
    var _S8 : u32 = _S6.x;
    var _S9 : u32 = _S6.y * _S7;
    var _S10 : u32 = _S8 + _S9;
    var _S11 : u32 = _S6.z * dxy_0;
    var _S12 : u32 = c1_0.x;
    var _S13 : u32 = _S12 + _S9;
    var _S14 : u32 = c1_0.y * _S7;
    var _S15 : u32 = _S8 + _S14;
    var _S16 : u32 = _S12 + _S14;
    var _S17 : u32 = c1_0.z * dxy_0;
    var _S18 : f32 = fr_0.x;
    var _S19 : f32 = fr_0.y;
    return mix(mix(mix(volData_0[_S10 + _S11], volData_0[_S13 + _S11], _S18), mix(volData_0[_S15 + _S11], volData_0[_S16 + _S11], _S18), _S19), mix(mix(volData_0[_S10 + _S17], volData_0[_S13 + _S17], _S18), mix(volData_0[_S15 + _S17], volData_0[_S16 + _S17], _S18), _S19), fr_0.z);
}

fn sampleVolume_0( worldPos_0 : vec3<f32>) -> f32
{
    return SampleVolumeTrilinear_0(params_0.dims_0, (worldPos_0 - params_0.volumeMin_0) / (params_0.volumeMax_0 - params_0.volumeMin_0));
}

fn CompositeFrontToBack_0( accColor_0 : ptr<function, vec3<f32>>,  accAlpha_0 : ptr<function, f32>,  sampleRGBA_0 : vec4<f32>,  stepAlpha_0 : f32)
{
    var sa_0 : f32 = clamp(stepAlpha_0, 0.0f, 1.0f);
    (*accColor_0) = (*accColor_0) + vec3<f32>((1.0f - (*accAlpha_0))) * sampleRGBA_0.xyz * vec3<f32>(sa_0);
    (*accAlpha_0) = (*accAlpha_0) + (1.0f - (*accAlpha_0)) * sa_0;
    return;
}

fn SampleTransferFunction_0( _S20 : u32,  _S21 : f32) -> vec4<f32>
{
    return tfTable_0[u32(clamp(_S21, 0.0f, 1.0f) * f32(_S20 - u32(1)))];
}

fn ApplyTransferFunction_0( _S22 : u32,  _S23 : f32,  _S24 : f32,  _S25 : f32) -> vec4<f32>
{
    return SampleTransferFunction_0(_S22, (_S23 - _S24) / (_S25 - _S24 + 0.00009999999747379f));
}

@compute
@workgroup_size(256, 1, 1)
fn vol_main(@builtin(global_invocation_id) gid_0 : vec3<u32>)
{
    var pixIdx_0 : u32 = gid_0.x;
    if(pixIdx_0 >= (params_0.width_0 * params_0.height_0))
    {
        return;
    }
    var px_0 : u32 = pixIdx_0 % params_0.width_0;
    var py_0 : u32 = pixIdx_0 / params_0.width_0;
    var viewPos4_0 : vec4<f32> = (((vec4<f32>((f32(px_0) + 0.5f) / f32(params_0.width_0) * 2.0f - 1.0f, - ((f32(py_0) + 0.5f) / f32(params_0.height_0) * 2.0f - 1.0f), -1.0f, 1.0f)) * (mat4x4<f32>(params_0.projInv_0.data_0[i32(0)][i32(0)], params_0.projInv_0.data_0[i32(1)][i32(0)], params_0.projInv_0.data_0[i32(2)][i32(0)], params_0.projInv_0.data_0[i32(3)][i32(0)], params_0.projInv_0.data_0[i32(0)][i32(1)], params_0.projInv_0.data_0[i32(1)][i32(1)], params_0.projInv_0.data_0[i32(2)][i32(1)], params_0.projInv_0.data_0[i32(3)][i32(1)], params_0.projInv_0.data_0[i32(0)][i32(2)], params_0.projInv_0.data_0[i32(1)][i32(2)], params_0.projInv_0.data_0[i32(2)][i32(2)], params_0.projInv_0.data_0[i32(3)][i32(2)], params_0.projInv_0.data_0[i32(0)][i32(3)], params_0.projInv_0.data_0[i32(1)][i32(3)], params_0.projInv_0.data_0[i32(2)][i32(3)], params_0.projInv_0.data_0[i32(3)][i32(3)]))));
    var _S26 : f32 = viewPos4_0.w;
    var worldDir_0 : vec3<f32> = normalize((((vec4<f32>(normalize(vec3<f32>(viewPos4_0.x / _S26, viewPos4_0.y / _S26, -1.0f)), 0.0f)) * (mat4x4<f32>(params_0.viewInv_0.data_0[i32(0)][i32(0)], params_0.viewInv_0.data_0[i32(1)][i32(0)], params_0.viewInv_0.data_0[i32(2)][i32(0)], params_0.viewInv_0.data_0[i32(3)][i32(0)], params_0.viewInv_0.data_0[i32(0)][i32(1)], params_0.viewInv_0.data_0[i32(1)][i32(1)], params_0.viewInv_0.data_0[i32(2)][i32(1)], params_0.viewInv_0.data_0[i32(3)][i32(1)], params_0.viewInv_0.data_0[i32(0)][i32(2)], params_0.viewInv_0.data_0[i32(1)][i32(2)], params_0.viewInv_0.data_0[i32(2)][i32(2)], params_0.viewInv_0.data_0[i32(3)][i32(2)], params_0.viewInv_0.data_0[i32(0)][i32(3)], params_0.viewInv_0.data_0[i32(1)][i32(3)], params_0.viewInv_0.data_0[i32(2)][i32(3)], params_0.viewInv_0.data_0[i32(3)][i32(3)])))).xyz);
    var worldOri_0 : vec3<f32> = (((vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f)) * (mat4x4<f32>(params_0.viewInv_0.data_0[i32(0)][i32(0)], params_0.viewInv_0.data_0[i32(1)][i32(0)], params_0.viewInv_0.data_0[i32(2)][i32(0)], params_0.viewInv_0.data_0[i32(3)][i32(0)], params_0.viewInv_0.data_0[i32(0)][i32(1)], params_0.viewInv_0.data_0[i32(1)][i32(1)], params_0.viewInv_0.data_0[i32(2)][i32(1)], params_0.viewInv_0.data_0[i32(3)][i32(1)], params_0.viewInv_0.data_0[i32(0)][i32(2)], params_0.viewInv_0.data_0[i32(1)][i32(2)], params_0.viewInv_0.data_0[i32(2)][i32(2)], params_0.viewInv_0.data_0[i32(3)][i32(2)], params_0.viewInv_0.data_0[i32(0)][i32(3)], params_0.viewInv_0.data_0[i32(1)][i32(3)], params_0.viewInv_0.data_0[i32(2)][i32(3)], params_0.viewInv_0.data_0[i32(3)][i32(3)])))).xyz;
    var tHit_0 : vec2<f32> = IntersectAABB_0(worldOri_0, worldDir_0, params_0.volumeMin_0, params_0.volumeMax_0);
    var _S27 : f32 = tHit_0.x;
    var _S28 : f32 = tHit_0.y;
    var _S29 : bool;
    if(_S27 > _S28)
    {
        _S29 = true;
    }
    else
    {
        _S29 = _S28 < 0.0f;
    }
    if(_S29)
    {
        return;
    }
    var _S30 : f32 = max(_S27, 0.0f);
    var accColor_1 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    var accAlpha_1 : f32 = 0.0f;
    var _S31 : i32 = i32((_S28 - _S30) / params_0.stepSize_0) + i32(1);
    var i_0 : i32 = i32(0);
    for(;;)
    {
        if(i_0 < _S31)
        {
        }
        else
        {
            break;
        }
        var tCurr_0 : f32 = _S30 + f32(i_0) * params_0.stepSize_0;
        if(tCurr_0 > _S28)
        {
            break;
        }
        var _S32 : vec4<f32> = ApplyTransferFunction_0(params_0.tfSize_0, sampleVolume_0(worldOri_0 + vec3<f32>(tCurr_0) * worldDir_0), params_0.valueRangeMin_0, params_0.valueRangeMax_0);
        CompositeFrontToBack_0(&(accColor_1), &(accAlpha_1), _S32, clamp(_S32.w * params_0.densityScale_0 * params_0.stepSize_0, 0.0f, 1.0f));
        if(accAlpha_1 >= 0.99000000953674316f)
        {
            break;
        }
        i_0 = i_0 + i32(1);
    }
    if(accAlpha_1 < 0.00100000004749745f)
    {
        return;
    }
    colorBuf_0[pixIdx_0] = vec4<f32>(accColor_1 + colorBuf_0[pixIdx_0].xyz * vec3<f32>((1.0f - accAlpha_1)), min(colorBuf_0[pixIdx_0].w + accAlpha_1, 1.0f));
    return;
}


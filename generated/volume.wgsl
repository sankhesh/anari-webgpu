// Generated from shaders/volume.slang with its default specializations
// by SlangKit (Slang 2026.13.1). Do not edit; regenerate with the
// anari_webgpu_update_wgsl target.

struct _MatrixStorage_float4x4_ColMajorstd140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct VolumeUniforms_std140_0
{
    @align(16) inverseViewProjection_0 : _MatrixStorage_float4x4_ColMajorstd140_0,
    @align(16) viewProjection_0 : _MatrixStorage_float4x4_ColMajorstd140_0,
    @align(16) worldToTexture_0 : _MatrixStorage_float4x4_ColMajorstd140_0,
    @align(16) dims_0 : vec4<f32>,
    @align(16) spacing_0 : vec4<f32>,
    @align(16) cellDims_0 : vec4<f32>,
    @align(16) eye_0 : vec4<f32>,
    @align(16) valueMin_0 : f32,
    @align(4) valueScale_0 : f32,
    @align(8) baseStep_0 : f32,
    @align(4) unitDistance_0 : f32,
    @align(16) width_0 : u32,
    @align(4) height_0 : u32,
    @align(8) colorCount_0 : u32,
    @align(4) opacityCount_0 : u32,
    @align(16) maxSteps_0 : u32,
    @align(4) nearest_0 : u32,
    @align(8) _pad_0 : u32,
    @align(4) cellCount_0 : u32,
};

@binding(2) @group(0) var<uniform> params_0 : VolumeUniforms_std140_0;
@binding(7) @group(0) var<storage, read> cellRanges_0 : array<vec2<f32>>;

@binding(10) @group(0) var<storage, read_write> classified_0 : array<vec2<f32>>;

@binding(8) @group(0) var<storage, read> surfaceDepth_0 : array<u32>;

@binding(1) @group(0) var<storage, read_write> depthBuf_0 : array<u32>;

@binding(0) @group(0) var<storage, read_write> colorBuf_0 : array<vec4<f32>>;

@binding(3) @group(0) var<storage, read> voxels_0 : array<f32>;

@binding(4) @group(0) var<storage, read> colorTable_0 : array<vec4<f32>>;

@binding(5) @group(0) var<storage, read> opacityTable_0 : array<f32>;

@binding(6) @group(0) var<storage, read> cells_0 : array<vec2<f32>>;

@binding(9) @group(0) var<storage, read> extremeTable_0 : array<vec2<f32>>;

struct VolumeGrid_0
{
     dims_1 : vec3<f32>,
     spacing_1 : vec3<f32>,
     cellData_0 : bool,
};

fn VolumeGrid_x24init_0( dims_2 : vec3<f32>,  spacing_2 : vec3<f32>,  cellData_1 : bool) -> VolumeGrid_0
{
    var _S1 : VolumeGrid_0;
    _S1.dims_1 = dims_2;
    _S1.spacing_1 = spacing_2;
    _S1.cellData_0 = cellData_1;
    return _S1;
}

fn FieldSampler_grid_0() -> VolumeGrid_0
{
    var _S2 : vec3<f32> = vec3<f32>(1.0f);
    var g_0 : VolumeGrid_0 = VolumeGrid_x24init_0(_S2, _S2, false);
    g_0.dims_1 = params_0.dims_0.xyz;
    g_0.spacing_1 = params_0.spacing_0.xyz;
    return g_0;
}

fn VolumeGrid_extent_0( this_0 : VolumeGrid_0) -> vec3<f32>
{
    var _S3 : vec3<f32>;
    if(this_0.cellData_0)
    {
        _S3 = this_0.dims_1;
    }
    else
    {
        _S3 = max(this_0.dims_1 - vec3<f32>(1.0f), vec3<f32>(1.0f));
    }
    return _S3;
}

fn VolumeGrid_sampleStep_0( this_1 : VolumeGrid_0) -> vec3<f32>
{
    return vec3<f32>(1.0f) / VolumeGrid_extent_0(this_1);
}

fn VolumeGrid_voxelOf_0( this_2 : VolumeGrid_0,  texCoord_0 : vec3<f32>) -> vec3<f32>
{
    var _S4 : vec3<f32> = vec3<f32>(0.0f);
    var _S5 : vec3<f32> = vec3<f32>(1.0f);
    var clamped_0 : vec3<f32> = clamp(texCoord_0, _S4, _S5);
    if(this_2.cellData_0)
    {
        return clamp(clamped_0 * this_2.dims_1 - vec3<f32>(0.5f), _S4, max(this_2.dims_1 - vec3<f32>(1.0f), _S4));
    }
    return clamped_0 * max(this_2.dims_1 - vec3<f32>(1.0f), _S5);
}

fn FieldSampler_locate_0( texCoord_1 : vec3<f32>) -> vec3<f32>
{
    return VolumeGrid_voxelOf_0(FieldSampler_grid_0(), texCoord_1);
}

fn voxel_0( i_0 : vec3<u32>) -> f32
{
    var _S6 : vec3<u32> = vec3<u32>(params_0.dims_0.xyz);
    return voxels_0[(i_0.z * _S6.y + i_0.y) * _S6.x + i_0.x];
}

fn FieldSampler_sample_0( v_0 : vec3<f32>) -> vec4<f32>
{
    var _S7 : vec3<u32> = vec3<u32>(u32(1));
    var last_0 : vec3<u32> = vec3<u32>(params_0.dims_0.xyz) - _S7;
    var _S8 : vec3<f32> = clamp(v_0, vec3<f32>(0.0f), vec3<f32>(last_0));
    if((params_0.nearest_0) != u32(0))
    {
        return vec4<f32>(voxel_0(min(vec3<u32>(round(_S8)), last_0)), 0.0f, 0.0f, 0.0f);
    }
    var i0_0 : vec3<u32> = min(vec3<u32>(_S8), last_0);
    var i1_0 : vec3<u32> = min(i0_0 + _S7, last_0);
    var f_0 : vec3<f32> = _S8 - vec3<f32>(i0_0);
    var _S9 : u32 = i1_0.x;
    var _S10 : u32 = i0_0.y;
    var _S11 : u32 = i0_0.z;
    var _S12 : f32 = f_0.x;
    var _S13 : u32 = i0_0.x;
    var _S14 : u32 = i1_0.y;
    var _S15 : u32 = i1_0.z;
    var _S16 : f32 = f_0.y;
    return vec4<f32>(mix(mix(mix(voxel_0(i0_0), voxel_0(vec3<u32>(_S9, _S10, _S11)), _S12), mix(voxel_0(vec3<u32>(_S13, _S14, _S11)), voxel_0(vec3<u32>(_S9, _S14, _S11)), _S12), _S16), mix(mix(voxel_0(vec3<u32>(_S13, _S10, _S15)), voxel_0(vec3<u32>(_S9, _S10, _S15)), _S12), mix(voxel_0(vec3<u32>(_S13, _S14, _S15)), voxel_0(i1_0), _S12), _S16), f_0.z), 0.0f, 0.0f, 0.0f);
}

fn IVolumeSampler_sampleNearx24defaultImpl_0( c_0 : vec3<f32>,  texCoord_2 : vec3<f32>) -> vec4<f32>
{
    return FieldSampler_sample_0(FieldSampler_locate_0(texCoord_2));
}

fn centralDifferences_0( c_1 : vec3<f32>,  texCoord_3 : vec3<f32>,  component_0 : u32) -> vec3<f32>
{
    var step_0 : vec3<f32> = VolumeGrid_sampleStep_0(FieldSampler_grid_0());
    var dx_0 : vec3<f32> = vec3<f32>(step_0.x, 0.0f, 0.0f);
    var dy_0 : vec3<f32> = vec3<f32>(0.0f, step_0.y, 0.0f);
    var dz_0 : vec3<f32> = vec3<f32>(0.0f, 0.0f, step_0.z);
    return vec3<f32>(IVolumeSampler_sampleNearx24defaultImpl_0(c_1, texCoord_3 + dx_0)[component_0] - IVolumeSampler_sampleNearx24defaultImpl_0(c_1, texCoord_3 - dx_0)[component_0], IVolumeSampler_sampleNearx24defaultImpl_0(c_1, texCoord_3 + dy_0)[component_0] - IVolumeSampler_sampleNearx24defaultImpl_0(c_1, texCoord_3 - dy_0)[component_0], IVolumeSampler_sampleNearx24defaultImpl_0(c_1, texCoord_3 + dz_0)[component_0] - IVolumeSampler_sampleNearx24defaultImpl_0(c_1, texCoord_3 - dz_0)[component_0]);
}

fn FieldTransfer_normalize_0( component_1 : u32,  value_0 : f32) -> f32
{
    return clamp((value_0 - params_0.valueMin_0) * params_0.valueScale_0, 0.0f, 1.0f);
}

fn opacityAt_0( t_0 : f32) -> f32
{
    var last_1 : u32 = params_0.opacityCount_0 - u32(1);
    var position_0 : f32 = t_0 * f32(last_1);
    var _S17 : u32 = min(u32(position_0), last_1);
    return mix(opacityTable_0[_S17], opacityTable_0[min(_S17 + u32(1), last_1)], position_0 - f32(_S17));
}

fn colorAt_0( t_1 : f32) -> vec4<f32>
{
    var last_2 : u32 = params_0.colorCount_0 - u32(1);
    var position_1 : f32 = t_1 * f32(last_2);
    var _S18 : u32 = min(u32(position_1), last_2);
    return mix(colorTable_0[_S18], colorTable_0[min(_S18 + u32(1), last_2)], vec4<f32>((position_1 - f32(_S18))));
}

fn FieldTransfer_opacity_0( component_2 : u32,  normalized_0 : f32) -> f32
{
    return opacityAt_0(normalized_0) * colorAt_0(normalized_0).w;
}

fn opacityNear_0( c_2 : vec3<f32>,  texCoord_4 : vec3<f32>,  component_3 : u32) -> f32
{
    return FieldTransfer_opacity_0(component_3, FieldTransfer_normalize_0(component_3, IVolumeSampler_sampleNearx24defaultImpl_0(c_2, texCoord_4)[component_3]));
}

fn IVolumeHooks_isoValueCountx24defaultImpl_0() -> u32
{
    return u32(0);
}

fn IVolumeHooks_isoValuex24defaultImpl_0( i_1 : u32) -> f32
{
    return 0.0f;
}

fn isoBoundary_0( index_0 : i32) -> f32
{
    if(index_0 <= i32(0))
    {
        return -3.39999995214436425e+38f;
    }
    if(index_0 > i32(IVolumeHooks_isoValueCountx24defaultImpl_0()))
    {
        return 3.39999995214436425e+38f;
    }
    return IVolumeHooks_isoValuex24defaultImpl_0(u32(index_0 - i32(1)));
}

fn isoIntervalOf_0( value_1 : f32) -> i32
{
    var index_1 : i32 = i32(0);
    var i_2 : u32 = u32(0);
    for(;;)
    {
        if(i_2 < (IVolumeHooks_isoValueCountx24defaultImpl_0()))
        {
        }
        else
        {
            break;
        }
        if(value_1 >= (IVolumeHooks_isoValuex24defaultImpl_0(i_2)))
        {
            index_1 = i32(i_2) + i32(1);
        }
        i_2 = i_2 + u32(1);
    }
    return index_1;
}

fn cellCanMove_0( component_4 : u32,  range_0 : vec2<f32>,  maximum_0 : bool,  value_2 : f32,  bound_0 : f32) -> bool
{
    var _S19 : f32 = range_0.x;
    var _S20 : f32 = range_0.y;
    if(_S19 > _S20)
    {
        return true;
    }
    var _S21 : bool;
    if(maximum_0)
    {
        var _S22 : f32 = FieldTransfer_normalize_0(component_4, _S20);
        if(_S22 > value_2)
        {
            _S21 = _S22 >= bound_0;
        }
        else
        {
            _S21 = false;
        }
        return _S21;
    }
    var _S23 : f32 = FieldTransfer_normalize_0(component_4, _S19);
    if(_S23 < value_2)
    {
        _S21 = _S23 <= bound_0;
    }
    else
    {
        _S21 = false;
    }
    return _S21;
}

fn opacityGradient_0( c_3 : vec3<f32>,  texCoord_5 : vec3<f32>,  component_5 : u32) -> vec3<f32>
{
    var step_1 : vec3<f32> = VolumeGrid_sampleStep_0(FieldSampler_grid_0());
    var dx_1 : vec3<f32> = vec3<f32>(step_1.x, 0.0f, 0.0f);
    var dy_1 : vec3<f32> = vec3<f32>(0.0f, step_1.y, 0.0f);
    var dz_1 : vec3<f32> = vec3<f32>(0.0f, 0.0f, step_1.z);
    return vec3<f32>(opacityNear_0(c_3, texCoord_5 + dx_1, component_5) - opacityNear_0(c_3, texCoord_5 - dx_1, component_5), opacityNear_0(c_3, texCoord_5 + dy_1, component_5) - opacityNear_0(c_3, texCoord_5 - dy_1, component_5), opacityNear_0(c_3, texCoord_5 + dz_1, component_5) - opacityNear_0(c_3, texCoord_5 - dz_1, component_5));
}

fn FieldTransfer_color_0( component_6 : u32,  normalized_1 : f32) -> vec3<f32>
{
    return colorAt_0(normalized_1).xyz;
}

fn IVolumeHooks_shadesx24defaultImpl_0() -> bool
{
    return false;
}

fn IVolumeSampler_gradientx24defaultImpl_0( c_4 : vec3<f32>,  texCoord_6 : vec3<f32>,  component_7 : u32) -> vec3<f32>
{
    return centralDifferences_0(c_4, texCoord_6, component_7);
}

fn IVolumeHooks_shadex24defaultImpl_0( color_0 : vec3<f32>,  texCoord_7 : vec3<f32>,  gradient_0 : vec3<f32>) -> vec3<f32>
{
    return color_0;
}

fn classifyAt_0( c_5 : vec3<f32>,  texCoord_8 : vec3<f32>,  component_8 : u32,  normalized_2 : f32,  normalFromOpacity_0 : bool) -> vec4<f32>
{
    var value_3 : f32 = clamp(normalized_2, 0.0f, 1.0f);
    var _S24 : f32 = FieldTransfer_opacity_0(component_8, value_3);
    var _S25 : vec3<f32> = FieldTransfer_color_0(component_8, value_3);
    var _S26 : bool;
    if(IVolumeHooks_shadesx24defaultImpl_0())
    {
        _S26 = _S24 > 0.0f;
    }
    else
    {
        _S26 = false;
    }
    var color_1 : vec3<f32>;
    if(_S26)
    {
        if(normalFromOpacity_0)
        {
            color_1 = opacityGradient_0(c_5, texCoord_8, component_8);
        }
        else
        {
            color_1 = IVolumeSampler_gradientx24defaultImpl_0(c_5, texCoord_8, component_8);
        }
        color_1 = IVolumeHooks_shadex24defaultImpl_0(_S25, texCoord_8, color_1);
    }
    else
    {
        color_1 = _S25;
    }
    return vec4<f32>(color_1, _S24);
}

struct VolumeRay_0
{
     origin_0 : vec3<f32>,
     direction_0 : vec3<f32>,
     tNear_0 : f32,
     tFar_0 : f32,
};

fn FieldCells_cellSize_0() -> f32
{
    return params_0.cellDims_0.w;
}

fn IVolumeHooks_hidesx24defaultImpl_0() -> bool
{
    return false;
}

fn IVolumeSampler_blockSizex24defaultImpl_0() -> f32
{
    return 0.0f;
}

fn VolumeGrid_shift_0( this_3 : VolumeGrid_0) -> vec3<f32>
{
    var _S27 : vec3<f32>;
    if(this_3.cellData_0)
    {
        _S27 = vec3<f32>(0.5f);
    }
    else
    {
        _S27 = vec3<f32>(0.0f);
    }
    return _S27;
}

fn blockExit_0( g_1 : VolumeGrid_0,  origin_1 : vec3<f32>,  direction_1 : vec3<f32>,  t_2 : f32,  size_0 : f32) -> f32
{
    var extent_0 : vec3<f32> = VolumeGrid_extent_0(g_1);
    var shift_0 : vec3<f32> = VolumeGrid_shift_0(g_1);
    var _S28 : vec3<f32> = vec3<f32>(size_0);
    var block_0 : vec3<f32> = floor(((origin_1 + vec3<f32>(t_2) * direction_1) * extent_0 - shift_0) / _S28);
    var _S29 : vec3<f32> = vec3<f32>(1.0f);
    var inverse_0 : vec3<f32> = _S29 / direction_1;
    var tMax_0 : vec3<f32> = max(((block_0 * _S28 + shift_0) / extent_0 - origin_1) * inverse_0, (((block_0 + _S29) * _S28 + shift_0) / extent_0 - origin_1) * inverse_0);
    return max(min(min(tMax_0.x, tMax_0.y), tMax_0.z) - t_2, 0.0f);
}

fn FieldCells_index_0( voxel_1 : vec3<f32>) -> u32
{
    var cell_0 : vec3<f32> = clamp(floor(voxel_1 / vec3<f32>(params_0.cellDims_0.w)), vec3<f32>(0.0f), params_0.cellDims_0.xyz - vec3<f32>(1.0f));
    return u32((cell_0.z * params_0.cellDims_0.y + cell_0.y) * params_0.cellDims_0.x + cell_0.x);
}

fn FieldCells_range_0( voxel_2 : vec3<f32>) -> vec2<f32>
{
    return cellRanges_0[FieldCells_index_0(voxel_2)];
}

fn IVolumeSampler_presentx24defaultImpl_0( c_6 : vec3<f32>) -> bool
{
    return true;
}

fn projectionBound_0( ray_0 : VolumeRay_0,  component_9 : u32,  maximum_1 : bool,  tFrom_0 : f32,  tTo_0 : f32,  reach_0 : f32,  start_0 : f32) -> f32
{
    var _S30 : VolumeGrid_0 = FieldSampler_grid_0();
    var _S31 : f32 = FieldCells_cellSize_0();
    var _S32 : bool;
    if(_S31 <= 0.0f)
    {
        _S32 = true;
    }
    else
    {
        _S32 = IVolumeHooks_hidesx24defaultImpl_0();
    }
    if(_S32)
    {
        _S32 = true;
    }
    else
    {
        _S32 = _S30.cellData_0;
    }
    if(_S32)
    {
        return start_0;
    }
    var _S33 : f32 = IVolumeSampler_blockSizex24defaultImpl_0();
    if(_S33 > 0.0f)
    {
        _S32 = ((((_S33)) % ((_S31)))) != 0.0f;
    }
    else
    {
        _S32 = false;
    }
    if(_S32)
    {
        return start_0;
    }
    var t_3 : f32 = tFrom_0;
    var bound_1 : f32 = start_0;
    var visited_0 : u32 = u32(0);
    for(;;)
    {
        if(visited_0 < u32(1024))
        {
            _S32 = t_3 < tTo_0;
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
        }
        else
        {
            break;
        }
        var _S34 : f32 = min(t_3 + blockExit_0(_S30, ray_0.origin_0, ray_0.direction_0, t_3, _S31), tTo_0);
        if((_S34 - t_3) >= reach_0)
        {
            var texCoord_9 : vec3<f32> = ray_0.origin_0 + vec3<f32>((0.5f * (t_3 + _S34))) * ray_0.direction_0;
            var _S35 : vec2<f32> = FieldCells_range_0(VolumeGrid_voxelOf_0(_S30, texCoord_9));
            var _S36 : f32;
            if(maximum_1)
            {
                _S36 = _S35.x;
            }
            else
            {
                _S36 = _S35.y;
            }
            var _S37 : f32 = FieldTransfer_normalize_0(component_9, _S36);
            var _S38 : bool;
            var _S39 : bool;
            if((_S35.x) <= (_S35.y))
            {
                if(maximum_1)
                {
                    _S38 = _S37 > bound_1;
                }
                else
                {
                    _S38 = false;
                }
                if(_S38)
                {
                    _S39 = true;
                }
                else
                {
                    if(!maximum_1)
                    {
                        _S39 = _S37 < bound_1;
                    }
                    else
                    {
                        _S39 = false;
                    }
                }
                _S38 = _S39;
            }
            else
            {
                _S38 = false;
            }
            if(_S38)
            {
                _S39 = IVolumeSampler_presentx24defaultImpl_0(FieldSampler_locate_0(texCoord_9));
            }
            else
            {
                _S39 = false;
            }
            var bound_2 : f32;
            if(_S39)
            {
                bound_2 = _S37;
            }
            else
            {
                bound_2 = bound_1;
            }
            bound_1 = bound_2;
        }
        var _S40 : f32 = _S34 + max(9.99999997475242708e-07f, abs(_S34) * 0.00000999999974738f);
        var _S41 : u32 = visited_0 + u32(1);
        t_3 = _S40;
        visited_0 = _S41;
    }
    return bound_1;
}

fn IVolumeHooks_hiddenx24defaultImpl_0( texCoord_10 : vec3<f32>) -> bool
{
    return false;
}

fn ITransferFunction_gradientOpacityx24defaultImpl_0( component_10 : u32,  magnitude_0 : f32) -> f32
{
    return 1.0f;
}

fn ITransferFunction_hasGradientOpacityx24defaultImpl_0() -> bool
{
    return false;
}

struct VolumeResult_0
{
     color_2 : vec4<f32>,
     covered_0 : bool,
     hit_0 : bool,
     tHit_0 : f32,
     texCoordHit_0 : vec3<f32>,
     visible_0 : bool,
     tVisible_0 : f32,
     texCoordVisible_0 : vec3<f32>,
};

struct VolumeMarch_0
{
     blendMode_0 : u32,
     component_11 : u32,
     baseStep_1 : f32,
     unitDistance_1 : f32,
     maxOpacity_0 : f32,
     maxSteps_1 : u32,
     jitter_0 : f32,
     adaptive_0 : bool,
     minStepFactor_0 : f32,
     maxStepFactor_0 : f32,
     skipEmpty_0 : bool,
     emptyThreshold_0 : f32,
     normalFromOpacity_1 : bool,
     averageMinimum_0 : f32,
     averageMaximum_0 : f32,
     opaqueAlpha_0 : f32,
};

fn VolumeResult_x24init_0( color_3 : vec4<f32>,  covered_1 : bool,  hit_1 : bool,  tHit_1 : f32,  texCoordHit_1 : vec3<f32>,  visible_1 : bool,  tVisible_1 : f32,  texCoordVisible_1 : vec3<f32>) -> VolumeResult_0
{
    var _S42 : VolumeResult_0;
    _S42.color_2 = color_3;
    _S42.covered_0 = covered_1;
    _S42.hit_0 = hit_1;
    _S42.tHit_0 = tHit_1;
    _S42.texCoordHit_0 = texCoordHit_1;
    _S42.visible_0 = visible_1;
    _S42.tVisible_0 = tVisible_1;
    _S42.texCoordVisible_0 = texCoordVisible_1;
    return _S42;
}

fn FieldCells_cell_0( voxel_3 : vec3<f32>) -> vec2<f32>
{
    return cells_0[FieldCells_index_0(voxel_3)];
}

fn adaptiveStep_0( baseStep_2 : f32,  detail_0 : f32,  minStepFactor_1 : f32,  maxStepFactor_1 : f32) -> f32
{
    return baseStep_2 * mix(maxStepFactor_1, minStepFactor_1, detail_0);
}

fn advancePast_0( t_4 : f32,  skipTo_0 : f32,  pace_0 : f32,  coarsest_0 : f32) -> f32
{
    if(pace_0 < 3.39999995214436425e+38f)
    {
        var _S43 : f32 = max(pace_0, coarsest_0);
        return t_4 + _S43 * max(1.0f, ceil((skipTo_0 - t_4) / _S43));
    }
    return max(skipTo_0, t_4 + max(9.99999997475242708e-07f, abs(t_4) * 0.00000999999974738f));
}

fn compositeFrontToBack_0( color_4 : ptr<function, vec3<f32>>,  alpha_0 : ptr<function, f32>,  sampleColor_0 : vec3<f32>,  opacity_0 : f32)
{
    var _S44 : f32 = (1.0f - (*alpha_0)) * opacity_0;
    (*color_4) = (*color_4) + vec3<f32>(_S44) * sampleColor_0;
    (*alpha_0) = (*alpha_0) + _S44;
    return;
}

fn correctOpacity_0( opacity_1 : f32,  step_2 : f32,  unitDistance_2 : f32,  maxOpacity_1 : f32) -> f32
{
    var _S45 : f32 = min(opacity_1, maxOpacity_1);
    var _S46 : f32;
    if(_S45 >= 1.0f)
    {
        _S46 = 1.0f;
    }
    else
    {
        _S46 = 1.0f - pow(1.0f - _S45, step_2 / unitDistance_2);
    }
    return _S46;
}

fn gradientMagnitude_0( gradient_1 : vec3<f32>,  spacing_3 : vec3<f32>) -> f32
{
    return length(gradient_1 / (spacing_3 * vec3<f32>(2.0f) / vec3<f32>(((spacing_3.x + spacing_3.y + spacing_3.z) / 3.0f))));
}

fn marchVolume_0( ray_1 : VolumeRay_0,  m_0 : VolumeMarch_0) -> VolumeResult_0
{
    var value_4 : f32;
    var _S47 : vec3<f32> = vec3<f32>(0.0f);
    var r_0 : VolumeResult_0 = VolumeResult_x24init_0(vec4<f32>(0.0f), false, false, 0.0f, _S47, false, 0.0f, _S47);
    if((ray_1.tNear_0) >= (ray_1.tFar_0))
    {
        return r_0;
    }
    var _S48 : VolumeGrid_0 = FieldSampler_grid_0();
    var compositing_0 : bool = (m_0.blendMode_0) == u32(0);
    var isosurface_0 : bool = (m_0.blendMode_0) == u32(5);
    var maximum_2 : bool = (m_0.blendMode_0) == u32(1);
    var minimum_0 : bool = (m_0.blendMode_0) == u32(2);
    var _S49 : f32 = FieldCells_cellSize_0();
    var useCells_0 : bool = _S49 > 0.0f;
    var adaptive_1 : bool;
    if(m_0.adaptive_0)
    {
        adaptive_1 = useCells_0;
    }
    else
    {
        adaptive_1 = false;
    }
    var _S50 : bool;
    if(compositing_0)
    {
        _S50 = m_0.skipEmpty_0;
    }
    else
    {
        _S50 = false;
    }
    if(_S50)
    {
        _S50 = useCells_0;
    }
    else
    {
        _S50 = false;
    }
    var _S51 : bool = !adaptive_1;
    var skipSettled_0 : bool;
    if(_S51)
    {
        skipSettled_0 = m_0.skipEmpty_0;
    }
    else
    {
        skipSettled_0 = false;
    }
    if(skipSettled_0)
    {
        skipSettled_0 = useCells_0;
    }
    else
    {
        skipSettled_0 = false;
    }
    if(skipSettled_0)
    {
        if(maximum_2)
        {
            skipSettled_0 = true;
        }
        else
        {
            skipSettled_0 = minimum_0;
        }
    }
    else
    {
        skipSettled_0 = false;
    }
    var _S52 : bool = IVolumeHooks_shadesx24defaultImpl_0();
    var _S53 : bool = ITransferFunction_hasGradientOpacityx24defaultImpl_0();
    var coarsest_1 : f32 = (ray_1.tFar_0 - ray_1.tNear_0) / f32(max(m_0.maxSteps_1, u32(2)) - u32(1));
    var color_5 : vec3<f32> = _S47;
    var alpha_1 : f32 = 0.0f;
    var value_5 : f32;
    if(minimum_0)
    {
        value_5 = 3.39999995214436425e+38f;
    }
    else
    {
        value_5 = -3.39999995214436425e+38f;
    }
    var bound_3 : f32;
    if(skipSettled_0)
    {
        bound_3 = projectionBound_0(ray_1, m_0.component_11, maximum_2, ray_1.tNear_0, ray_1.tFar_0, 2.0f * max(coarsest_1, m_0.baseStep_1), value_5);
    }
    else
    {
        bound_3 = value_5;
    }
    var _S54 : f32 = ray_1.tNear_0 + m_0.jitter_0;
    var isoSeen_0 : bool = false;
    var isoInterval_0 : i32 = i32(0);
    var t_5 : f32 = _S54;
    var steps_0 : u32 = u32(0);
    var count_0 : f32 = 0.0f;
    for(;;)
    {
        var _S55 : bool;
        if(t_5 < (ray_1.tFar_0))
        {
            _S55 = steps_0 < (m_0.maxSteps_1);
        }
        else
        {
            _S55 = false;
        }
        if(_S55)
        {
        }
        else
        {
            break;
        }
        var _S56 : u32 = steps_0 + u32(1);
        var texCoord_11 : vec3<f32> = ray_1.origin_0 + vec3<f32>(t_5) * ray_1.direction_0;
        var voxel_4 : vec3<f32> = VolumeGrid_voxelOf_0(_S48, texCoord_11);
        var _S57 : vec3<f32> = FieldSampler_locate_0(texCoord_11);
        var crossed_0 : bool;
        var skipTo_1 : f32;
        var pace_1 : f32;
        var step_3 : f32;
        if(!IVolumeSampler_presentx24defaultImpl_0(_S57))
        {
            var _S58 : f32 = t_5 + blockExit_0(_S48, ray_1.origin_0, ray_1.direction_0, t_5, IVolumeSampler_blockSizex24defaultImpl_0());
            if(_S51)
            {
                pace_1 = m_0.baseStep_1;
            }
            else
            {
                pace_1 = 3.39999995214436425e+38f;
            }
            skipTo_1 = _S58;
            step_3 = m_0.baseStep_1;
        }
        else
        {
            if(adaptive_1)
            {
                crossed_0 = true;
            }
            else
            {
                crossed_0 = _S50;
            }
            var _S59 : bool;
            if(crossed_0)
            {
                var _S60 : vec2<f32> = FieldCells_cell_0(voxel_4);
                if(_S50)
                {
                    _S59 = (_S60.x) <= (m_0.emptyThreshold_0);
                }
                else
                {
                    _S59 = false;
                }
                if(_S59)
                {
                    var _S61 : f32 = t_5 + blockExit_0(_S48, ray_1.origin_0, ray_1.direction_0, t_5, _S49);
                    if(_S51)
                    {
                        pace_1 = m_0.baseStep_1;
                    }
                    else
                    {
                        pace_1 = 3.39999995214436425e+38f;
                    }
                    skipTo_1 = _S61;
                    step_3 = m_0.baseStep_1;
                }
                else
                {
                    if(adaptive_1)
                    {
                        skipTo_1 = adaptiveStep_0(m_0.baseStep_1, _S60.y, m_0.minStepFactor_0, m_0.maxStepFactor_0);
                    }
                    else
                    {
                        skipTo_1 = m_0.baseStep_1;
                    }
                    var _S62 : f32 = skipTo_1;
                    skipTo_1 = 3.39999995214436425e+38f;
                    pace_1 = 3.39999995214436425e+38f;
                    step_3 = _S62;
                }
            }
            else
            {
                if(skipSettled_0)
                {
                    _S59 = !cellCanMove_0(m_0.component_11, FieldCells_range_0(voxel_4), maximum_2, value_5, bound_3);
                }
                else
                {
                    _S59 = false;
                }
                if(_S59)
                {
                    skipTo_1 = t_5 + blockExit_0(_S48, ray_1.origin_0, ray_1.direction_0, t_5, _S49);
                    pace_1 = m_0.baseStep_1;
                }
                else
                {
                    skipTo_1 = 3.39999995214436425e+38f;
                    pace_1 = 3.39999995214436425e+38f;
                }
                step_3 = m_0.baseStep_1;
            }
        }
        if(skipTo_1 < 3.39999995214436425e+38f)
        {
            t_5 = advancePast_0(t_5, skipTo_1, pace_1, coarsest_1);
            steps_0 = _S56;
            continue;
        }
        var _S63 : f32 = max(step_3, coarsest_1);
        if(IVolumeHooks_hiddenx24defaultImpl_0(texCoord_11))
        {
            t_5 = t_5 + _S63;
            steps_0 = _S56;
            continue;
        }
        var _S64 : vec4<f32> = FieldSampler_sample_0(_S57);
        var _S65 : f32 = FieldTransfer_normalize_0(m_0.component_11, _S64[m_0.component_11]);
        var crossing_0 : f32;
        if(isosurface_0)
        {
            if(!isoSeen_0)
            {
                var _S66 : i32 = isoIntervalOf_0(_S64[m_0.component_11]);
                isoSeen_0 = true;
                isoInterval_0 = _S66;
            }
            else
            {
                var interval_0 : i32 = clamp(isoInterval_0, i32(0), i32(IVolumeHooks_isoValueCountx24defaultImpl_0()));
                var _S67 : f32 = isoBoundary_0(interval_0);
                var _S68 : bool = (_S64[m_0.component_11]) < _S67;
                var interval_1 : i32;
                if(_S68)
                {
                    var interval_2 : i32 = interval_0 - i32(1);
                    value_4 = _S67;
                    interval_1 = interval_2;
                }
                else
                {
                    value_4 = 0.0f;
                    interval_1 = interval_0;
                }
                var _S69 : i32 = interval_1 + i32(1);
                var _S70 : f32 = isoBoundary_0(_S69);
                var interval_3 : i32;
                if((_S64[m_0.component_11]) > _S70)
                {
                    crossed_0 = true;
                    crossing_0 = _S70;
                    interval_3 = _S69;
                }
                else
                {
                    crossed_0 = _S68;
                    crossing_0 = value_4;
                    interval_3 = interval_1;
                }
                if(crossed_0)
                {
                    var _S71 : vec4<f32> = classifyAt_0(_S57, texCoord_11, m_0.component_11, FieldTransfer_normalize_0(m_0.component_11, crossing_0), m_0.normalFromOpacity_1);
                    compositeFrontToBack_0(&(color_5), &(alpha_1), _S71.xyz, _S71.w);
                }
                isoInterval_0 = interval_3;
            }
            t_5 = t_5 + _S63;
            steps_0 = _S56;
            continue;
        }
        if(!compositing_0)
        {
            if(maximum_2)
            {
                value_4 = max(value_5, _S65);
                crossing_0 = count_0;
            }
            else
            {
                if(minimum_0)
                {
                    value_4 = min(value_5, _S65);
                    crossing_0 = count_0;
                }
                else
                {
                    if((m_0.blendMode_0) != u32(3))
                    {
                        crossed_0 = true;
                    }
                    else
                    {
                        if((_S64[m_0.component_11]) >= (m_0.averageMinimum_0))
                        {
                            crossed_0 = (_S64[m_0.component_11]) <= (m_0.averageMaximum_0);
                        }
                        else
                        {
                            crossed_0 = false;
                        }
                    }
                    if(crossed_0)
                    {
                        if(count_0 == 0.0f)
                        {
                            value_4 = 0.0f;
                        }
                        else
                        {
                            value_4 = value_5;
                        }
                        var count_1 : f32 = count_0 + 1.0f;
                        value_4 = value_4 + FieldTransfer_opacity_0(m_0.component_11, _S65) * _S65;
                        crossing_0 = count_1;
                    }
                    else
                    {
                        value_4 = value_5;
                        crossing_0 = count_0;
                    }
                }
            }
            t_5 = t_5 + _S63;
            steps_0 = _S56;
            value_5 = value_4;
            count_0 = crossing_0;
            continue;
        }
        var _S72 : f32 = FieldTransfer_opacity_0(m_0.component_11, _S65);
        var gradient_2 : vec3<f32>;
        if(_S72 > 0.0f)
        {
            if(_S52)
            {
                crossed_0 = true;
            }
            else
            {
                crossed_0 = _S53;
            }
            if(crossed_0)
            {
                gradient_2 = IVolumeSampler_gradientx24defaultImpl_0(_S57, texCoord_11, m_0.component_11);
            }
            else
            {
                gradient_2 = _S47;
            }
            var opacity_2 : f32 = correctOpacity_0(_S72, _S63, m_0.unitDistance_1, m_0.maxOpacity_0);
            if(_S53)
            {
                value_4 = opacity_2 * ITransferFunction_gradientOpacityx24defaultImpl_0(m_0.component_11, gradientMagnitude_0(gradient_2, _S48.spacing_1));
            }
            else
            {
                value_4 = opacity_2;
            }
            value_4 = clamp(value_4, 0.0f, 1.0f);
        }
        else
        {
            gradient_2 = _S47;
            value_4 = _S72;
        }
        if(value_4 > 0.0f)
        {
            var _S73 : vec3<f32> = FieldTransfer_color_0(m_0.component_11, _S65);
            var sampleColor_1 : vec3<f32>;
            if(_S52)
            {
                if(m_0.normalFromOpacity_1)
                {
                    sampleColor_1 = opacityGradient_0(_S57, texCoord_11, m_0.component_11);
                }
                else
                {
                    sampleColor_1 = gradient_2;
                }
                sampleColor_1 = IVolumeHooks_shadex24defaultImpl_0(_S73, texCoord_11, sampleColor_1);
            }
            else
            {
                sampleColor_1 = _S73;
            }
            if(!r_0.hit_0)
            {
                r_0.hit_0 = true;
                r_0.tHit_0 = t_5;
                r_0.texCoordHit_0 = texCoord_11;
            }
            compositeFrontToBack_0(&(color_5), &(alpha_1), sampleColor_1, value_4);
            if(!r_0.visible_0)
            {
                crossed_0 = alpha_1 > 0.0117647061124444f;
            }
            else
            {
                crossed_0 = false;
            }
            if(crossed_0)
            {
                r_0.visible_0 = true;
                r_0.tVisible_0 = t_5;
                r_0.texCoordVisible_0 = texCoord_11;
            }
            if(alpha_1 >= (m_0.opaqueAlpha_0))
            {
                break;
            }
        }
        t_5 = t_5 + _S63;
        steps_0 = _S56;
    }
    if(maximum_2)
    {
        adaptive_1 = true;
    }
    else
    {
        adaptive_1 = minimum_0;
    }
    if(adaptive_1)
    {
        if(value_5 <= -3.39999995214436425e+38f)
        {
            adaptive_1 = true;
        }
        else
        {
            adaptive_1 = value_5 >= 3.39999995214436425e+38f;
        }
        if(adaptive_1)
        {
            return r_0;
        }
        var v_1 : f32 = clamp(value_5, 0.0f, 1.0f);
        var _S74 : f32 = FieldTransfer_opacity_0(m_0.component_11, v_1);
        r_0.color_2 = vec4<f32>(FieldTransfer_color_0(m_0.component_11, v_1) * vec3<f32>(_S74), _S74);
        r_0.covered_0 = true;
        return r_0;
    }
    if(!compositing_0)
    {
        adaptive_1 = !isosurface_0;
    }
    else
    {
        adaptive_1 = false;
    }
    if(adaptive_1)
    {
        if(count_0 == 0.0f)
        {
            return r_0;
        }
        if((m_0.blendMode_0) == u32(3))
        {
            value_4 = value_5 / count_0;
        }
        else
        {
            value_4 = value_5;
        }
        r_0.color_2 = vec4<f32>(vec3<f32>(clamp(value_4, 0.0f, 1.0f)), 1.0f);
        r_0.covered_0 = true;
        return r_0;
    }
    r_0.color_2 = vec4<f32>(min(color_5, vec3<f32>(1.0f)), min(alpha_1, 1.0f));
    if(alpha_1 > 0.0f)
    {
        adaptive_1 = true;
    }
    else
    {
        adaptive_1 = r_0.hit_0;
    }
    r_0.covered_0 = adaptive_1;
    return r_0;
}

fn firstbithigh_0( value_6 : u32) -> u32
{
    var _S75 : u32 = firstLeadingBit(value_6);
    return _S75;
}

fn ExtremesTable_extremes_0( function_0 : u32,  level_0 : u32,  entry_0 : u32) -> vec2<f32>
{
    return extremeTable_0[level_0 * u32(1024) + entry_0];
}

fn classifyCell_0( function_1 : u32,  tableSize_0 : u32,  range_1 : vec2<f32>,  rangeMin_0 : f32,  rangeScale_0 : f32) -> vec2<f32>
{
    var low_0 : f32 = clamp((range_1.x - rangeMin_0) * rangeScale_0, 0.0f, 1.0f);
    var high_0 : f32 = clamp((range_1.y - rangeMin_0) * rangeScale_0, 0.0f, 1.0f);
    var lastEntry_0 : u32 = tableSize_0 - u32(1);
    var _S76 : f32 = f32(tableSize_0);
    var _S77 : u32 = min(u32(floor(max(low_0 * _S76 - 0.5f, 0.0f))), lastEntry_0);
    var last_3 : u32 = clamp(u32(ceil(max(high_0 * _S76 - 0.5f, 0.0f))), _S77, lastEntry_0);
    var level_1 : u32 = firstbithigh_0(last_3 - _S77 + u32(1));
    var _S78 : vec2<f32> = ExtremesTable_extremes_0(function_1, level_1, _S77);
    var _S79 : vec2<f32> = ExtremesTable_extremes_0(function_1, level_1, last_3 + u32(1) - ((u32(1) << (level_1))));
    var _S80 : f32 = max(_S78.y, _S79.y);
    return vec2<f32>(_S80, clamp(4.0f * max(high_0 - low_0, _S80 - min(_S78.x, _S79.x)), 0.0f, 1.0f));
}

@compute
@workgroup_size(64, 1, 1)
fn vol_classify(@builtin(global_invocation_id) gid_0 : vec3<u32>)
{
    var i_3 : u32 = gid_0.y * u32(65536) + gid_0.x;
    if(i_3 >= (params_0.cellCount_0))
    {
        return;
    }
    var range_2 : vec2<f32> = cellRanges_0[i_3];
    var _S81 : vec2<f32>;
    if((range_2.x) > (range_2.y))
    {
        _S81 = vec2<f32>(1.0f);
    }
    else
    {
        if((params_0.valueScale_0) < 0.0f)
        {
            _S81 = range_2.yx;
        }
        else
        {
            _S81 = range_2;
        }
        _S81 = classifyCell_0(u32(0), u32(1024), _S81, params_0.valueMin_0, params_0.valueScale_0);
    }
    classified_0[i_3] = _S81;
    return;
}

fn unproject_0( inverseViewProjection_1 : mat4x4<f32>,  ndc_0 : vec2<f32>,  depth_0 : f32) -> vec3<f32>
{
    var p_0 : vec4<f32> = (((vec4<f32>(ndc_0, depth_0, 1.0f)) * (inverseViewProjection_1)));
    return p_0.xyz / vec3<f32>(p_0.w);
}

fn VolumeRay_x24init_0( origin_2 : vec3<f32>,  direction_2 : vec3<f32>,  tNear_1 : f32,  tFar_1 : f32) -> VolumeRay_0
{
    var _S82 : VolumeRay_0;
    _S82.origin_0 = origin_2;
    _S82.direction_0 = direction_2;
    _S82.tNear_0 = tNear_1;
    _S82.tFar_0 = tFar_1;
    return _S82;
}

fn intersectUnitBox_0( origin_3 : vec3<f32>,  direction_3 : vec3<f32>) -> vec2<f32>
{
    var inverse_1 : vec3<f32> = vec3<f32>(1.0f) / direction_3;
    var t0_0 : vec3<f32> = (vec3<f32>(0.0f) - origin_3) * inverse_1;
    var t1_0 : vec3<f32> = (vec3<f32>(1.0f) - origin_3) * inverse_1;
    var tMin_0 : vec3<f32> = min(t0_0, t1_0);
    var tMax_1 : vec3<f32> = max(t0_0, t1_0);
    return vec2<f32>(max(max(tMin_0.x, tMin_0.y), tMin_0.z), min(min(tMax_1.x, tMax_1.y), tMax_1.z));
}

fn volumeRay_0( worldToTexture_1 : mat4x4<f32>,  origin_4 : vec3<f32>,  direction_4 : vec3<f32>,  tLimit_0 : f32) -> VolumeRay_0
{
    var r_1 : VolumeRay_0 = VolumeRay_x24init_0(vec3<f32>(0.0f), vec3<f32>(0.0f, 0.0f, 1.0f), 0.0f, 0.0f);
    var _S83 : vec3<f32> = (((vec4<f32>(origin_4, 1.0f)) * (worldToTexture_1))).xyz;
    r_1.origin_0 = _S83;
    var _S84 : vec3<f32> = (((vec4<f32>(direction_4, 0.0f)) * (worldToTexture_1))).xyz;
    r_1.direction_0 = _S84;
    var span_0 : vec2<f32> = intersectUnitBox_0(_S83, _S84);
    r_1.tNear_0 = max(span_0.x, 0.0f);
    r_1.tFar_0 = min(span_0.y, tLimit_0);
    return r_1;
}

fn VolumeMarch_x24init_0( blendMode_1 : u32,  component_12 : u32,  baseStep_3 : f32,  unitDistance_3 : f32,  maxOpacity_2 : f32,  maxSteps_2 : u32,  jitter_1 : f32,  adaptive_2 : bool,  minStepFactor_2 : f32,  maxStepFactor_2 : f32,  skipEmpty_1 : bool,  emptyThreshold_1 : f32,  normalFromOpacity_2 : bool,  averageMinimum_1 : f32,  averageMaximum_1 : f32,  opaqueAlpha_1 : f32) -> VolumeMarch_0
{
    var _S85 : VolumeMarch_0;
    _S85.blendMode_0 = blendMode_1;
    _S85.component_11 = component_12;
    _S85.baseStep_1 = baseStep_3;
    _S85.unitDistance_1 = unitDistance_3;
    _S85.maxOpacity_0 = maxOpacity_2;
    _S85.maxSteps_1 = maxSteps_2;
    _S85.jitter_0 = jitter_1;
    _S85.adaptive_0 = adaptive_2;
    _S85.minStepFactor_0 = minStepFactor_2;
    _S85.maxStepFactor_0 = maxStepFactor_2;
    _S85.skipEmpty_0 = skipEmpty_1;
    _S85.emptyThreshold_0 = emptyThreshold_1;
    _S85.normalFromOpacity_1 = normalFromOpacity_2;
    _S85.averageMinimum_0 = averageMinimum_1;
    _S85.averageMaximum_0 = averageMaximum_1;
    _S85.opaqueAlpha_0 = opaqueAlpha_1;
    return _S85;
}

@compute
@workgroup_size(256, 1, 1)
fn vol_main(@builtin(global_invocation_id) gid_1 : vec3<u32>)
{
    var pix_0 : u32 = gid_1.x;
    if(pix_0 >= (params_0.width_0 * params_0.height_0))
    {
        return;
    }
    var px_0 : u32 = pix_0 % params_0.width_0;
    var py_0 : u32 = pix_0 / params_0.width_0;
    var ndc_1 : vec2<f32> = vec2<f32>((f32(px_0) + 0.5f) / f32(params_0.width_0) * 2.0f - 1.0f, 1.0f - (f32(py_0) + 0.5f) / f32(params_0.height_0) * 2.0f);
    var origin_5 : vec3<f32> = params_0.eye_0.xyz;
    var farPoint_0 : vec3<f32> = unproject_0(mat4x4<f32>(params_0.inverseViewProjection_0.data_0[i32(0)][i32(0)], params_0.inverseViewProjection_0.data_0[i32(1)][i32(0)], params_0.inverseViewProjection_0.data_0[i32(2)][i32(0)], params_0.inverseViewProjection_0.data_0[i32(3)][i32(0)], params_0.inverseViewProjection_0.data_0[i32(0)][i32(1)], params_0.inverseViewProjection_0.data_0[i32(1)][i32(1)], params_0.inverseViewProjection_0.data_0[i32(2)][i32(1)], params_0.inverseViewProjection_0.data_0[i32(3)][i32(1)], params_0.inverseViewProjection_0.data_0[i32(0)][i32(2)], params_0.inverseViewProjection_0.data_0[i32(1)][i32(2)], params_0.inverseViewProjection_0.data_0[i32(2)][i32(2)], params_0.inverseViewProjection_0.data_0[i32(3)][i32(2)], params_0.inverseViewProjection_0.data_0[i32(0)][i32(3)], params_0.inverseViewProjection_0.data_0[i32(1)][i32(3)], params_0.inverseViewProjection_0.data_0[i32(2)][i32(3)], params_0.inverseViewProjection_0.data_0[i32(3)][i32(3)]), ndc_1, 1.0f);
    var direction_5 : vec3<f32> = normalize(farPoint_0 - origin_5);
    var tLimit_1 : f32 = distance(origin_5, farPoint_0);
    var surface_0 : f32 = (bitcast<f32>((surfaceDepth_0[pix_0])));
    var tLimit_2 : f32;
    if(surface_0 < 1.0f)
    {
        tLimit_2 = min(tLimit_1, dot(unproject_0(mat4x4<f32>(params_0.inverseViewProjection_0.data_0[i32(0)][i32(0)], params_0.inverseViewProjection_0.data_0[i32(1)][i32(0)], params_0.inverseViewProjection_0.data_0[i32(2)][i32(0)], params_0.inverseViewProjection_0.data_0[i32(3)][i32(0)], params_0.inverseViewProjection_0.data_0[i32(0)][i32(1)], params_0.inverseViewProjection_0.data_0[i32(1)][i32(1)], params_0.inverseViewProjection_0.data_0[i32(2)][i32(1)], params_0.inverseViewProjection_0.data_0[i32(3)][i32(1)], params_0.inverseViewProjection_0.data_0[i32(0)][i32(2)], params_0.inverseViewProjection_0.data_0[i32(1)][i32(2)], params_0.inverseViewProjection_0.data_0[i32(2)][i32(2)], params_0.inverseViewProjection_0.data_0[i32(3)][i32(2)], params_0.inverseViewProjection_0.data_0[i32(0)][i32(3)], params_0.inverseViewProjection_0.data_0[i32(1)][i32(3)], params_0.inverseViewProjection_0.data_0[i32(2)][i32(3)], params_0.inverseViewProjection_0.data_0[i32(3)][i32(3)]), ndc_1, surface_0) - origin_5, direction_5));
    }
    else
    {
        tLimit_2 = tLimit_1;
    }
    var ray_2 : VolumeRay_0 = volumeRay_0(mat4x4<f32>(params_0.worldToTexture_0.data_0[i32(0)][i32(0)], params_0.worldToTexture_0.data_0[i32(1)][i32(0)], params_0.worldToTexture_0.data_0[i32(2)][i32(0)], params_0.worldToTexture_0.data_0[i32(3)][i32(0)], params_0.worldToTexture_0.data_0[i32(0)][i32(1)], params_0.worldToTexture_0.data_0[i32(1)][i32(1)], params_0.worldToTexture_0.data_0[i32(2)][i32(1)], params_0.worldToTexture_0.data_0[i32(3)][i32(1)], params_0.worldToTexture_0.data_0[i32(0)][i32(2)], params_0.worldToTexture_0.data_0[i32(1)][i32(2)], params_0.worldToTexture_0.data_0[i32(2)][i32(2)], params_0.worldToTexture_0.data_0[i32(3)][i32(2)], params_0.worldToTexture_0.data_0[i32(0)][i32(3)], params_0.worldToTexture_0.data_0[i32(1)][i32(3)], params_0.worldToTexture_0.data_0[i32(2)][i32(3)], params_0.worldToTexture_0.data_0[i32(3)][i32(3)]), origin_5, direction_5, tLimit_2);
    if((ray_2.tNear_0) >= (ray_2.tFar_0))
    {
        return;
    }
    var m_1 : VolumeMarch_0 = VolumeMarch_x24init_0(u32(0), u32(0), 1.0f, 1.0f, 1.0f, u32(1024), 0.0f, false, 1.0f, 1.0f, false, 0.0f, false, -3.39999995214436425e+38f, 3.39999995214436425e+38f, 0.99000000953674316f);
    m_1.baseStep_1 = params_0.baseStep_0;
    m_1.unitDistance_1 = params_0.unitDistance_0;
    m_1.maxSteps_1 = params_0.maxSteps_0;
    m_1.skipEmpty_0 = true;
    m_1.jitter_0 = 0.5f * params_0.baseStep_0;
    var _S86 : VolumeResult_0 = marchVolume_0(ray_2, m_1);
    var entry_1 : vec4<f32> = (((vec4<f32>(origin_5 + vec3<f32>(ray_2.tNear_0) * direction_5, 1.0f)) * (mat4x4<f32>(params_0.viewProjection_0.data_0[i32(0)][i32(0)], params_0.viewProjection_0.data_0[i32(1)][i32(0)], params_0.viewProjection_0.data_0[i32(2)][i32(0)], params_0.viewProjection_0.data_0[i32(3)][i32(0)], params_0.viewProjection_0.data_0[i32(0)][i32(1)], params_0.viewProjection_0.data_0[i32(1)][i32(1)], params_0.viewProjection_0.data_0[i32(2)][i32(1)], params_0.viewProjection_0.data_0[i32(3)][i32(1)], params_0.viewProjection_0.data_0[i32(0)][i32(2)], params_0.viewProjection_0.data_0[i32(1)][i32(2)], params_0.viewProjection_0.data_0[i32(2)][i32(2)], params_0.viewProjection_0.data_0[i32(3)][i32(2)], params_0.viewProjection_0.data_0[i32(0)][i32(3)], params_0.viewProjection_0.data_0[i32(1)][i32(3)], params_0.viewProjection_0.data_0[i32(2)][i32(3)], params_0.viewProjection_0.data_0[i32(3)][i32(3)]))));
    depthBuf_0[pix_0] = min(depthBuf_0[pix_0], (bitcast<u32>((clamp(entry_1.z / entry_1.w, 0.0f, 1.0f)))));
    if(!_S86.covered_0)
    {
        return;
    }
    var a_0 : f32 = _S86.color_2.w;
    var _S87 : f32 = 1.0f - a_0;
    colorBuf_0[pix_0] = vec4<f32>(_S86.color_2.xyz + colorBuf_0[pix_0].xyz * vec3<f32>(_S87), a_0 + colorBuf_0[pix_0].w * _S87);
    return;
}


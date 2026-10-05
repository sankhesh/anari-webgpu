// Generated from shaders/pbr.slang with its default specializations
// by vtkSlang (Slang 2026.13.1). Do not edit; regenerate with the
// anari_webgpu_update_wgsl target.

struct _MatrixStorage_float4x4_ColMajorstd140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct PBRUniforms_std140_0
{
    @align(16) mvp_0 : _MatrixStorage_float4x4_ColMajorstd140_0,
    @align(16) baseColor_0 : vec4<f32>,
    @align(16) emissive_0 : vec4<f32>,
    @align(16) metallicRoughness_0 : vec4<f32>,
    @align(16) lightDir_0 : vec3<f32>,
    @align(4) ambientRadiance_0 : f32,
    @align(16) lightColor_0 : vec3<f32>,
    @align(4) lightIrradiance_0 : f32,
    @align(16) width_0 : u32,
    @align(4) height_0 : u32,
    @align(8) numTriangles_0 : u32,
    @align(4) hasNormals_0 : u32,
    @align(16) hasColors_0 : u32,
    @align(4) alphaMode_0 : u32,
    @align(8) alphaCutoff_0 : f32,
    @align(4) _pad_0 : u32,
};

@binding(2) @group(0) var<uniform> params_0 : PBRUniforms_std140_0;
@binding(6) @group(0) var<storage, read> indices_0 : array<u32>;

@binding(3) @group(0) var<storage, read> positions_0 : array<f32>;

@binding(4) @group(0) var<storage, read> normals_0 : array<f32>;

@binding(5) @group(0) var<storage, read> colors_0 : array<f32>;

@binding(1) @group(0) var<storage, read_write> depthBufI_0 : array<atomic<u32>>;

@binding(0) @group(0) var<storage, read_write> colorBuf_0 : array<vec4<f32>>;

struct VertexSample_0
{
     position_0 : vec3<f32>,
     normal_0 : vec3<f32>,
     tangent_0 : vec3<f32>,
     uv_0 : vec2<f32>,
     color_0 : vec4<f32>,
     pointId_0 : u32,
     cellId_0 : u32,
     primitiveId_0 : u32,
     modelToClip_0 : mat4x4<f32>,
};

fn VertexSample_x24init_0( position_1 : vec3<f32>,  normal_1 : vec3<f32>,  tangent_1 : vec3<f32>,  uv_1 : vec2<f32>,  color_1 : vec4<f32>,  pointId_1 : u32,  cellId_1 : u32,  primitiveId_1 : u32,  modelToClip_1 : mat4x4<f32>) -> VertexSample_0
{
    var _S1 : VertexSample_0;
    _S1.position_0 = position_1;
    _S1.normal_0 = normal_1;
    _S1.tangent_0 = tangent_1;
    _S1.uv_0 = uv_1;
    _S1.color_0 = color_1;
    _S1.pointId_0 = pointId_1;
    _S1.cellId_0 = cellId_1;
    _S1.primitiveId_0 = primitiveId_1;
    _S1.modelToClip_0 = modelToClip_1;
    return _S1;
}

struct ScreenTriangle_0
{
     s0_0 : vec3<f32>,
     s1_0 : vec3<f32>,
     s2_0 : vec3<f32>,
     ndc0_0 : vec3<f32>,
     ndc1_0 : vec3<f32>,
     ndc2_0 : vec3<f32>,
     area_0 : f32,
     minX_0 : f32,
     maxX_0 : f32,
     minY_0 : f32,
     maxY_0 : f32,
};

fn ScreenTriangle_x24init_0( s0_1 : vec3<f32>,  s1_1 : vec3<f32>,  s2_1 : vec3<f32>,  ndc0_1 : vec3<f32>,  ndc1_1 : vec3<f32>,  ndc2_1 : vec3<f32>,  area_1 : f32,  minX_1 : f32,  maxX_1 : f32,  minY_1 : f32,  maxY_1 : f32) -> ScreenTriangle_0
{
    var _S2 : ScreenTriangle_0;
    _S2.s0_0 = s0_1;
    _S2.s1_0 = s1_1;
    _S2.s2_0 = s2_1;
    _S2.ndc0_0 = ndc0_1;
    _S2.ndc1_0 = ndc1_1;
    _S2.ndc2_0 = ndc2_1;
    _S2.area_0 = area_1;
    _S2.minX_0 = minX_1;
    _S2.maxX_0 = maxX_1;
    _S2.minY_0 = minY_1;
    _S2.maxY_0 = maxY_1;
    return _S2;
}

struct PBRTriangle_0
{
     v0_0 : VertexSample_0,
     v1_0 : VertexSample_0,
     v2_0 : VertexSample_0,
     t_0 : ScreenTriangle_0,
     id_0 : u32,
};

fn PBRTriangle_x24init_0( v0_1 : VertexSample_0,  v1_1 : VertexSample_0,  v2_1 : VertexSample_0,  t_1 : ScreenTriangle_0,  id_1 : u32) -> PBRTriangle_0
{
    var _S3 : PBRTriangle_0;
    _S3.v0_0 = v0_1;
    _S3.v1_0 = v1_1;
    _S3.v2_0 = v2_1;
    _S3.t_0 = t_1;
    _S3.id_0 = id_1;
    return _S3;
}

fn pbr_triangleIndices_0( triIdx_0 : u32) -> vec3<u32>
{
    var _S4 : u32 = triIdx_0 * u32(3);
    return vec3<u32>(indices_0[_S4], indices_0[_S4 + u32(1)], indices_0[_S4 + u32(2)]);
}

fn pbr_getPos_0( i_0 : u32) -> vec3<f32>
{
    var _S5 : u32 = i_0 * u32(3);
    return vec3<f32>(positions_0[_S5], positions_0[_S5 + u32(1)], positions_0[_S5 + u32(2)]);
}

fn pbr_getNorm_0( i_1 : u32) -> vec3<f32>
{
    var _S6 : u32 = i_1 * u32(3);
    return vec3<f32>(normals_0[_S6], normals_0[_S6 + u32(1)], normals_0[_S6 + u32(2)]);
}

fn pbr_getColor_0( i_2 : u32) -> vec4<f32>
{
    var _S7 : u32 = i_2 * u32(4);
    return vec4<f32>(colors_0[_S7], colors_0[_S7 + u32(1)], colors_0[_S7 + u32(2)], colors_0[_S7 + u32(3)]);
}

fn IdentityVertex_modify_0( v_0 : VertexSample_0) -> VertexSample_0
{
    return v_0;
}

fn pbr_fetchVertex_0( triIdx_1 : u32,  idx_0 : u32) -> VertexSample_0
{
    var _S8 : vec3<f32> = vec3<f32>(0.0f);
    var v_1 : VertexSample_0 = VertexSample_x24init_0(_S8, vec3<f32>(0.0f, 0.0f, 1.0f), _S8, vec2<f32>(0.0f), vec4<f32>(1.0f), u32(0), u32(0), u32(0), mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    v_1.position_0 = pbr_getPos_0(idx_0);
    if((params_0.hasNormals_0) != u32(0))
    {
        v_1.normal_0 = pbr_getNorm_0(idx_0);
    }
    if((params_0.hasColors_0) != u32(0))
    {
        v_1.color_0 = pbr_getColor_0(idx_0);
    }
    v_1.pointId_0 = idx_0;
    v_1.cellId_0 = triIdx_1;
    v_1.primitiveId_0 = triIdx_1;
    v_1.modelToClip_0 = mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]);
    return IdentityVertex_modify_0(v_1);
}

fn edgeFunction_0( a_0 : vec2<f32>,  b_0 : vec2<f32>,  c_0 : vec2<f32>) -> f32
{
    var _S9 : f32 = a_0.x;
    var _S10 : f32 = a_0.y;
    return (c_0.x - _S9) * (b_0.y - _S10) - (c_0.y - _S10) * (b_0.x - _S9);
}

fn setupScreenTriangle_0( mvp_1 : mat4x4<f32>,  p0_0 : vec3<f32>,  p1_0 : vec3<f32>,  p2_0 : vec3<f32>,  width_1 : u32,  height_1 : u32,  t_2 : ptr<function, ScreenTriangle_0>) -> bool
{
    var _S11 : vec3<f32> = vec3<f32>(0.0f);
    (*t_2) = ScreenTriangle_x24init_0(_S11, _S11, _S11, _S11, _S11, _S11, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    var c0_0 : vec4<f32> = (((vec4<f32>(p0_0, 1.0f)) * (mvp_1)));
    var c1_0 : vec4<f32> = (((vec4<f32>(p1_0, 1.0f)) * (mvp_1)));
    var c2_0 : vec4<f32> = (((vec4<f32>(p2_0, 1.0f)) * (mvp_1)));
    var _S12 : f32 = c0_0.w;
    var _S13 : bool;
    if(_S12 <= 0.0f)
    {
        _S13 = true;
    }
    else
    {
        _S13 = (c1_0.w) <= 0.0f;
    }
    if(_S13)
    {
        _S13 = true;
    }
    else
    {
        _S13 = (c2_0.w) <= 0.0f;
    }
    if(_S13)
    {
        return false;
    }
    var _S14 : vec3<f32> = c0_0.xyz / vec3<f32>(_S12);
    (*t_2).ndc0_0 = _S14;
    var _S15 : vec3<f32> = c1_0.xyz / vec3<f32>(c1_0.w);
    (*t_2).ndc1_0 = _S15;
    var _S16 : vec3<f32> = c2_0.xyz / vec3<f32>(c2_0.w);
    (*t_2).ndc2_0 = _S16;
    var W_0 : f32 = f32(width_1);
    var H_0 : f32 = f32(height_1);
    var _S17 : f32 = (_S14.x * 0.5f + 0.5f) * W_0;
    var _S18 : f32 = (1.0f - (_S14.y * 0.5f + 0.5f)) * H_0;
    (*t_2).s0_0 = vec3<f32>(_S17, _S18, _S14.z);
    var _S19 : f32 = (_S15.x * 0.5f + 0.5f) * W_0;
    var _S20 : f32 = (1.0f - (_S15.y * 0.5f + 0.5f)) * H_0;
    (*t_2).s1_0 = vec3<f32>(_S19, _S20, _S15.z);
    var _S21 : f32 = (_S16.x * 0.5f + 0.5f) * W_0;
    var _S22 : f32 = (1.0f - (_S16.y * 0.5f + 0.5f)) * H_0;
    (*t_2).s2_0 = vec3<f32>(_S21, _S22, _S16.z);
    (*t_2).minX_0 = max(0.0f, min(min(_S17, _S19), _S21));
    (*t_2).maxX_0 = min(W_0 - 1.0f, max(max(_S17, _S19), _S21));
    (*t_2).minY_0 = max(0.0f, min(min(_S18, _S20), _S22));
    (*t_2).maxY_0 = min(H_0 - 1.0f, max(max(_S18, _S20), _S22));
    var _S23 : f32 = edgeFunction_0(vec2<f32>(_S17, _S18), vec2<f32>(_S19, _S20), vec2<f32>(_S21, _S22));
    (*t_2).area_0 = _S23;
    return (abs(_S23)) >= 9.99999997475242708e-07f;
}

fn pbr_setupTriangle_0( triIdx_2 : u32,  r_0 : ptr<function, PBRTriangle_0>) -> bool
{
    var _S24 : vec3<f32> = vec3<f32>(0.0f);
    var _S25 : VertexSample_0 = VertexSample_x24init_0(_S24, vec3<f32>(0.0f, 0.0f, 1.0f), _S24, vec2<f32>(0.0f), vec4<f32>(1.0f), u32(0), u32(0), u32(0), mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    (*r_0) = PBRTriangle_x24init_0(_S25, _S25, _S25, ScreenTriangle_x24init_0(_S24, _S24, _S24, _S24, _S24, _S24, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), u32(0));
    (*r_0).id_0 = triIdx_2;
    var i_3 : vec3<u32> = pbr_triangleIndices_0(triIdx_2);
    (*r_0).v0_0 = pbr_fetchVertex_0(triIdx_2, i_3.x);
    (*r_0).v1_0 = pbr_fetchVertex_0(triIdx_2, i_3.y);
    (*r_0).v2_0 = pbr_fetchVertex_0(triIdx_2, i_3.z);
    var _S26 : mat4x4<f32> = mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]);
    var _S27 : ScreenTriangle_0 = (*r_0).t_0;
    var _S28 : bool = setupScreenTriangle_0(_S26, (*r_0).v0_0.position_0, (*r_0).v1_0.position_0, (*r_0).v2_0.position_0, params_0.width_0, params_0.height_0, &(_S27));
    (*r_0).t_0 = _S27;
    return _S28;
}

fn barycentric_0( t_3 : ScreenTriangle_0,  px_0 : i32,  py_0 : i32) -> vec3<f32>
{
    var p_0 : vec2<f32> = vec2<f32>(f32(px_0) + 0.5f, f32(py_0) + 0.5f);
    var _S29 : vec2<f32> = t_3.s1_0.xy;
    var _S30 : vec2<f32> = t_3.s2_0.xy;
    var _S31 : vec2<f32> = t_3.s0_0.xy;
    return vec3<f32>(edgeFunction_0(_S29, _S30, p_0) / t_3.area_0, edgeFunction_0(_S30, _S31, p_0) / t_3.area_0, edgeFunction_0(_S31, _S29, p_0) / t_3.area_0);
}

fn covered_0( w_0 : vec3<f32>) -> bool
{
    var _S32 : bool;
    if((w_0.x) >= 0.0f)
    {
        _S32 = (w_0.y) >= 0.0f;
    }
    else
    {
        _S32 = false;
    }
    if(_S32)
    {
        _S32 = (w_0.z) >= 0.0f;
    }
    else
    {
        _S32 = false;
    }
    return _S32;
}

fn interpolateDepth_0( t_4 : ScreenTriangle_0,  w_1 : vec3<f32>) -> f32
{
    return w_1.x * t_4.s0_0.z + w_1.y * t_4.s1_0.z + w_1.z * t_4.s2_0.z;
}

struct CameraSample_0
{
     shadingToClip_0 : mat4x4<f32>,
     parallel_0 : bool,
};

fn CameraSample_x24init_0( shadingToClip_1 : mat4x4<f32>,  parallel_1 : bool) -> CameraSample_0
{
    var _S33 : CameraSample_0;
    _S33.shadingToClip_0 = shadingToClip_1;
    _S33.parallel_0 = parallel_1;
    return _S33;
}

struct HostMaterial_0
{
     baseColor_1 : vec3<f32>,
     opacity_0 : f32,
     metallic_0 : f32,
     roughness_0 : f32,
     emissive_1 : vec3<f32>,
     ambientColor_0 : vec3<f32>,
     specularColor_0 : vec3<f32>,
     specularPower_0 : f32,
};

fn HostMaterial_x24init_0( baseColor_2 : vec3<f32>,  opacity_1 : f32,  metallic_1 : f32,  roughness_1 : f32,  emissive_2 : vec3<f32>,  ambientColor_1 : vec3<f32>,  specularColor_1 : vec3<f32>,  specularPower_1 : f32) -> HostMaterial_0
{
    var _S34 : HostMaterial_0;
    _S34.baseColor_1 = baseColor_2;
    _S34.opacity_0 = opacity_1;
    _S34.metallic_0 = metallic_1;
    _S34.roughness_0 = roughness_1;
    _S34.emissive_1 = emissive_2;
    _S34.ambientColor_0 = ambientColor_1;
    _S34.specularColor_0 = specularColor_1;
    _S34.specularPower_0 = specularPower_1;
    return _S34;
}

struct SurfaceSample_0
{
     position_2 : vec3<f32>,
     normal_2 : vec3<f32>,
     tangent_2 : vec3<f32>,
     viewDir_0 : vec3<f32>,
     uv_2 : vec2<f32>,
     scalar_0 : f32,
     fragCoord_0 : vec4<f32>,
     primitiveId_2 : u32,
     cellId_2 : u32,
     frontFacing_0 : bool,
     userData_0 : vec4<u32>,
     camera_0 : CameraSample_0,
     host_0 : HostMaterial_0,
};

fn SurfaceSample_x24init_0( position_3 : vec3<f32>,  normal_3 : vec3<f32>,  tangent_3 : vec3<f32>,  viewDir_1 : vec3<f32>,  uv_3 : vec2<f32>,  scalar_1 : f32,  fragCoord_1 : vec4<f32>,  primitiveId_3 : u32,  cellId_3 : u32,  frontFacing_1 : bool,  userData_1 : vec4<u32>,  camera_1 : CameraSample_0,  host_1 : HostMaterial_0) -> SurfaceSample_0
{
    var _S35 : SurfaceSample_0;
    _S35.position_2 = position_3;
    _S35.normal_2 = normal_3;
    _S35.tangent_2 = tangent_3;
    _S35.viewDir_0 = viewDir_1;
    _S35.uv_2 = uv_3;
    _S35.scalar_0 = scalar_1;
    _S35.fragCoord_0 = fragCoord_1;
    _S35.primitiveId_2 = primitiveId_3;
    _S35.cellId_2 = cellId_3;
    _S35.frontFacing_0 = frontFacing_1;
    _S35.userData_0 = userData_1;
    _S35.camera_0 = camera_1;
    _S35.host_0 = host_1;
    return _S35;
}

fn pbr_surfaceSample_0( r_1 : PBRTriangle_0,  w_2 : vec3<f32>,  px_1 : i32,  py_1 : i32,  depth_0 : f32) -> SurfaceSample_0
{
    var _S36 : vec3<f32> = vec3<f32>(0.0f);
    const _S37 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 1.0f);
    var s_0 : SurfaceSample_0 = SurfaceSample_x24init_0(_S36, _S37, _S36, _S37, vec2<f32>(0.0f), 0.0f, vec4<f32>(0.0f), u32(0), u32(0), true, vec4<u32>(u32(0)), CameraSample_x24init_0(mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f), false), HostMaterial_x24init_0(vec3<f32>(1.0f), 1.0f, 0.0f, 1.0f, _S36, _S36, _S36, 1.0f));
    var _S38 : f32 = w_2.x;
    var _S39 : vec3<f32> = vec3<f32>(_S38);
    var _S40 : f32 = w_2.y;
    var _S41 : vec3<f32> = vec3<f32>(_S40);
    var _S42 : f32 = w_2.z;
    var _S43 : vec3<f32> = vec3<f32>(_S42);
    s_0.position_2 = r_1.v0_0.position_0 * _S39 + r_1.v1_0.position_0 * _S41 + r_1.v2_0.position_0 * _S43;
    if((params_0.hasNormals_0) != u32(0))
    {
        s_0.normal_2 = normalize(r_1.v0_0.normal_0 * _S39 + r_1.v1_0.normal_0 * _S41 + r_1.v2_0.normal_0 * _S43);
    }
    else
    {
        s_0.normal_2 = normalize(cross(r_1.v1_0.position_0 - r_1.v0_0.position_0, r_1.v2_0.position_0 - r_1.v0_0.position_0));
    }
    var ndcC_0 : vec3<f32> = r_1.t_0.ndc0_0 * _S39 + r_1.t_0.ndc1_0 * _S41 + r_1.t_0.ndc2_0 * _S43;
    s_0.viewDir_0 = normalize(vec3<f32>(- ndcC_0.x, - ndcC_0.y, 1.0f));
    s_0.uv_2 = r_1.v0_0.uv_0 * vec2<f32>(_S38) + r_1.v1_0.uv_0 * vec2<f32>(_S40) + r_1.v2_0.uv_0 * vec2<f32>(_S42);
    s_0.fragCoord_0 = vec4<f32>(f32(px_1) + 0.5f, f32(py_1) + 0.5f, depth_0, 1.0f);
    s_0.primitiveId_2 = r_1.id_0;
    s_0.cellId_2 = r_1.id_0;
    s_0.frontFacing_0 = (r_1.t_0.area_0) < 0.0f;
    s_0.camera_0.shadingToClip_0 = mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]);
    return s_0;
}

fn depthToU32_0( d_0 : f32) -> u32
{
    return (bitcast<u32>((clamp(d_0, 0.0f, 1.0f))));
}

fn StandardOutput_keep_0( s_1 : SurfaceSample_0) -> bool
{
    return true;
}

fn StandardOutput_depth_0( depth_1 : f32,  s_2 : SurfaceSample_0) -> f32
{
    return depth_1;
}

@compute
@workgroup_size(256, 1, 1)
fn pbr_depth_main(@builtin(global_invocation_id) gid_0 : vec3<u32>)
{
    var triIdx_3 : u32 = gid_0.x;
    if(triIdx_3 >= (params_0.numTriangles_0))
    {
        return;
    }
    var r_2 : PBRTriangle_0;
    var _S44 : bool = pbr_setupTriangle_0(triIdx_3, &(r_2));
    if(!_S44)
    {
        return;
    }
    var py_2 : i32 = i32(r_2.t_0.minY_0);
    for(;;)
    {
        if(py_2 <= i32(r_2.t_0.maxY_0))
        {
        }
        else
        {
            break;
        }
        var px_2 : i32 = i32(r_2.t_0.minX_0);
        for(;;)
        {
            if(px_2 <= i32(r_2.t_0.maxX_0))
            {
            }
            else
            {
                break;
            }
            var w_3 : vec3<f32> = barycentric_0(r_2.t_0, px_2, py_2);
            if(!covered_0(w_3))
            {
                px_2 = px_2 + i32(1);
                continue;
            }
            var z_0 : f32 = interpolateDepth_0(r_2.t_0, w_3);
            var s_3 : SurfaceSample_0 = pbr_surfaceSample_0(r_2, w_3, px_2, py_2, z_0);
            if(!StandardOutput_keep_0(s_3))
            {
                px_2 = px_2 + i32(1);
                continue;
            }
            var _S45 : u32 = atomicMin(&(depthBufI_0[u32(py_2) * params_0.width_0 + u32(px_2)]), depthToU32_0(StandardOutput_depth_0(z_0, s_3)));
            px_2 = px_2 + i32(1);
        }
        py_2 = py_2 + i32(1);
    }
    return;
}

struct LightSample_0
{
     direction_0 : vec3<f32>,
     radiance_0 : vec3<f32>,
};

fn LightSample_x24init_0( direction_1 : vec3<f32>,  radiance_1 : vec3<f32>) -> LightSample_0
{
    var _S46 : LightSample_0;
    _S46.direction_0 = direction_1;
    _S46.radiance_0 = radiance_1;
    return _S46;
}

fn GeometricNormal_normal_0( s_4 : SurfaceSample_0) -> vec3<f32>
{
    return s_4.normal_2;
}

struct MaterialSample_0
{
     baseColor_3 : vec3<f32>,
     opacity_2 : f32,
     metallic_2 : f32,
     roughness_2 : f32,
     emissive_3 : vec3<f32>,
     ambientColor_2 : vec3<f32>,
     specularColor_2 : vec3<f32>,
     specularPower_2 : f32,
};

fn MaterialSample_x24init_0( baseColor_4 : vec3<f32>,  opacity_3 : f32,  metallic_3 : f32,  roughness_3 : f32,  emissive_4 : vec3<f32>,  ambientColor_3 : vec3<f32>,  specularColor_3 : vec3<f32>,  specularPower_3 : f32) -> MaterialSample_0
{
    var _S47 : MaterialSample_0;
    _S47.baseColor_3 = baseColor_4;
    _S47.opacity_2 = opacity_3;
    _S47.metallic_2 = metallic_3;
    _S47.roughness_2 = roughness_3;
    _S47.emissive_3 = emissive_4;
    _S47.ambientColor_2 = ambientColor_3;
    _S47.specularColor_2 = specularColor_3;
    _S47.specularPower_2 = specularPower_3;
    return _S47;
}

fn HostDrivenMaterial_evaluate_0( s_5 : SurfaceSample_0) -> MaterialSample_0
{
    var _S48 : vec3<f32> = vec3<f32>(0.0f);
    var m_0 : MaterialSample_0 = MaterialSample_x24init_0(vec3<f32>(1.0f), 1.0f, 0.0f, 1.0f, _S48, _S48, _S48, 1.0f);
    m_0.baseColor_3 = s_5.host_0.baseColor_1;
    m_0.opacity_2 = s_5.host_0.opacity_0;
    m_0.metallic_2 = s_5.host_0.metallic_0;
    m_0.roughness_2 = s_5.host_0.roughness_0;
    m_0.emissive_3 = s_5.host_0.emissive_1;
    m_0.ambientColor_2 = s_5.host_0.ambientColor_0;
    m_0.specularColor_2 = s_5.host_0.specularColor_0;
    m_0.specularPower_2 = s_5.host_0.specularPower_0;
    return m_0;
}

fn CookTorranceLighting_ambient_0( m_1 : MaterialSample_0,  s_6 : SurfaceSample_0,  ambientRadiance_1 : vec3<f32>) -> vec3<f32>
{
    return ambientRadiance_1 * m_1.baseColor_3;
}

fn CookTorranceLighting_D_GGX_0( NdotH_0 : f32,  alpha_0 : f32) -> f32
{
    var a2_0 : f32 = alpha_0 * alpha_0;
    var d_1 : f32 = NdotH_0 * NdotH_0 * (a2_0 - 1.0f) + 1.0f;
    return a2_0 / (3.14159274101257324f * d_1 * d_1 + 0.00009999999747379f);
}

fn CookTorranceLighting_G_SchlickGGX_0( NdotX_0 : f32,  roughness_4 : f32) -> f32
{
    var r_3 : f32 = roughness_4 + 1.0f;
    var k_0 : f32 = r_3 * r_3 / 8.0f;
    return NdotX_0 / (NdotX_0 * (1.0f - k_0) + k_0 + 0.00009999999747379f);
}

fn CookTorranceLighting_F_Schlick_0( cosTheta_0 : f32,  F0_0 : vec3<f32>) -> vec3<f32>
{
    var t_5 : f32 = clamp(1.0f - cosTheta_0, 0.0f, 1.0f);
    return F0_0 + (vec3<f32>(1.0f) - F0_0) * vec3<f32>((t_5 * t_5 * t_5 * t_5 * t_5));
}

fn CookTorranceLighting_direct_0( m_2 : MaterialSample_0,  s_7 : SurfaceSample_0,  light_0 : LightSample_0) -> vec3<f32>
{
    var H_1 : vec3<f32> = normalize(s_7.viewDir_0 + light_0.direction_0);
    var _S49 : f32 = max(dot(s_7.normal_2, light_0.direction_0), 0.0f);
    var _S50 : f32 = max(dot(s_7.normal_2, s_7.viewDir_0), 0.00009999999747379f);
    var _S51 : f32 = m_2.roughness_2;
    var F_0 : vec3<f32> = CookTorranceLighting_F_Schlick_0(max(dot(H_1, s_7.viewDir_0), 0.0f), mix(vec3<f32>(0.03999999910593033f), m_2.baseColor_3, vec3<f32>(m_2.metallic_2)));
    return ((vec3<f32>(1.0f) - F_0) * vec3<f32>((1.0f - m_2.metallic_2)) * m_2.baseColor_3 / vec3<f32>(3.14159274101257324f) + vec3<f32>((CookTorranceLighting_D_GGX_0(max(dot(s_7.normal_2, H_1), 0.0f), _S51 * _S51) * (CookTorranceLighting_G_SchlickGGX_0(_S50, m_2.roughness_2) * CookTorranceLighting_G_SchlickGGX_0(_S49, m_2.roughness_2)))) * F_0 / vec3<f32>((4.0f * _S50 * _S49 + 0.00009999999747379f))) * light_0.radiance_0 * vec3<f32>(_S49);
}

fn gammaCorrect_0( c_1 : f32) -> f32
{
    return pow(max(c_1, 0.0f), 0.45454543828964233f);
}

fn tonemapAndGamma_0( color_2 : vec3<f32>) -> vec3<f32>
{
    var tm_0 : vec3<f32> = color_2 / (color_2 + vec3<f32>(1.0f, 1.0f, 1.0f));
    return vec3<f32>(gammaCorrect_0(tm_0.x), gammaCorrect_0(tm_0.y), gammaCorrect_0(tm_0.z));
}

fn CookTorranceLighting_resolve_0( radiance_2 : vec3<f32>,  m_3 : MaterialSample_0) -> vec3<f32>
{
    return tonemapAndGamma_0(radiance_2);
}

fn StandardOutput_finish_0( color_3 : vec4<f32>,  m_4 : MaterialSample_0,  s_8 : SurfaceSample_0) -> vec4<f32>
{
    return color_3;
}

@compute
@workgroup_size(256, 1, 1)
fn pbr_main(@builtin(global_invocation_id) gid_1 : vec3<u32>)
{
    var triIdx_4 : u32 = gid_1.x;
    if(triIdx_4 >= (params_0.numTriangles_0))
    {
        return;
    }
    var r_4 : PBRTriangle_0;
    var _S52 : bool = pbr_setupTriangle_0(triIdx_4, &(r_4));
    if(!_S52)
    {
        return;
    }
    var light_1 : LightSample_0 = LightSample_x24init_0(vec3<f32>(0.0f, 0.0f, 1.0f), vec3<f32>(1.0f));
    light_1.direction_0 = normalize((vec3<f32>(0) - params_0.lightDir_0));
    light_1.radiance_0 = vec3<f32>(params_0.lightIrradiance_0) * params_0.lightColor_0;
    var py_3 : i32 = i32(r_4.t_0.minY_0);
    for(;;)
    {
        if(py_3 <= i32(r_4.t_0.maxY_0))
        {
        }
        else
        {
            break;
        }
        var px_3 : i32 = i32(r_4.t_0.minX_0);
        for(;;)
        {
            if(px_3 <= i32(r_4.t_0.maxX_0))
            {
            }
            else
            {
                break;
            }
            var w_4 : vec3<f32> = barycentric_0(r_4.t_0, px_3, py_3);
            if(!covered_0(w_4))
            {
                px_3 = px_3 + i32(1);
                continue;
            }
            var z_1 : f32 = interpolateDepth_0(r_4.t_0, w_4);
            var _S53 : SurfaceSample_0 = pbr_surfaceSample_0(r_4, w_4, px_3, py_3, z_1);
            var s_9 : SurfaceSample_0 = _S53;
            if(!StandardOutput_keep_0(_S53))
            {
                px_3 = px_3 + i32(1);
                continue;
            }
            var pixIdx_0 : u32 = u32(py_3) * params_0.width_0 + u32(px_3);
            var dU_0 : u32 = depthToU32_0(StandardOutput_depth_0(z_1, s_9));
            var _S54 : u32 = atomicLoad(&(depthBufI_0[pixIdx_0]));
            if(dU_0 > (_S54 + u32(256)))
            {
                px_3 = px_3 + i32(1);
                continue;
            }
            var baseRGB_0 : vec3<f32> = params_0.baseColor_0.xyz;
            var baseRGB_1 : vec3<f32>;
            if((params_0.hasColors_0) != u32(0))
            {
                baseRGB_1 = baseRGB_0 * (r_4.v0_0.color_0 * vec4<f32>(w_4.x) + r_4.v1_0.color_0 * vec4<f32>(w_4.y) + r_4.v2_0.color_0 * vec4<f32>(w_4.z)).xyz;
            }
            else
            {
                baseRGB_1 = baseRGB_0;
            }
            s_9.host_0.baseColor_1 = baseRGB_1;
            s_9.host_0.opacity_0 = params_0.baseColor_0.w;
            s_9.host_0.metallic_0 = params_0.metallicRoughness_0.x;
            s_9.host_0.roughness_0 = params_0.metallicRoughness_0.y;
            s_9.host_0.emissive_1 = params_0.emissive_0.xyz * vec3<f32>(params_0.emissive_0.w);
            s_9.normal_2 = GeometricNormal_normal_0(s_9);
            var _S55 : MaterialSample_0 = HostDrivenMaterial_evaluate_0(s_9);
            var Lo_0 : vec3<f32> = CookTorranceLighting_resolve_0(CookTorranceLighting_ambient_0(_S55, s_9, vec3<f32>(params_0.ambientRadiance_0)) + CookTorranceLighting_direct_0(_S55, s_9, light_1), _S55) + _S55.emissive_3;
            var fragAlpha_0 : f32;
            if((params_0.alphaMode_0) == u32(2))
            {
                if((_S55.opacity_2) < (params_0.alphaCutoff_0))
                {
                    px_3 = px_3 + i32(1);
                    continue;
                }
                fragAlpha_0 = 1.0f;
            }
            else
            {
                if((params_0.alphaMode_0) == u32(1))
                {
                    fragAlpha_0 = _S55.opacity_2;
                }
                else
                {
                    fragAlpha_0 = 1.0f;
                }
            }
            colorBuf_0[pixIdx_0] = StandardOutput_finish_0(vec4<f32>(Lo_0, fragAlpha_0), _S55, s_9);
            px_3 = px_3 + i32(1);
        }
        py_3 = py_3 + i32(1);
    }
    return;
}


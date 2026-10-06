// Generated from shaders/raster.slang with its default specializations
// by vtkSlang (Slang 2026.13.1). Do not edit; regenerate with the
// anari_webgpu_update_wgsl target.

struct _MatrixStorage_float4x4_ColMajorstd140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct RasterUniforms_std140_0
{
    @align(16) mvp_0 : _MatrixStorage_float4x4_ColMajorstd140_0,
    @align(16) materialColor_0 : vec4<f32>,
    @align(16) lightDir_0 : vec3<f32>,
    @align(4) ambientRadiance_0 : f32,
    @align(16) width_0 : u32,
    @align(4) height_0 : u32,
    @align(8) numTriangles_0 : u32,
    @align(4) hasNormals_0 : u32,
    @align(16) hasColors_0 : u32,
    @align(4) _pad0_0 : u32,
    @align(8) _pad1_0 : u32,
    @align(4) _pad2_0 : u32,
};

@binding(2) @group(0) var<uniform> params_0 : RasterUniforms_std140_0;
@binding(6) @group(0) var<storage, read> indices_0 : array<u32>;

@binding(3) @group(0) var<storage, read> positions_0 : array<f32>;

@binding(4) @group(0) var<storage, read> normals_0 : array<f32>;

@binding(5) @group(0) var<storage, read> colors_0 : array<f32>;

@binding(1) @group(0) var<storage, read_write> depthBufI_0 : array<atomic<u32>>;

@binding(0) @group(0) var<storage, read_write> colorBuf_0 : array<vec4<f32>>;

struct FragmentContext_0
{
     positionVC_0 : vec4<f32>,
     normalVC_0 : vec3<f32>,
     tangentVC_0 : vec3<f32>,
     uv_0 : vec2<f32>,
     ambientColor_0 : vec3<f32>,
     diffuseColor_0 : vec3<f32>,
     specularColor_0 : vec3<f32>,
     specularPower_0 : f32,
     opacity_0 : f32,
     color_0 : vec4<f32>,
     depth_0 : f32,
     ids_0 : vec4<u32>,
     fragCoord_0 : vec4<f32>,
     primitiveId_0 : u32,
     cellId_0 : u32,
     frontFacing_0 : bool,
};

fn IShaderHooks_outputx24defaultImpl_0( f_0 : ptr<function, FragmentContext_0>)
{
    return;
}

fn IShaderHooks_litx24defaultImpl_0( f_1 : ptr<function, FragmentContext_0>)
{
    return;
}

fn IShaderHooks_normalx24defaultImpl_0( f_2 : ptr<function, FragmentContext_0>)
{
    return;
}

fn IShaderHooks_colorx24defaultImpl_0( f_3 : ptr<function, FragmentContext_0>)
{
    return;
}

fn IShaderHooks_clipx24defaultImpl_0( f_4 : ptr<function, FragmentContext_0>) -> bool
{
    return true;
}

struct VertexContext_0
{
     positionMC_0 : vec4<f32>,
     normalMC_0 : vec3<f32>,
     tangentMC_0 : vec3<f32>,
     uv_1 : vec2<f32>,
     color_1 : vec4<f32>,
     positionClip_0 : vec4<f32>,
     pointId_0 : u32,
     cellId_1 : u32,
     primitiveId_1 : u32,
     modelToClip_0 : mat4x4<f32>,
     normalModelToView_0 : mat3x3<f32>,
};

fn IShaderHooks_positionx24defaultImpl_0( v_0 : ptr<function, VertexContext_0>)
{
    return;
}

fn IShaderHooks_vertexx24defaultImpl_0( v_1 : ptr<function, VertexContext_0>)
{
    return;
}

struct VertexSample_0
{
     position_0 : vec3<f32>,
     normal_0 : vec3<f32>,
     tangent_0 : vec3<f32>,
     uv_2 : vec2<f32>,
     color_2 : vec4<f32>,
     pointId_1 : u32,
     cellId_2 : u32,
     primitiveId_2 : u32,
     modelToClip_1 : mat4x4<f32>,
};

fn VertexSample_x24init_0( position_1 : vec3<f32>,  normal_1 : vec3<f32>,  tangent_1 : vec3<f32>,  uv_3 : vec2<f32>,  color_3 : vec4<f32>,  pointId_2 : u32,  cellId_3 : u32,  primitiveId_3 : u32,  modelToClip_2 : mat4x4<f32>) -> VertexSample_0
{
    var _S1 : VertexSample_0;
    _S1.position_0 = position_1;
    _S1.normal_0 = normal_1;
    _S1.tangent_0 = tangent_1;
    _S1.uv_2 = uv_3;
    _S1.color_2 = color_3;
    _S1.pointId_1 = pointId_2;
    _S1.cellId_2 = cellId_3;
    _S1.primitiveId_2 = primitiveId_3;
    _S1.modelToClip_1 = modelToClip_2;
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

struct Triangle_0
{
     v0_0 : VertexSample_0,
     v1_0 : VertexSample_0,
     v2_0 : VertexSample_0,
     t_0 : ScreenTriangle_0,
     faceNormal_0 : vec3<f32>,
     id_0 : u32,
};

fn Triangle_x24init_0( v0_1 : VertexSample_0,  v1_1 : VertexSample_0,  v2_1 : VertexSample_0,  t_1 : ScreenTriangle_0,  faceNormal_1 : vec3<f32>,  id_1 : u32) -> Triangle_0
{
    var _S3 : Triangle_0;
    _S3.v0_0 = v0_1;
    _S3.v1_0 = v1_1;
    _S3.v2_0 = v2_1;
    _S3.t_0 = t_1;
    _S3.faceNormal_0 = faceNormal_1;
    _S3.id_0 = id_1;
    return _S3;
}

fn triangleIndices_0( triIdx_0 : u32) -> vec3<u32>
{
    var _S4 : u32 = triIdx_0 * u32(3);
    return vec3<u32>(indices_0[_S4], indices_0[_S4 + u32(1)], indices_0[_S4 + u32(2)]);
}

fn getPosition_0( i_0 : u32) -> vec3<f32>
{
    var _S5 : u32 = i_0 * u32(3);
    return vec3<f32>(positions_0[_S5], positions_0[_S5 + u32(1)], positions_0[_S5 + u32(2)]);
}

fn getNormal_0( i_1 : u32) -> vec3<f32>
{
    var _S6 : u32 = i_1 * u32(3);
    return vec3<f32>(normals_0[_S6], normals_0[_S6 + u32(1)], normals_0[_S6 + u32(2)]);
}

fn getColor_0( i_2 : u32) -> vec4<f32>
{
    var _S7 : u32 = i_2 * u32(4);
    return vec4<f32>(colors_0[_S7], colors_0[_S7 + u32(1)], colors_0[_S7 + u32(2)], colors_0[_S7 + u32(3)]);
}

fn VertexContext_x24init_0( positionMC_1 : vec4<f32>,  normalMC_1 : vec3<f32>,  tangentMC_1 : vec3<f32>,  uv_4 : vec2<f32>,  color_4 : vec4<f32>,  positionClip_1 : vec4<f32>,  pointId_3 : u32,  cellId_4 : u32,  primitiveId_4 : u32,  modelToClip_3 : mat4x4<f32>,  normalModelToView_1 : mat3x3<f32>) -> VertexContext_0
{
    var _S8 : VertexContext_0;
    _S8.positionMC_0 = positionMC_1;
    _S8.normalMC_0 = normalMC_1;
    _S8.tangentMC_0 = tangentMC_1;
    _S8.uv_1 = uv_4;
    _S8.color_1 = color_4;
    _S8.positionClip_0 = positionClip_1;
    _S8.pointId_0 = pointId_3;
    _S8.cellId_1 = cellId_4;
    _S8.primitiveId_1 = primitiveId_4;
    _S8.modelToClip_0 = modelToClip_3;
    _S8.normalModelToView_0 = normalModelToView_1;
    return _S8;
}

fn vertexContext_0( v_2 : VertexSample_0) -> VertexContext_0
{
    const _S9 : vec4<f32> = vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f);
    var c_0 : VertexContext_0 = VertexContext_x24init_0(_S9, vec3<f32>(0.0f, 0.0f, 1.0f), vec3<f32>(0.0f), vec2<f32>(0.0f), vec4<f32>(1.0f), _S9, u32(0), u32(0), u32(0), mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f), mat3x3<f32>(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    c_0.positionMC_0 = vec4<f32>(v_2.position_0, 1.0f);
    c_0.normalMC_0 = v_2.normal_0;
    c_0.tangentMC_0 = v_2.tangent_0;
    c_0.uv_1 = v_2.uv_2;
    c_0.color_1 = v_2.color_2;
    c_0.pointId_0 = v_2.pointId_1;
    c_0.cellId_1 = v_2.cellId_2;
    c_0.primitiveId_1 = v_2.primitiveId_2;
    c_0.modelToClip_0 = mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]);
    return c_0;
}

fn IdentityVertex_modify_0( v_3 : VertexSample_0) -> VertexSample_0
{
    return v_3;
}

fn fetchVertex_0( triIdx_1 : u32,  idx_0 : u32) -> VertexSample_0
{
    var _S10 : vec3<f32> = vec3<f32>(0.0f);
    var v_4 : VertexSample_0 = VertexSample_x24init_0(_S10, vec3<f32>(0.0f, 0.0f, 1.0f), _S10, vec2<f32>(0.0f), vec4<f32>(1.0f), u32(0), u32(0), u32(0), mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    v_4.position_0 = getPosition_0(idx_0);
    if((params_0.hasNormals_0) != u32(0))
    {
        v_4.normal_0 = getNormal_0(idx_0);
    }
    if((params_0.hasColors_0) != u32(0))
    {
        v_4.color_2 = getColor_0(idx_0);
    }
    v_4.pointId_1 = idx_0;
    v_4.cellId_2 = triIdx_1;
    v_4.primitiveId_2 = triIdx_1;
    v_4.modelToClip_1 = mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]);
    var _S11 : VertexSample_0 = IdentityVertex_modify_0(v_4);
    v_4 = _S11;
    var c_1 : VertexContext_0 = vertexContext_0(_S11);
    IShaderHooks_vertexx24defaultImpl_0(&(c_1));
    v_4.position_0 = c_1.positionMC_0.xyz;
    v_4.normal_0 = c_1.normalMC_0;
    v_4.tangent_0 = c_1.tangentMC_0;
    v_4.uv_2 = c_1.uv_1;
    v_4.color_2 = c_1.color_1;
    return v_4;
}

fn project_0( v_5 : VertexSample_0) -> vec4<f32>
{
    var c_2 : VertexContext_0 = vertexContext_0(v_5);
    c_2.positionClip_0 = (((c_2.positionMC_0) * (mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]))));
    IShaderHooks_positionx24defaultImpl_0(&(c_2));
    return c_2.positionClip_0;
}

fn edgeFunction_0( a_0 : vec2<f32>,  b_0 : vec2<f32>,  c_3 : vec2<f32>) -> f32
{
    var _S12 : f32 = a_0.x;
    var _S13 : f32 = a_0.y;
    return (c_3.x - _S12) * (b_0.y - _S13) - (c_3.y - _S13) * (b_0.x - _S12);
}

fn setupScreenTriangleClip_0( c0_0 : vec4<f32>,  c1_0 : vec4<f32>,  c2_0 : vec4<f32>,  width_1 : u32,  height_1 : u32,  t_2 : ptr<function, ScreenTriangle_0>) -> bool
{
    var _S14 : vec3<f32> = vec3<f32>(0.0f);
    (*t_2) = ScreenTriangle_x24init_0(_S14, _S14, _S14, _S14, _S14, _S14, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    var _S15 : f32 = c0_0.w;
    var _S16 : bool;
    if(_S15 <= 0.0f)
    {
        _S16 = true;
    }
    else
    {
        _S16 = (c1_0.w) <= 0.0f;
    }
    if(_S16)
    {
        _S16 = true;
    }
    else
    {
        _S16 = (c2_0.w) <= 0.0f;
    }
    if(_S16)
    {
        return false;
    }
    var _S17 : vec3<f32> = c0_0.xyz / vec3<f32>(_S15);
    (*t_2).ndc0_0 = _S17;
    var _S18 : vec3<f32> = c1_0.xyz / vec3<f32>(c1_0.w);
    (*t_2).ndc1_0 = _S18;
    var _S19 : vec3<f32> = c2_0.xyz / vec3<f32>(c2_0.w);
    (*t_2).ndc2_0 = _S19;
    var W_0 : f32 = f32(width_1);
    var H_0 : f32 = f32(height_1);
    var _S20 : f32 = (_S17.x * 0.5f + 0.5f) * W_0;
    var _S21 : f32 = (1.0f - (_S17.y * 0.5f + 0.5f)) * H_0;
    (*t_2).s0_0 = vec3<f32>(_S20, _S21, _S17.z);
    var _S22 : f32 = (_S18.x * 0.5f + 0.5f) * W_0;
    var _S23 : f32 = (1.0f - (_S18.y * 0.5f + 0.5f)) * H_0;
    (*t_2).s1_0 = vec3<f32>(_S22, _S23, _S18.z);
    var _S24 : f32 = (_S19.x * 0.5f + 0.5f) * W_0;
    var _S25 : f32 = (1.0f - (_S19.y * 0.5f + 0.5f)) * H_0;
    (*t_2).s2_0 = vec3<f32>(_S24, _S25, _S19.z);
    (*t_2).minX_0 = max(0.0f, min(min(_S20, _S22), _S24));
    (*t_2).maxX_0 = min(W_0 - 1.0f, max(max(_S20, _S22), _S24));
    (*t_2).minY_0 = max(0.0f, min(min(_S21, _S23), _S25));
    (*t_2).maxY_0 = min(H_0 - 1.0f, max(max(_S21, _S23), _S25));
    var _S26 : f32 = edgeFunction_0(vec2<f32>(_S20, _S21), vec2<f32>(_S22, _S23), vec2<f32>(_S24, _S25));
    (*t_2).area_0 = _S26;
    return (abs(_S26)) >= 9.99999997475242708e-07f;
}

fn setupTriangle_0( triIdx_2 : u32,  r_0 : ptr<function, Triangle_0>) -> bool
{
    var _S27 : vec3<f32> = vec3<f32>(0.0f);
    var _S28 : VertexSample_0 = VertexSample_x24init_0(_S27, vec3<f32>(0.0f, 0.0f, 1.0f), _S27, vec2<f32>(0.0f), vec4<f32>(1.0f), u32(0), u32(0), u32(0), mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    (*r_0) = Triangle_x24init_0(_S28, _S28, _S28, ScreenTriangle_x24init_0(_S27, _S27, _S27, _S27, _S27, _S27, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), _S27, u32(0));
    (*r_0).id_0 = triIdx_2;
    var i_3 : vec3<u32> = triangleIndices_0(triIdx_2);
    var _S29 : VertexSample_0 = fetchVertex_0(triIdx_2, i_3.x);
    (*r_0).v0_0 = _S29;
    var _S30 : VertexSample_0 = fetchVertex_0(triIdx_2, i_3.y);
    (*r_0).v1_0 = _S30;
    var _S31 : VertexSample_0 = fetchVertex_0(triIdx_2, i_3.z);
    (*r_0).v2_0 = _S31;
    var _S32 : vec4<f32> = project_0(_S29);
    var _S33 : vec4<f32> = project_0(_S30);
    var _S34 : vec4<f32> = project_0(_S31);
    var _S35 : ScreenTriangle_0 = (*r_0).t_0;
    var _S36 : bool = setupScreenTriangleClip_0(_S32, _S33, _S34, params_0.width_0, params_0.height_0, &(_S35));
    (*r_0).t_0 = _S35;
    if(!_S36)
    {
        return false;
    }
    if((params_0.hasNormals_0) != u32(0))
    {
        (*r_0).faceNormal_0 = normalize(((*r_0).v0_0.normal_0 + (*r_0).v1_0.normal_0 + (*r_0).v2_0.normal_0) / vec3<f32>(3.0f));
    }
    else
    {
        (*r_0).faceNormal_0 = normalize(cross((*r_0).v1_0.position_0 - (*r_0).v0_0.position_0, (*r_0).v2_0.position_0 - (*r_0).v0_0.position_0));
    }
    return true;
}

fn barycentric_0( t_3 : ScreenTriangle_0,  px_0 : i32,  py_0 : i32) -> vec3<f32>
{
    var p_0 : vec2<f32> = vec2<f32>(f32(px_0) + 0.5f, f32(py_0) + 0.5f);
    var _S37 : vec2<f32> = t_3.s1_0.xy;
    var _S38 : vec2<f32> = t_3.s2_0.xy;
    var _S39 : vec2<f32> = t_3.s0_0.xy;
    return vec3<f32>(edgeFunction_0(_S37, _S38, p_0) / t_3.area_0, edgeFunction_0(_S38, _S39, p_0) / t_3.area_0, edgeFunction_0(_S39, _S37, p_0) / t_3.area_0);
}

fn covered_0( w_0 : vec3<f32>) -> bool
{
    var _S40 : bool;
    if((w_0.x) >= 0.0f)
    {
        _S40 = (w_0.y) >= 0.0f;
    }
    else
    {
        _S40 = false;
    }
    if(_S40)
    {
        _S40 = (w_0.z) >= 0.0f;
    }
    else
    {
        _S40 = false;
    }
    return _S40;
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
    var _S41 : CameraSample_0;
    _S41.shadingToClip_0 = shadingToClip_1;
    _S41.parallel_0 = parallel_1;
    return _S41;
}

struct HostMaterial_0
{
     baseColor_0 : vec3<f32>,
     opacity_1 : f32,
     metallic_0 : f32,
     roughness_0 : f32,
     emissive_0 : vec3<f32>,
     ambientColor_1 : vec3<f32>,
     specularColor_1 : vec3<f32>,
     specularPower_1 : f32,
};

fn HostMaterial_x24init_0( baseColor_1 : vec3<f32>,  opacity_2 : f32,  metallic_1 : f32,  roughness_1 : f32,  emissive_1 : vec3<f32>,  ambientColor_2 : vec3<f32>,  specularColor_2 : vec3<f32>,  specularPower_2 : f32) -> HostMaterial_0
{
    var _S42 : HostMaterial_0;
    _S42.baseColor_0 = baseColor_1;
    _S42.opacity_1 = opacity_2;
    _S42.metallic_0 = metallic_1;
    _S42.roughness_0 = roughness_1;
    _S42.emissive_0 = emissive_1;
    _S42.ambientColor_1 = ambientColor_2;
    _S42.specularColor_1 = specularColor_2;
    _S42.specularPower_1 = specularPower_2;
    return _S42;
}

struct SurfaceSample_0
{
     position_2 : vec3<f32>,
     normal_2 : vec3<f32>,
     tangent_2 : vec3<f32>,
     viewDir_0 : vec3<f32>,
     uv_5 : vec2<f32>,
     scalar_0 : f32,
     fragCoord_1 : vec4<f32>,
     primitiveId_5 : u32,
     cellId_5 : u32,
     frontFacing_1 : bool,
     userData_0 : vec4<u32>,
     camera_0 : CameraSample_0,
     host_0 : HostMaterial_0,
};

fn SurfaceSample_x24init_0( position_3 : vec3<f32>,  normal_3 : vec3<f32>,  tangent_3 : vec3<f32>,  viewDir_1 : vec3<f32>,  uv_6 : vec2<f32>,  scalar_1 : f32,  fragCoord_2 : vec4<f32>,  primitiveId_6 : u32,  cellId_6 : u32,  frontFacing_2 : bool,  userData_1 : vec4<u32>,  camera_1 : CameraSample_0,  host_1 : HostMaterial_0) -> SurfaceSample_0
{
    var _S43 : SurfaceSample_0;
    _S43.position_2 = position_3;
    _S43.normal_2 = normal_3;
    _S43.tangent_2 = tangent_3;
    _S43.viewDir_0 = viewDir_1;
    _S43.uv_5 = uv_6;
    _S43.scalar_0 = scalar_1;
    _S43.fragCoord_1 = fragCoord_2;
    _S43.primitiveId_5 = primitiveId_6;
    _S43.cellId_5 = cellId_6;
    _S43.frontFacing_1 = frontFacing_2;
    _S43.userData_0 = userData_1;
    _S43.camera_0 = camera_1;
    _S43.host_0 = host_1;
    return _S43;
}

fn surfaceSample_0( r_1 : Triangle_0,  w_2 : vec3<f32>,  px_1 : i32,  py_1 : i32,  depth_1 : f32) -> SurfaceSample_0
{
    var _S44 : vec3<f32> = vec3<f32>(0.0f);
    const _S45 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 1.0f);
    var s_0 : SurfaceSample_0 = SurfaceSample_x24init_0(_S44, _S45, _S44, _S45, vec2<f32>(0.0f), 0.0f, vec4<f32>(0.0f), u32(0), u32(0), true, vec4<u32>(u32(0)), CameraSample_x24init_0(mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f), false), HostMaterial_x24init_0(vec3<f32>(1.0f), 1.0f, 0.0f, 1.0f, _S44, _S44, _S44, 1.0f));
    var _S46 : f32 = w_2.x;
    var _S47 : f32 = w_2.y;
    var _S48 : f32 = w_2.z;
    s_0.position_2 = r_1.v0_0.position_0 * vec3<f32>(_S46) + r_1.v1_0.position_0 * vec3<f32>(_S47) + r_1.v2_0.position_0 * vec3<f32>(_S48);
    s_0.normal_2 = r_1.faceNormal_0;
    s_0.uv_5 = r_1.v0_0.uv_2 * vec2<f32>(_S46) + r_1.v1_0.uv_2 * vec2<f32>(_S47) + r_1.v2_0.uv_2 * vec2<f32>(_S48);
    s_0.fragCoord_1 = vec4<f32>(f32(px_1) + 0.5f, f32(py_1) + 0.5f, depth_1, 1.0f);
    s_0.primitiveId_5 = r_1.id_0;
    s_0.cellId_5 = r_1.id_0;
    s_0.frontFacing_1 = (r_1.t_0.area_0) < 0.0f;
    s_0.camera_0.shadingToClip_0 = mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]);
    return s_0;
}

fn FragmentContext_x24init_0( positionVC_1 : vec4<f32>,  normalVC_1 : vec3<f32>,  tangentVC_1 : vec3<f32>,  uv_7 : vec2<f32>,  ambientColor_3 : vec3<f32>,  diffuseColor_1 : vec3<f32>,  specularColor_3 : vec3<f32>,  specularPower_3 : f32,  opacity_3 : f32,  color_5 : vec4<f32>,  depth_2 : f32,  ids_1 : vec4<u32>,  fragCoord_3 : vec4<f32>,  primitiveId_7 : u32,  cellId_7 : u32,  frontFacing_3 : bool) -> FragmentContext_0
{
    var _S49 : FragmentContext_0;
    _S49.positionVC_0 = positionVC_1;
    _S49.normalVC_0 = normalVC_1;
    _S49.tangentVC_0 = tangentVC_1;
    _S49.uv_0 = uv_7;
    _S49.ambientColor_0 = ambientColor_3;
    _S49.diffuseColor_0 = diffuseColor_1;
    _S49.specularColor_0 = specularColor_3;
    _S49.specularPower_0 = specularPower_3;
    _S49.opacity_0 = opacity_3;
    _S49.color_0 = color_5;
    _S49.depth_0 = depth_2;
    _S49.ids_0 = ids_1;
    _S49.fragCoord_0 = fragCoord_3;
    _S49.primitiveId_0 = primitiveId_7;
    _S49.cellId_0 = cellId_7;
    _S49.frontFacing_0 = frontFacing_3;
    return _S49;
}

fn fragmentContext_0( s_1 : SurfaceSample_0) -> FragmentContext_0
{
    const _S50 : vec4<f32> = vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f);
    var _S51 : vec3<f32> = vec3<f32>(0.0f);
    var f_5 : FragmentContext_0 = FragmentContext_x24init_0(_S50, vec3<f32>(0.0f, 0.0f, 1.0f), _S51, vec2<f32>(0.0f), _S51, vec3<f32>(1.0f), _S51, 1.0f, 1.0f, _S50, 0.0f, vec4<u32>(u32(0)), vec4<f32>(0.0f), u32(0), u32(0), true);
    f_5.positionVC_0 = vec4<f32>(s_1.position_2, 1.0f);
    f_5.normalVC_0 = s_1.normal_2;
    f_5.tangentVC_0 = s_1.tangent_2;
    f_5.uv_0 = s_1.uv_5;
    f_5.fragCoord_0 = s_1.fragCoord_1;
    f_5.primitiveId_0 = s_1.primitiveId_5;
    f_5.cellId_0 = s_1.cellId_5;
    f_5.frontFacing_0 = s_1.frontFacing_1;
    f_5.depth_0 = s_1.fragCoord_1.z;
    return f_5;
}

fn StandardOutput_keep_0( s_2 : SurfaceSample_0) -> bool
{
    return true;
}

fn StandardOutput_depth_0( depth_3 : f32,  s_3 : SurfaceSample_0) -> f32
{
    return depth_3;
}

fn fragmentDepth_0( s_4 : SurfaceSample_0,  z_0 : f32,  depth_4 : ptr<function, f32>) -> bool
{
    var f_6 : FragmentContext_0 = fragmentContext_0(s_4);
    (*depth_4) = z_0;
    var _S52 : bool = IShaderHooks_clipx24defaultImpl_0(&(f_6));
    var _S53 : bool;
    if(!_S52)
    {
        _S53 = true;
    }
    else
    {
        _S53 = !StandardOutput_keep_0(s_4);
    }
    if(_S53)
    {
        return false;
    }
    f_6.depth_0 = StandardOutput_depth_0(z_0, s_4);
    IShaderHooks_outputx24defaultImpl_0(&(f_6));
    (*depth_4) = f_6.depth_0;
    return true;
}

fn depthToU32_0( d_0 : f32) -> u32
{
    return (bitcast<u32>((clamp(d_0, 0.0f, 1.0f))));
}

@compute
@workgroup_size(256, 1, 1)
fn depth_main(@builtin(global_invocation_id) gid_0 : vec3<u32>)
{
    var triIdx_3 : u32 = gid_0.x;
    if(triIdx_3 >= (params_0.numTriangles_0))
    {
        return;
    }
    var r_2 : Triangle_0;
    var _S54 : bool = setupTriangle_0(triIdx_3, &(r_2));
    if(!_S54)
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
            var z_1 : f32 = interpolateDepth_0(r_2.t_0, w_3);
            var depth_5 : f32;
            var _S55 : bool = fragmentDepth_0(surfaceSample_0(r_2, w_3, px_2, py_2, z_1), z_1, &(depth_5));
            if(!_S55)
            {
                px_2 = px_2 + i32(1);
                continue;
            }
            var _S56 : u32 = atomicMin(&(depthBufI_0[u32(py_2) * params_0.width_0 + u32(px_2)]), depthToU32_0(depth_5));
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
    var _S57 : LightSample_0;
    _S57.direction_0 = direction_1;
    _S57.radiance_0 = radiance_1;
    return _S57;
}

fn GeometricNormal_normal_0( s_5 : SurfaceSample_0) -> vec3<f32>
{
    return s_5.normal_2;
}

struct MaterialSample_0
{
     baseColor_2 : vec3<f32>,
     opacity_4 : f32,
     metallic_2 : f32,
     roughness_2 : f32,
     emissive_2 : vec3<f32>,
     ambientColor_4 : vec3<f32>,
     specularColor_4 : vec3<f32>,
     specularPower_4 : f32,
};

fn MaterialSample_x24init_0( baseColor_3 : vec3<f32>,  opacity_5 : f32,  metallic_3 : f32,  roughness_3 : f32,  emissive_3 : vec3<f32>,  ambientColor_5 : vec3<f32>,  specularColor_5 : vec3<f32>,  specularPower_5 : f32) -> MaterialSample_0
{
    var _S58 : MaterialSample_0;
    _S58.baseColor_2 = baseColor_3;
    _S58.opacity_4 = opacity_5;
    _S58.metallic_2 = metallic_3;
    _S58.roughness_2 = roughness_3;
    _S58.emissive_2 = emissive_3;
    _S58.ambientColor_4 = ambientColor_5;
    _S58.specularColor_4 = specularColor_5;
    _S58.specularPower_4 = specularPower_5;
    return _S58;
}

fn HostDrivenMaterial_evaluate_0( s_6 : SurfaceSample_0) -> MaterialSample_0
{
    var _S59 : vec3<f32> = vec3<f32>(0.0f);
    var m_0 : MaterialSample_0 = MaterialSample_x24init_0(vec3<f32>(1.0f), 1.0f, 0.0f, 1.0f, _S59, _S59, _S59, 1.0f);
    m_0.baseColor_2 = s_6.host_0.baseColor_0;
    m_0.opacity_4 = s_6.host_0.opacity_1;
    m_0.metallic_2 = s_6.host_0.metallic_0;
    m_0.roughness_2 = s_6.host_0.roughness_0;
    m_0.emissive_2 = s_6.host_0.emissive_0;
    m_0.ambientColor_4 = s_6.host_0.ambientColor_1;
    m_0.specularColor_4 = s_6.host_0.specularColor_1;
    m_0.specularPower_4 = s_6.host_0.specularPower_1;
    return m_0;
}

fn LambertLighting_ambient_0( m_1 : MaterialSample_0,  s_7 : SurfaceSample_0,  ambientRadiance_1 : vec3<f32>) -> vec3<f32>
{
    return m_1.baseColor_2 * ambientRadiance_1;
}

fn LambertLighting_direct_0( m_2 : MaterialSample_0,  s_8 : SurfaceSample_0,  light_0 : LightSample_0) -> vec3<f32>
{
    return m_2.baseColor_2 * light_0.radiance_0 * vec3<f32>(max(dot(s_8.normal_2, light_0.direction_0), 0.0f));
}

fn LambertLighting_resolve_0( radiance_2 : vec3<f32>,  m_3 : MaterialSample_0) -> vec3<f32>
{
    return min(radiance_2, m_3.baseColor_2);
}

fn StandardOutput_finish_0( color_6 : vec4<f32>,  m_4 : MaterialSample_0,  s_9 : SurfaceSample_0) -> vec4<f32>
{
    return color_6;
}

@compute
@workgroup_size(256, 1, 1)
fn main(@builtin(global_invocation_id) gid_1 : vec3<u32>)
{
    var triIdx_4 : u32 = gid_1.x;
    if(triIdx_4 >= (params_0.numTriangles_0))
    {
        return;
    }
    var r_3 : Triangle_0;
    var _S60 : bool = setupTriangle_0(triIdx_4, &(r_3));
    if(!_S60)
    {
        return;
    }
    var _S61 : vec3<f32> = vec3<f32>(1.0f);
    var light_1 : LightSample_0 = LightSample_x24init_0(vec3<f32>(0.0f, 0.0f, 1.0f), _S61);
    light_1.direction_0 = normalize((vec3<f32>(0) - params_0.lightDir_0));
    light_1.radiance_0 = _S61;
    var _S62 : vec4<f32> = r_3.v0_0.color_2 * params_0.materialColor_0;
    var _S63 : vec4<f32> = r_3.v1_0.color_2 * params_0.materialColor_0;
    var _S64 : vec4<f32> = r_3.v2_0.color_2 * params_0.materialColor_0;
    var py_3 : i32 = i32(r_3.t_0.minY_0);
    for(;;)
    {
        if(py_3 <= i32(r_3.t_0.maxY_0))
        {
        }
        else
        {
            break;
        }
        var px_3 : i32 = i32(r_3.t_0.minX_0);
        for(;;)
        {
            if(px_3 <= i32(r_3.t_0.maxX_0))
            {
            }
            else
            {
                break;
            }
            var w_4 : vec3<f32> = barycentric_0(r_3.t_0, px_3, py_3);
            if(!covered_0(w_4))
            {
                px_3 = px_3 + i32(1);
                continue;
            }
            var z_2 : f32 = interpolateDepth_0(r_3.t_0, w_4);
            var _S65 : SurfaceSample_0 = surfaceSample_0(r_3, w_4, px_3, py_3, z_2);
            var s_10 : SurfaceSample_0 = _S65;
            var depth_6 : f32;
            var _S66 : bool = fragmentDepth_0(_S65, z_2, &(depth_6));
            if(!_S66)
            {
                px_3 = px_3 + i32(1);
                continue;
            }
            var pixIdx_0 : u32 = u32(py_3) * params_0.width_0 + u32(px_3);
            var _S67 : u32 = depthToU32_0(depth_6);
            var _S68 : u32 = atomicLoad(&(depthBufI_0[pixIdx_0]));
            if(_S67 > (_S68 + u32(256)))
            {
                px_3 = px_3 + i32(1);
                continue;
            }
            var color_7 : vec4<f32> = _S62 * vec4<f32>(w_4.x) + _S63 * vec4<f32>(w_4.y) + _S64 * vec4<f32>(w_4.z);
            var f_7 : FragmentContext_0 = fragmentContext_0(s_10);
            f_7.diffuseColor_0 = color_7.xyz;
            f_7.opacity_0 = color_7.w;
            IShaderHooks_colorx24defaultImpl_0(&(f_7));
            IShaderHooks_normalx24defaultImpl_0(&(f_7));
            s_10.host_0.baseColor_0 = f_7.diffuseColor_0;
            s_10.host_0.opacity_1 = f_7.opacity_0;
            s_10.normal_2 = f_7.normalVC_0;
            s_10.normal_2 = GeometricNormal_normal_0(s_10);
            var _S69 : MaterialSample_0 = HostDrivenMaterial_evaluate_0(s_10);
            f_7.color_0 = vec4<f32>(LambertLighting_resolve_0(LambertLighting_ambient_0(_S69, s_10, vec3<f32>(params_0.ambientRadiance_0)) + LambertLighting_direct_0(_S69, s_10, light_1), _S69) + _S69.emissive_2, _S69.opacity_4);
            IShaderHooks_litx24defaultImpl_0(&(f_7));
            f_7.color_0 = StandardOutput_finish_0(f_7.color_0, _S69, s_10);
            f_7.depth_0 = depth_6;
            IShaderHooks_outputx24defaultImpl_0(&(f_7));
            colorBuf_0[pixIdx_0] = f_7.color_0;
            px_3 = px_3 + i32(1);
        }
        py_3 = py_3 + i32(1);
    }
    return;
}


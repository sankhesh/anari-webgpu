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

@binding(1) @group(0) var<storage, read_write> depthBufI_0 : array<atomic<u32>>;

@binding(4) @group(0) var<storage, read> normals_0 : array<f32>;

@binding(5) @group(0) var<storage, read> colors_0 : array<f32>;

@binding(0) @group(0) var<storage, read_write> colorBuf_0 : array<vec4<f32>>;

fn triangleIndices_0( triIdx_0 : u32) -> vec3<u32>
{
    var _S1 : u32 = triIdx_0 * u32(3);
    return vec3<u32>(indices_0[_S1], indices_0[_S1 + u32(1)], indices_0[_S1 + u32(2)]);
}

fn getPosition_0( i_0 : u32) -> vec3<f32>
{
    var _S2 : u32 = i_0 * u32(3);
    return vec3<f32>(positions_0[_S2], positions_0[_S2 + u32(1)], positions_0[_S2 + u32(2)]);
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
    var _S3 : ScreenTriangle_0;
    _S3.s0_0 = s0_1;
    _S3.s1_0 = s1_1;
    _S3.s2_0 = s2_1;
    _S3.ndc0_0 = ndc0_1;
    _S3.ndc1_0 = ndc1_1;
    _S3.ndc2_0 = ndc2_1;
    _S3.area_0 = area_1;
    _S3.minX_0 = minX_1;
    _S3.maxX_0 = maxX_1;
    _S3.minY_0 = minY_1;
    _S3.maxY_0 = maxY_1;
    return _S3;
}

fn edgeFunction_0( a_0 : vec2<f32>,  b_0 : vec2<f32>,  c_0 : vec2<f32>) -> f32
{
    var _S4 : f32 = a_0.x;
    var _S5 : f32 = a_0.y;
    return (c_0.x - _S4) * (b_0.y - _S5) - (c_0.y - _S5) * (b_0.x - _S4);
}

fn setupScreenTriangle_0( mvp_1 : mat4x4<f32>,  p0_0 : vec3<f32>,  p1_0 : vec3<f32>,  p2_0 : vec3<f32>,  width_1 : u32,  height_1 : u32,  t_0 : ptr<function, ScreenTriangle_0>) -> bool
{
    var _S6 : vec3<f32> = vec3<f32>(0.0f);
    (*t_0) = ScreenTriangle_x24init_0(_S6, _S6, _S6, _S6, _S6, _S6, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    var c0_0 : vec4<f32> = (((vec4<f32>(p0_0, 1.0f)) * (mvp_1)));
    var c1_0 : vec4<f32> = (((vec4<f32>(p1_0, 1.0f)) * (mvp_1)));
    var c2_0 : vec4<f32> = (((vec4<f32>(p2_0, 1.0f)) * (mvp_1)));
    var _S7 : f32 = c0_0.w;
    var _S8 : bool;
    if(_S7 <= 0.0f)
    {
        _S8 = true;
    }
    else
    {
        _S8 = (c1_0.w) <= 0.0f;
    }
    if(_S8)
    {
        _S8 = true;
    }
    else
    {
        _S8 = (c2_0.w) <= 0.0f;
    }
    if(_S8)
    {
        return false;
    }
    var _S9 : vec3<f32> = c0_0.xyz / vec3<f32>(_S7);
    (*t_0).ndc0_0 = _S9;
    var _S10 : vec3<f32> = c1_0.xyz / vec3<f32>(c1_0.w);
    (*t_0).ndc1_0 = _S10;
    var _S11 : vec3<f32> = c2_0.xyz / vec3<f32>(c2_0.w);
    (*t_0).ndc2_0 = _S11;
    var W_0 : f32 = f32(width_1);
    var H_0 : f32 = f32(height_1);
    var _S12 : f32 = (_S9.x * 0.5f + 0.5f) * W_0;
    var _S13 : f32 = (1.0f - (_S9.y * 0.5f + 0.5f)) * H_0;
    (*t_0).s0_0 = vec3<f32>(_S12, _S13, _S9.z);
    var _S14 : f32 = (_S10.x * 0.5f + 0.5f) * W_0;
    var _S15 : f32 = (1.0f - (_S10.y * 0.5f + 0.5f)) * H_0;
    (*t_0).s1_0 = vec3<f32>(_S14, _S15, _S10.z);
    var _S16 : f32 = (_S11.x * 0.5f + 0.5f) * W_0;
    var _S17 : f32 = (1.0f - (_S11.y * 0.5f + 0.5f)) * H_0;
    (*t_0).s2_0 = vec3<f32>(_S16, _S17, _S11.z);
    (*t_0).minX_0 = max(0.0f, min(min(_S12, _S14), _S16));
    (*t_0).maxX_0 = min(W_0 - 1.0f, max(max(_S12, _S14), _S16));
    (*t_0).minY_0 = max(0.0f, min(min(_S13, _S15), _S17));
    (*t_0).maxY_0 = min(H_0 - 1.0f, max(max(_S13, _S15), _S17));
    var _S18 : f32 = edgeFunction_0(vec2<f32>(_S12, _S13), vec2<f32>(_S14, _S15), vec2<f32>(_S16, _S17));
    (*t_0).area_0 = _S18;
    return (abs(_S18)) >= 9.99999997475242708e-07f;
}

fn barycentric_0( t_1 : ScreenTriangle_0,  px_0 : i32,  py_0 : i32) -> vec3<f32>
{
    var p_0 : vec2<f32> = vec2<f32>(f32(px_0) + 0.5f, f32(py_0) + 0.5f);
    var _S19 : vec2<f32> = t_1.s1_0.xy;
    var _S20 : vec2<f32> = t_1.s2_0.xy;
    var _S21 : vec2<f32> = t_1.s0_0.xy;
    return vec3<f32>(edgeFunction_0(_S19, _S20, p_0) / t_1.area_0, edgeFunction_0(_S20, _S21, p_0) / t_1.area_0, edgeFunction_0(_S21, _S19, p_0) / t_1.area_0);
}

fn covered_0( w_0 : vec3<f32>) -> bool
{
    var _S22 : bool;
    if((w_0.x) >= 0.0f)
    {
        _S22 = (w_0.y) >= 0.0f;
    }
    else
    {
        _S22 = false;
    }
    if(_S22)
    {
        _S22 = (w_0.z) >= 0.0f;
    }
    else
    {
        _S22 = false;
    }
    return _S22;
}

fn interpolateDepth_0( t_2 : ScreenTriangle_0,  w_1 : vec3<f32>) -> f32
{
    return w_1.x * t_2.s0_0.z + w_1.y * t_2.s1_0.z + w_1.z * t_2.s2_0.z;
}

fn depthToU32_0( d_0 : f32) -> u32
{
    return (bitcast<u32>((clamp(d_0, 0.0f, 1.0f))));
}

@compute
@workgroup_size(256, 1, 1)
fn depth_main(@builtin(global_invocation_id) gid_0 : vec3<u32>)
{
    var triIdx_1 : u32 = gid_0.x;
    if(triIdx_1 >= (params_0.numTriangles_0))
    {
        return;
    }
    var i_1 : vec3<u32> = triangleIndices_0(triIdx_1);
    var t_3 : ScreenTriangle_0;
    var _S23 : bool = setupScreenTriangle_0(mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]), getPosition_0(i_1.x), getPosition_0(i_1.y), getPosition_0(i_1.z), params_0.width_0, params_0.height_0, &(t_3));
    if(!_S23)
    {
        return;
    }
    var py_1 : i32 = i32(t_3.minY_0);
    for(;;)
    {
        if(py_1 <= i32(t_3.maxY_0))
        {
        }
        else
        {
            break;
        }
        var px_1 : i32 = i32(t_3.minX_0);
        for(;;)
        {
            if(px_1 <= i32(t_3.maxX_0))
            {
            }
            else
            {
                break;
            }
            var w_2 : vec3<f32> = barycentric_0(t_3, px_1, py_1);
            if(!covered_0(w_2))
            {
                px_1 = px_1 + i32(1);
                continue;
            }
            var _S24 : u32 = atomicMin(&(depthBufI_0[u32(py_1) * params_0.width_0 + u32(px_1)]), depthToU32_0(interpolateDepth_0(t_3, w_2)));
            px_1 = px_1 + i32(1);
        }
        py_1 = py_1 + i32(1);
    }
    return;
}

fn getNormal_0( i_2 : u32) -> vec3<f32>
{
    var _S25 : u32 = i_2 * u32(3);
    return vec3<f32>(normals_0[_S25], normals_0[_S25 + u32(1)], normals_0[_S25 + u32(2)]);
}

fn getColor_0( i_3 : u32) -> vec4<f32>
{
    var _S26 : u32 = i_3 * u32(4);
    return vec4<f32>(colors_0[_S26], colors_0[_S26 + u32(1)], colors_0[_S26 + u32(2)], colors_0[_S26 + u32(3)]);
}

struct LightSample_0
{
     direction_0 : vec3<f32>,
     radiance_0 : vec3<f32>,
};

struct HostMaterial_0
{
     baseColor_0 : vec3<f32>,
     opacity_0 : f32,
     metallic_0 : f32,
     roughness_0 : f32,
     emissive_0 : vec3<f32>,
     ambientColor_0 : vec3<f32>,
     specularColor_0 : vec3<f32>,
     specularPower_0 : f32,
};

struct SurfaceSample_0
{
     position_0 : vec3<f32>,
     normal_0 : vec3<f32>,
     viewDir_0 : vec3<f32>,
     uv_0 : vec2<f32>,
     scalar_0 : f32,
     userData_0 : vec4<u32>,
     host_0 : HostMaterial_0,
};

struct MaterialSample_0
{
     baseColor_1 : vec3<f32>,
     opacity_1 : f32,
     metallic_1 : f32,
     roughness_1 : f32,
     emissive_1 : vec3<f32>,
     ambientColor_1 : vec3<f32>,
     specularColor_1 : vec3<f32>,
     specularPower_1 : f32,
};

fn MaterialSample_x24init_0( baseColor_2 : vec3<f32>,  opacity_2 : f32,  metallic_2 : f32,  roughness_2 : f32,  emissive_2 : vec3<f32>,  ambientColor_2 : vec3<f32>,  specularColor_2 : vec3<f32>,  specularPower_2 : f32) -> MaterialSample_0
{
    var _S27 : MaterialSample_0;
    _S27.baseColor_1 = baseColor_2;
    _S27.opacity_1 = opacity_2;
    _S27.metallic_1 = metallic_2;
    _S27.roughness_1 = roughness_2;
    _S27.emissive_1 = emissive_2;
    _S27.ambientColor_1 = ambientColor_2;
    _S27.specularColor_1 = specularColor_2;
    _S27.specularPower_1 = specularPower_2;
    return _S27;
}

fn HostDrivenMaterial_evaluate_0( s_0 : SurfaceSample_0) -> MaterialSample_0
{
    var _S28 : vec3<f32> = vec3<f32>(0.0f);
    var m_0 : MaterialSample_0 = MaterialSample_x24init_0(vec3<f32>(1.0f), 1.0f, 0.0f, 1.0f, _S28, _S28, _S28, 1.0f);
    m_0.baseColor_1 = s_0.host_0.baseColor_0;
    m_0.opacity_1 = s_0.host_0.opacity_0;
    m_0.metallic_1 = s_0.host_0.metallic_0;
    m_0.roughness_1 = s_0.host_0.roughness_0;
    m_0.emissive_1 = s_0.host_0.emissive_0;
    m_0.ambientColor_1 = s_0.host_0.ambientColor_0;
    m_0.specularColor_1 = s_0.host_0.specularColor_0;
    m_0.specularPower_1 = s_0.host_0.specularPower_0;
    return m_0;
}

fn LambertLighting_ambient_0( m_1 : MaterialSample_0,  s_1 : SurfaceSample_0,  ambientRadiance_1 : vec3<f32>) -> vec3<f32>
{
    return m_1.baseColor_1 * ambientRadiance_1;
}

fn LambertLighting_direct_0( m_2 : MaterialSample_0,  s_2 : SurfaceSample_0,  light_0 : LightSample_0) -> vec3<f32>
{
    return m_2.baseColor_1 * light_0.radiance_0 * vec3<f32>(max(dot(s_2.normal_0, light_0.direction_0), 0.0f));
}

fn LambertLighting_resolve_0( radiance_1 : vec3<f32>,  m_3 : MaterialSample_0) -> vec3<f32>
{
    return min(radiance_1, m_3.baseColor_1);
}

@compute
@workgroup_size(256, 1, 1)
fn main(@builtin(global_invocation_id) gid_1 : vec3<u32>)
{
    var triIdx_2 : u32 = gid_1.x;
    if(triIdx_2 >= (params_0.numTriangles_0))
    {
        return;
    }
    var i_4 : vec3<u32> = triangleIndices_0(triIdx_2);
    var _S29 : u32 = i_4.x;
    var p0_1 : vec3<f32> = getPosition_0(_S29);
    var _S30 : u32 = i_4.y;
    var p1_1 : vec3<f32> = getPosition_0(_S30);
    var _S31 : u32 = i_4.z;
    var p2_1 : vec3<f32> = getPosition_0(_S31);
    var t_4 : ScreenTriangle_0;
    var _S32 : bool = setupScreenTriangle_0(mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]), p0_1, p1_1, p2_1, params_0.width_0, params_0.height_0, &(t_4));
    if(!_S32)
    {
        return;
    }
    var faceNormal_0 : vec3<f32>;
    if((params_0.hasNormals_0) != u32(0))
    {
        faceNormal_0 = normalize((getNormal_0(_S29) + getNormal_0(_S30) + getNormal_0(_S31)) / vec3<f32>(3.0f));
    }
    else
    {
        faceNormal_0 = normalize(cross(p1_1 - p0_1, p2_1 - p0_1));
    }
    var light_1 : LightSample_0;
    light_1.direction_0 = normalize((vec3<f32>(0) - params_0.lightDir_0));
    light_1.radiance_0 = vec3<f32>(1.0f);
    var _S33 : vec4<f32> = params_0.materialColor_0;
    var _S34 : vec4<f32> = params_0.materialColor_0;
    var _S35 : vec4<f32> = params_0.materialColor_0;
    var c0_1 : vec4<f32>;
    var c1_1 : vec4<f32>;
    var c2_1 : vec4<f32>;
    if((params_0.hasColors_0) != u32(0))
    {
        var _S36 : vec4<f32> = getColor_0(_S30) * params_0.materialColor_0;
        var _S37 : vec4<f32> = getColor_0(_S31) * params_0.materialColor_0;
        c0_1 = getColor_0(_S29) * params_0.materialColor_0;
        c1_1 = _S36;
        c2_1 = _S37;
    }
    else
    {
        c0_1 = _S33;
        c1_1 = _S34;
        c2_1 = _S35;
    }
    var py_2 : i32 = i32(t_4.minY_0);
    for(;;)
    {
        if(py_2 <= i32(t_4.maxY_0))
        {
        }
        else
        {
            break;
        }
        var px_2 : i32 = i32(t_4.minX_0);
        for(;;)
        {
            if(px_2 <= i32(t_4.maxX_0))
            {
            }
            else
            {
                break;
            }
            var w_3 : vec3<f32> = barycentric_0(t_4, px_2, py_2);
            if(!covered_0(w_3))
            {
                px_2 = px_2 + i32(1);
                continue;
            }
            var pixIdx_0 : u32 = u32(py_2) * params_0.width_0 + u32(px_2);
            var dU_0 : u32 = depthToU32_0(interpolateDepth_0(t_4, w_3));
            var _S38 : u32 = atomicLoad(&(depthBufI_0[pixIdx_0]));
            if(dU_0 > (_S38 + u32(256)))
            {
                px_2 = px_2 + i32(1);
                continue;
            }
            var _S39 : f32 = w_3.x;
            var _S40 : f32 = w_3.y;
            var _S41 : f32 = w_3.z;
            var color_0 : vec4<f32> = c0_1 * vec4<f32>(_S39) + c1_1 * vec4<f32>(_S40) + c2_1 * vec4<f32>(_S41);
            var s_3 : SurfaceSample_0;
            s_3.position_0 = p0_1 * vec3<f32>(_S39) + p1_1 * vec3<f32>(_S40) + p2_1 * vec3<f32>(_S41);
            s_3.normal_0 = faceNormal_0;
            s_3.host_0.baseColor_0 = color_0.xyz;
            s_3.host_0.opacity_0 = color_0.w;
            var _S42 : MaterialSample_0 = HostDrivenMaterial_evaluate_0(s_3);
            colorBuf_0[pixIdx_0] = vec4<f32>(LambertLighting_resolve_0(LambertLighting_ambient_0(_S42, s_3, vec3<f32>(params_0.ambientRadiance_0)) + LambertLighting_direct_0(_S42, s_3, light_1), _S42) + _S42.emissive_1, _S42.opacity_1);
            px_2 = px_2 + i32(1);
        }
        py_2 = py_2 + i32(1);
    }
    return;
}


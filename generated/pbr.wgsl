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

@binding(1) @group(0) var<storage, read_write> depthBufI_0 : array<atomic<u32>>;

@binding(4) @group(0) var<storage, read> normals_0 : array<f32>;

@binding(5) @group(0) var<storage, read> colors_0 : array<f32>;

@binding(0) @group(0) var<storage, read_write> colorBuf_0 : array<vec4<f32>>;

fn pbr_triangleIndices_0( triIdx_0 : u32) -> vec3<u32>
{
    var _S1 : u32 = triIdx_0 * u32(3);
    return vec3<u32>(indices_0[_S1], indices_0[_S1 + u32(1)], indices_0[_S1 + u32(2)]);
}

fn pbr_getPos_0( i_0 : u32) -> vec3<f32>
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
fn pbr_depth_main(@builtin(global_invocation_id) gid_0 : vec3<u32>)
{
    var triIdx_1 : u32 = gid_0.x;
    if(triIdx_1 >= (params_0.numTriangles_0))
    {
        return;
    }
    var i_1 : vec3<u32> = pbr_triangleIndices_0(triIdx_1);
    var t_3 : ScreenTriangle_0;
    var _S23 : bool = setupScreenTriangle_0(mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]), pbr_getPos_0(i_1.x), pbr_getPos_0(i_1.y), pbr_getPos_0(i_1.z), params_0.width_0, params_0.height_0, &(t_3));
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

fn pbr_getNorm_0( i_2 : u32) -> vec3<f32>
{
    var _S25 : u32 = i_2 * u32(3);
    return vec3<f32>(normals_0[_S25], normals_0[_S25 + u32(1)], normals_0[_S25 + u32(2)]);
}

fn pbr_getColor_0( i_3 : u32) -> vec4<f32>
{
    var _S26 : u32 = i_3 * u32(4);
    return vec4<f32>(colors_0[_S26], colors_0[_S26 + u32(1)], colors_0[_S26 + u32(2)], colors_0[_S26 + u32(3)]);
}

fn D_GGX_0( NdotH_0 : f32,  alpha_0 : f32) -> f32
{
    var a2_0 : f32 = alpha_0 * alpha_0;
    var d_1 : f32 = NdotH_0 * NdotH_0 * (a2_0 - 1.0f) + 1.0f;
    return a2_0 / (3.14159274101257324f * d_1 * d_1 + 0.00009999999747379f);
}

fn G_SchlickGGX_0( NdotX_0 : f32,  roughness_0 : f32) -> f32
{
    var r_0 : f32 = roughness_0 + 1.0f;
    var k_0 : f32 = r_0 * r_0 / 8.0f;
    return NdotX_0 / (NdotX_0 * (1.0f - k_0) + k_0 + 0.00009999999747379f);
}

fn G_Smith_0( NdotV_0 : f32,  NdotL_0 : f32,  roughness_1 : f32) -> f32
{
    return G_SchlickGGX_0(NdotV_0, roughness_1) * G_SchlickGGX_0(NdotL_0, roughness_1);
}

fn F_Schlick_0( cosTheta_0 : f32,  F0_0 : vec3<f32>) -> vec3<f32>
{
    var t_4 : f32 = clamp(1.0f - cosTheta_0, 0.0f, 1.0f);
    return F0_0 + (vec3<f32>(1.0f, 1.0f, 1.0f) - F0_0) * vec3<f32>((t_4 * t_4 * t_4 * t_4 * t_4));
}

fn gammaCorrect_0( c_1 : f32) -> f32
{
    return pow(max(c_1, 0.0f), 0.45454543828964233f);
}

fn tonemapAndGamma_0( color_0 : vec3<f32>) -> vec3<f32>
{
    var tm_0 : vec3<f32> = color_0 / (color_0 + vec3<f32>(1.0f, 1.0f, 1.0f));
    return vec3<f32>(gammaCorrect_0(tm_0.x), gammaCorrect_0(tm_0.y), gammaCorrect_0(tm_0.z));
}

@compute
@workgroup_size(256, 1, 1)
fn pbr_main(@builtin(global_invocation_id) gid_1 : vec3<u32>)
{
    var triIdx_2 : u32 = gid_1.x;
    if(triIdx_2 >= (params_0.numTriangles_0))
    {
        return;
    }
    var i_4 : vec3<u32> = pbr_triangleIndices_0(triIdx_2);
    var i0_0 : u32 = i_4.x;
    var i1_0 : u32 = i_4.y;
    var i2_0 : u32 = i_4.z;
    var p0_1 : vec3<f32> = pbr_getPos_0(i0_0);
    var p1_1 : vec3<f32> = pbr_getPos_0(i1_0);
    var p2_1 : vec3<f32> = pbr_getPos_0(i2_0);
    var t_5 : ScreenTriangle_0;
    var _S27 : bool = setupScreenTriangle_0(mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]), p0_1, p1_1, p2_1, params_0.width_0, params_0.height_0, &(t_5));
    if(!_S27)
    {
        return;
    }
    var _S28 : f32 = params_0.metallicRoughness_0.x;
    var roughness_2 : f32 = params_0.metallicRoughness_0.y;
    var _S29 : f32 = roughness_2 * roughness_2;
    var _S30 : vec3<f32> = normalize((vec3<f32>(0) - params_0.lightDir_0));
    var py_2 : i32 = i32(t_5.minY_0);
    for(;;)
    {
        if(py_2 <= i32(t_5.maxY_0))
        {
        }
        else
        {
            break;
        }
        var px_2 : i32 = i32(t_5.minX_0);
        for(;;)
        {
            if(px_2 <= i32(t_5.maxX_0))
            {
            }
            else
            {
                break;
            }
            var w_3 : vec3<f32> = barycentric_0(t_5, px_2, py_2);
            if(!covered_0(w_3))
            {
                px_2 = px_2 + i32(1);
                continue;
            }
            var w0_0 : f32 = w_3.x;
            var w1_0 : f32 = w_3.y;
            var w2_0 : f32 = w_3.z;
            var pixIdx_0 : u32 = u32(py_2) * params_0.width_0 + u32(px_2);
            var dU_0 : u32 = depthToU32_0(interpolateDepth_0(t_5, w_3));
            var _S31 : u32 = atomicLoad(&(depthBufI_0[pixIdx_0]));
            if(dU_0 > (_S31 + u32(256)))
            {
                px_2 = px_2 + i32(1);
                continue;
            }
            var N_0 : vec3<f32>;
            if((params_0.hasNormals_0) != u32(0))
            {
                N_0 = normalize(pbr_getNorm_0(i0_0) * vec3<f32>(w0_0) + pbr_getNorm_0(i1_0) * vec3<f32>(w1_0) + pbr_getNorm_0(i2_0) * vec3<f32>(w2_0));
            }
            else
            {
                N_0 = normalize(cross(p1_1 - p0_1, p2_1 - p0_1));
            }
            var ndcC_0 : vec3<f32> = t_5.ndc0_0 * vec3<f32>(w0_0) + t_5.ndc1_0 * vec3<f32>(w1_0) + t_5.ndc2_0 * vec3<f32>(w2_0);
            var V_0 : vec3<f32> = normalize(vec3<f32>(- ndcC_0.x, - ndcC_0.y, 1.0f));
            var H_1 : vec3<f32> = normalize(V_0 + _S30);
            var _S32 : f32 = max(dot(N_0, _S30), 0.0f);
            var _S33 : f32 = max(dot(N_0, V_0), 0.00009999999747379f);
            var _S34 : f32 = max(dot(N_0, H_1), 0.0f);
            var _S35 : f32 = max(dot(H_1, V_0), 0.0f);
            var baseRGB_0 : vec3<f32> = params_0.baseColor_0.xyz;
            var baseRGB_1 : vec3<f32>;
            if((params_0.hasColors_0) != u32(0))
            {
                baseRGB_1 = baseRGB_0 * (pbr_getColor_0(i0_0) * vec4<f32>(w0_0) + pbr_getColor_0(i1_0) * vec4<f32>(w1_0) + pbr_getColor_0(i2_0) * vec4<f32>(w2_0)).xyz;
            }
            else
            {
                baseRGB_1 = baseRGB_0;
            }
            var F_0 : vec3<f32> = F_Schlick_0(_S35, mix(vec3<f32>(0.03999999910593033f, 0.03999999910593033f, 0.03999999910593033f), baseRGB_1, vec3<f32>(_S28)));
            var Lo_0 : vec3<f32> = tonemapAndGamma_0(((vec3<f32>(1.0f, 1.0f, 1.0f) - F_0) * vec3<f32>((1.0f - _S28)) * baseRGB_1 / vec3<f32>(3.14159274101257324f) + vec3<f32>((D_GGX_0(_S34, _S29) * G_Smith_0(_S33, _S32, roughness_2))) * F_0 / vec3<f32>((4.0f * _S33 * _S32 + 0.00009999999747379f))) * (vec3<f32>(params_0.lightIrradiance_0) * params_0.lightColor_0) * vec3<f32>(_S32) + vec3<f32>(params_0.ambientRadiance_0) * baseRGB_1) + params_0.emissive_0.xyz * vec3<f32>(params_0.emissive_0.w);
            var fragAlpha_0 : f32;
            if((params_0.alphaMode_0) == u32(2))
            {
                if((params_0.baseColor_0.w) < (params_0.alphaCutoff_0))
                {
                    px_2 = px_2 + i32(1);
                    continue;
                }
                fragAlpha_0 = 1.0f;
            }
            else
            {
                if((params_0.alphaMode_0) == u32(1))
                {
                    fragAlpha_0 = params_0.baseColor_0.w;
                }
                else
                {
                    fragAlpha_0 = 1.0f;
                }
            }
            colorBuf_0[pixIdx_0] = vec4<f32>(Lo_0, fragAlpha_0);
            px_2 = px_2 + i32(1);
        }
        py_2 = py_2 + i32(1);
    }
    return;
}


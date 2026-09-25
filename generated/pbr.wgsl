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

fn pbr_getPos_0( i_0 : u32) -> vec3<f32>
{
    var _S1 : u32 = i_0 * u32(3);
    return vec3<f32>(positions_0[_S1], positions_0[_S1 + u32(1)], positions_0[_S1 + u32(2)]);
}

fn edgeFunction_0( a_0 : vec2<f32>,  b_0 : vec2<f32>,  c_0 : vec2<f32>) -> f32
{
    var _S2 : f32 = a_0.x;
    var _S3 : f32 = a_0.y;
    return (c_0.x - _S2) * (b_0.y - _S3) - (c_0.y - _S3) * (b_0.x - _S2);
}

fn depthToU32_0( d_0 : f32) -> u32
{
    return (bitcast<u32>((clamp(d_0, 0.0f, 1.0f))));
}

fn pbr_getNorm_0( i_1 : u32) -> vec3<f32>
{
    var _S4 : u32 = i_1 * u32(3);
    return vec3<f32>(normals_0[_S4], normals_0[_S4 + u32(1)], normals_0[_S4 + u32(2)]);
}

fn pbr_getColor_0( i_2 : u32) -> vec4<f32>
{
    var _S5 : u32 = i_2 * u32(4);
    return vec4<f32>(colors_0[_S5], colors_0[_S5 + u32(1)], colors_0[_S5 + u32(2)], colors_0[_S5 + u32(3)]);
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
    var t_0 : f32 = clamp(1.0f - cosTheta_0, 0.0f, 1.0f);
    return F0_0 + (vec3<f32>(1.0f, 1.0f, 1.0f) - F0_0) * vec3<f32>((t_0 * t_0 * t_0 * t_0 * t_0));
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
fn pbr_main(@builtin(global_invocation_id) gid_0 : vec3<u32>)
{
    var triIdx_0 : u32 = gid_0.x;
    if(triIdx_0 >= (params_0.numTriangles_0))
    {
        return;
    }
    var _S6 : u32 = triIdx_0 * u32(3);
    var i0_0 : u32 = indices_0[_S6];
    var i1_0 : u32 = indices_0[_S6 + u32(1)];
    var i2_0 : u32 = indices_0[_S6 + u32(2)];
    var p0_0 : vec3<f32> = pbr_getPos_0(i0_0);
    var p1_0 : vec3<f32> = pbr_getPos_0(i1_0);
    var p2_0 : vec3<f32> = pbr_getPos_0(i2_0);
    var clip0_0 : vec4<f32> = (((vec4<f32>(p0_0, 1.0f)) * (mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]))));
    var clip1_0 : vec4<f32> = (((vec4<f32>(p1_0, 1.0f)) * (mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]))));
    var clip2_0 : vec4<f32> = (((vec4<f32>(p2_0, 1.0f)) * (mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]))));
    var _S7 : f32 = clip0_0.w;
    var _S8 : bool;
    if(_S7 <= 0.0f)
    {
        _S8 = true;
    }
    else
    {
        _S8 = (clip1_0.w) <= 0.0f;
    }
    if(_S8)
    {
        _S8 = true;
    }
    else
    {
        _S8 = (clip2_0.w) <= 0.0f;
    }
    if(_S8)
    {
        return;
    }
    var ndc0_0 : vec3<f32> = clip0_0.xyz / vec3<f32>(_S7);
    var ndc1_0 : vec3<f32> = clip1_0.xyz / vec3<f32>(clip1_0.w);
    var ndc2_0 : vec3<f32> = clip2_0.xyz / vec3<f32>(clip2_0.w);
    var W_0 : f32 = f32(params_0.width_0);
    var H_0 : f32 = f32(params_0.height_0);
    var _S9 : f32 = (ndc0_0.x * 0.5f + 0.5f) * W_0;
    var _S10 : f32 = (1.0f - (ndc0_0.y * 0.5f + 0.5f)) * H_0;
    var _S11 : f32 = ndc0_0.z;
    var _S12 : f32 = (ndc1_0.x * 0.5f + 0.5f) * W_0;
    var _S13 : f32 = (1.0f - (ndc1_0.y * 0.5f + 0.5f)) * H_0;
    var _S14 : f32 = ndc1_0.z;
    var _S15 : f32 = (ndc2_0.x * 0.5f + 0.5f) * W_0;
    var _S16 : f32 = (1.0f - (ndc2_0.y * 0.5f + 0.5f)) * H_0;
    var _S17 : f32 = ndc2_0.z;
    var _S18 : f32 = max(0.0f, min(min(_S9, _S12), _S15));
    var _S19 : f32 = min(W_0 - 1.0f, max(max(_S9, _S12), _S15));
    var _S20 : f32 = max(0.0f, min(min(_S10, _S13), _S16));
    var _S21 : f32 = min(H_0 - 1.0f, max(max(_S10, _S13), _S16));
    var _S22 : vec2<f32> = vec2<f32>(_S9, _S10);
    var _S23 : vec2<f32> = vec2<f32>(_S12, _S13);
    var _S24 : vec2<f32> = vec2<f32>(_S15, _S16);
    var area_0 : f32 = edgeFunction_0(_S22, _S23, _S24);
    if((abs(area_0)) < 9.99999997475242708e-07f)
    {
        return;
    }
    var _S25 : f32 = params_0.metallicRoughness_0.x;
    var roughness_2 : f32 = params_0.metallicRoughness_0.y;
    var _S26 : f32 = roughness_2 * roughness_2;
    var _S27 : vec3<f32> = normalize((vec3<f32>(0) - params_0.lightDir_0));
    var py_0 : i32 = i32(_S20);
    for(;;)
    {
        if(py_0 <= i32(_S21))
        {
        }
        else
        {
            break;
        }
        var px_0 : i32 = i32(_S18);
        for(;;)
        {
            if(px_0 <= i32(_S19))
            {
            }
            else
            {
                break;
            }
            var pp_0 : vec2<f32> = vec2<f32>(f32(px_0) + 0.5f, f32(py_0) + 0.5f);
            var w0_0 : f32 = edgeFunction_0(_S23, _S24, pp_0) / area_0;
            var w1_0 : f32 = edgeFunction_0(_S24, _S22, pp_0) / area_0;
            var w2_0 : f32 = edgeFunction_0(_S22, _S23, pp_0) / area_0;
            if(w0_0 < 0.0f)
            {
                _S8 = true;
            }
            else
            {
                _S8 = w1_0 < 0.0f;
            }
            var _S28 : bool;
            if(_S8)
            {
                _S28 = true;
            }
            else
            {
                _S28 = w2_0 < 0.0f;
            }
            if(_S28)
            {
                px_0 = px_0 + i32(1);
                continue;
            }
            var dU_0 : u32 = depthToU32_0(w0_0 * _S11 + w1_0 * _S14 + w2_0 * _S17);
            var pixIdx_0 : u32 = u32(py_0) * params_0.width_0 + u32(px_0);
            var prev_0 : u32 = atomicMin(&(depthBufI_0[pixIdx_0]), dU_0);
            if(dU_0 > prev_0)
            {
                px_0 = px_0 + i32(1);
                continue;
            }
            var N_0 : vec3<f32>;
            if((params_0.hasNormals_0) != u32(0))
            {
                N_0 = normalize(pbr_getNorm_0(i0_0) * vec3<f32>(w0_0) + pbr_getNorm_0(i1_0) * vec3<f32>(w1_0) + pbr_getNorm_0(i2_0) * vec3<f32>(w2_0));
            }
            else
            {
                N_0 = normalize(cross(p1_0 - p0_0, p2_0 - p0_0));
            }
            var ndcC_0 : vec3<f32> = ndc0_0 * vec3<f32>(w0_0) + ndc1_0 * vec3<f32>(w1_0) + ndc2_0 * vec3<f32>(w2_0);
            var V_0 : vec3<f32> = normalize(vec3<f32>(- ndcC_0.x, - ndcC_0.y, 1.0f));
            var H_1 : vec3<f32> = normalize(V_0 + _S27);
            var _S29 : f32 = max(dot(N_0, _S27), 0.0f);
            var _S30 : f32 = max(dot(N_0, V_0), 0.00009999999747379f);
            var _S31 : f32 = max(dot(N_0, H_1), 0.0f);
            var _S32 : f32 = max(dot(H_1, V_0), 0.0f);
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
            var F_0 : vec3<f32> = F_Schlick_0(_S32, mix(vec3<f32>(0.03999999910593033f, 0.03999999910593033f, 0.03999999910593033f), baseRGB_1, vec3<f32>(_S25)));
            var Lo_0 : vec3<f32> = tonemapAndGamma_0(((vec3<f32>(1.0f, 1.0f, 1.0f) - F_0) * vec3<f32>((1.0f - _S25)) * baseRGB_1 / vec3<f32>(3.14159274101257324f) + vec3<f32>((D_GGX_0(_S31, _S26) * G_Smith_0(_S30, _S29, roughness_2))) * F_0 / vec3<f32>((4.0f * _S30 * _S29 + 0.00009999999747379f))) * (vec3<f32>(params_0.lightIrradiance_0) * params_0.lightColor_0) * vec3<f32>(_S29) + vec3<f32>(params_0.ambientRadiance_0) * baseRGB_1) + params_0.emissive_0.xyz * vec3<f32>(params_0.emissive_0.w);
            var fragAlpha_0 : f32;
            if((params_0.alphaMode_0) == u32(2))
            {
                if((params_0.baseColor_0.w) < (params_0.alphaCutoff_0))
                {
                    px_0 = px_0 + i32(1);
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
            px_0 = px_0 + i32(1);
        }
        py_0 = py_0 + i32(1);
    }
    return;
}


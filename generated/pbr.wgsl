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

fn IShaderHooks_outputx24defaultImpl_1( f_1 : ptr<function, FragmentContext_0>)
{
    return;
}

fn IShaderHooks_litx24defaultImpl_0( f_2 : ptr<function, FragmentContext_0>)
{
    return;
}

fn IShaderHooks_normalx24defaultImpl_0( f_3 : ptr<function, FragmentContext_0>)
{
    return;
}

fn IShaderHooks_colorx24defaultImpl_0( f_4 : ptr<function, FragmentContext_0>)
{
    return;
}

fn IShaderHooks_clipx24defaultImpl_0( f_5 : ptr<function, FragmentContext_0>) -> bool
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

struct PBRTriangle_0
{
     v0_0 : VertexSample_0,
     v1_0 : VertexSample_0,
     v2_0 : VertexSample_0,
     t_0 : ScreenTriangle_0,
     id_0 : u32,
};

struct PBRSource_0
{
     r_0 : PBRTriangle_0,
     w_0 : vec3<f32>,
};

struct CameraSample_0
{
     shadingToClip_0 : mat4x4<f32>,
     parallel_0 : bool,
};

struct HostMaterial_0
{
     baseColor_1 : vec3<f32>,
     opacity_1 : f32,
     metallic_0 : f32,
     roughness_0 : f32,
     emissive_1 : vec3<f32>,
     ambientColor_1 : vec3<f32>,
     specularColor_1 : vec3<f32>,
     specularPower_1 : f32,
};

struct SurfaceSample_0
{
     position_1 : vec3<f32>,
     normal_1 : vec3<f32>,
     tangent_1 : vec3<f32>,
     viewDir_0 : vec3<f32>,
     uv_3 : vec2<f32>,
     scalar_0 : f32,
     fragCoord_1 : vec4<f32>,
     primitiveId_3 : u32,
     cellId_3 : u32,
     frontFacing_1 : bool,
     userData_0 : vec4<u32>,
     camera_0 : CameraSample_0,
     host_0 : HostMaterial_0,
};

fn ISurfaceSource_depthx24defaultImpl_0( this_0 : PBRSource_0,  f_6 : FragmentContext_0,  s_0 : SurfaceSample_0) -> f32
{
    return f_6.depth_0;
}

fn ISurfaceSource_geometryx24defaultImpl_0( this_1 : PBRSource_0,  f_7 : ptr<function, FragmentContext_0>)
{
    return;
}

fn GeometricNormal_normal_0( s_1 : SurfaceSample_0) -> vec3<f32>
{
    return s_1.normal_1;
}

struct MaterialSample_0
{
     baseColor_2 : vec3<f32>,
     opacity_2 : f32,
     metallic_1 : f32,
     roughness_1 : f32,
     emissive_2 : vec3<f32>,
     ambientColor_2 : vec3<f32>,
     specularColor_2 : vec3<f32>,
     specularPower_2 : f32,
};

fn MaterialSample_x24init_0( baseColor_3 : vec3<f32>,  opacity_3 : f32,  metallic_2 : f32,  roughness_2 : f32,  emissive_3 : vec3<f32>,  ambientColor_3 : vec3<f32>,  specularColor_3 : vec3<f32>,  specularPower_3 : f32) -> MaterialSample_0
{
    var _S1 : MaterialSample_0;
    _S1.baseColor_2 = baseColor_3;
    _S1.opacity_2 = opacity_3;
    _S1.metallic_1 = metallic_2;
    _S1.roughness_1 = roughness_2;
    _S1.emissive_2 = emissive_3;
    _S1.ambientColor_2 = ambientColor_3;
    _S1.specularColor_2 = specularColor_3;
    _S1.specularPower_2 = specularPower_3;
    return _S1;
}

fn HostDrivenMaterial_evaluate_0( s_2 : SurfaceSample_0) -> MaterialSample_0
{
    var _S2 : vec3<f32> = vec3<f32>(0.0f);
    var m_0 : MaterialSample_0 = MaterialSample_x24init_0(vec3<f32>(1.0f), 1.0f, 0.0f, 1.0f, _S2, _S2, _S2, 1.0f);
    m_0.baseColor_2 = s_2.host_0.baseColor_1;
    m_0.opacity_2 = s_2.host_0.opacity_1;
    m_0.metallic_1 = s_2.host_0.metallic_0;
    m_0.roughness_1 = s_2.host_0.roughness_0;
    m_0.emissive_2 = s_2.host_0.emissive_1;
    m_0.ambientColor_2 = s_2.host_0.ambientColor_1;
    m_0.specularColor_2 = s_2.host_0.specularColor_1;
    m_0.specularPower_2 = s_2.host_0.specularPower_1;
    return m_0;
}

fn PBRSource_lightCount_0( this_2 : PBRSource_0,  s_3 : SurfaceSample_0) -> u32
{
    return u32(1);
}

fn PBRSource_ambientRadiance_0( this_3 : PBRSource_0,  s_4 : SurfaceSample_0) -> vec3<f32>
{
    return vec3<f32>(params_0.ambientRadiance_0);
}

fn CookTorranceLighting_ambient_0( m_1 : MaterialSample_0,  s_5 : SurfaceSample_0,  ambientRadiance_1 : vec3<f32>) -> vec3<f32>
{
    return ambientRadiance_1 * m_1.baseColor_2;
}

struct LightSample_0
{
     direction_0 : vec3<f32>,
     radiance_0 : vec3<f32>,
};

fn LightSample_x24init_0( direction_1 : vec3<f32>,  radiance_1 : vec3<f32>) -> LightSample_0
{
    var _S3 : LightSample_0;
    _S3.direction_0 = direction_1;
    _S3.radiance_0 = radiance_1;
    return _S3;
}

fn PBRSource_light_0( this_4 : PBRSource_0,  i_0 : u32,  s_6 : SurfaceSample_0) -> LightSample_0
{
    var light_0 : LightSample_0 = LightSample_x24init_0(vec3<f32>(0.0f, 0.0f, 1.0f), vec3<f32>(1.0f));
    light_0.direction_0 = normalize((vec3<f32>(0) - params_0.lightDir_0));
    light_0.radiance_0 = vec3<f32>(params_0.lightIrradiance_0) * params_0.lightColor_0;
    return light_0;
}

fn CookTorranceLighting_D_GGX_0( NdotH_0 : f32,  alpha_0 : f32) -> f32
{
    var a2_0 : f32 = alpha_0 * alpha_0;
    var d_0 : f32 = NdotH_0 * NdotH_0 * (a2_0 - 1.0f) + 1.0f;
    return a2_0 / (3.14159274101257324f * d_0 * d_0 + 0.00009999999747379f);
}

fn CookTorranceLighting_G_SchlickGGX_0( NdotX_0 : f32,  roughness_3 : f32) -> f32
{
    var r_1 : f32 = roughness_3 + 1.0f;
    var k_0 : f32 = r_1 * r_1 / 8.0f;
    return NdotX_0 / (NdotX_0 * (1.0f - k_0) + k_0 + 0.00009999999747379f);
}

fn CookTorranceLighting_F_Schlick_0( cosTheta_0 : f32,  F0_0 : vec3<f32>) -> vec3<f32>
{
    var t_1 : f32 = clamp(1.0f - cosTheta_0, 0.0f, 1.0f);
    return F0_0 + (vec3<f32>(1.0f) - F0_0) * vec3<f32>((t_1 * t_1 * t_1 * t_1 * t_1));
}

fn CookTorranceLighting_direct_0( m_2 : MaterialSample_0,  s_7 : SurfaceSample_0,  light_1 : LightSample_0) -> vec3<f32>
{
    var H_0 : vec3<f32> = normalize(s_7.viewDir_0 + light_1.direction_0);
    var _S4 : f32 = max(dot(s_7.normal_1, light_1.direction_0), 0.0f);
    var _S5 : f32 = max(dot(s_7.normal_1, s_7.viewDir_0), 0.00009999999747379f);
    var _S6 : f32 = m_2.roughness_1;
    var F_0 : vec3<f32> = CookTorranceLighting_F_Schlick_0(max(dot(H_0, s_7.viewDir_0), 0.0f), mix(vec3<f32>(0.03999999910593033f), m_2.baseColor_2, vec3<f32>(m_2.metallic_1)));
    return ((vec3<f32>(1.0f) - F_0) * vec3<f32>((1.0f - m_2.metallic_1)) * m_2.baseColor_2 / vec3<f32>(3.14159274101257324f) + vec3<f32>((CookTorranceLighting_D_GGX_0(max(dot(s_7.normal_1, H_0), 0.0f), _S6 * _S6) * (CookTorranceLighting_G_SchlickGGX_0(_S5, m_2.roughness_1) * CookTorranceLighting_G_SchlickGGX_0(_S4, m_2.roughness_1)))) * F_0 / vec3<f32>((4.0f * _S5 * _S4 + 0.00009999999747379f))) * light_1.radiance_0 * vec3<f32>(_S4);
}

fn gammaCorrect_0( c_0 : f32) -> f32
{
    return pow(max(c_0, 0.0f), 0.45454543828964233f);
}

fn tonemapAndGamma_0( color_3 : vec3<f32>) -> vec3<f32>
{
    var tm_0 : vec3<f32> = color_3 / (color_3 + vec3<f32>(1.0f, 1.0f, 1.0f));
    return vec3<f32>(gammaCorrect_0(tm_0.x), gammaCorrect_0(tm_0.y), gammaCorrect_0(tm_0.z));
}

fn CookTorranceLighting_resolve_0( radiance_2 : vec3<f32>,  m_3 : MaterialSample_0) -> vec3<f32>
{
    return tonemapAndGamma_0(radiance_2);
}

fn PBRSource_opacity_0( this_5 : PBRSource_0,  m_4 : MaterialSample_0,  color_4 : ptr<function, vec4<f32>>) -> bool
{
    if((params_0.alphaMode_0) == u32(2))
    {
        if((m_4.opacity_2) < (params_0.alphaCutoff_0))
        {
            return false;
        }
        (*color_4)[i32(3)] = 1.0f;
    }
    else
    {
        if((params_0.alphaMode_0) != u32(1))
        {
            (*color_4)[i32(3)] = 1.0f;
        }
    }
    return true;
}

fn StandardOutput_finish_0( color_5 : vec4<f32>,  m_5 : MaterialSample_0,  s_8 : SurfaceSample_0) -> vec4<f32>
{
    return color_5;
}

fn surfaceShade_0( source_0 : PBRSource_0,  f_8 : ptr<function, FragmentContext_0>,  s_9 : SurfaceSample_0) -> bool
{
    var _S7 : SurfaceSample_0 = s_9;
    _S7.normal_1 = GeometricNormal_normal_0(s_9);
    var _S8 : MaterialSample_0 = HostDrivenMaterial_evaluate_0(_S7);
    var lights_0 : u32 = PBRSource_lightCount_0(source_0, _S7);
    if(lights_0 == u32(0))
    {
        (*f_8).color_0 = vec4<f32>(_S8.ambientColor_2 + _S8.baseColor_2, _S8.opacity_2);
    }
    else
    {
        var _S9 : vec3<f32> = CookTorranceLighting_ambient_0(_S8, _S7, PBRSource_ambientRadiance_0(source_0, _S7));
        var i_1 : u32 = u32(0);
        var radiance_3 : vec3<f32> = _S9;
        for(;;)
        {
            if(i_1 < lights_0)
            {
            }
            else
            {
                break;
            }
            var radiance_4 : vec3<f32> = radiance_3 + CookTorranceLighting_direct_0(_S8, _S7, PBRSource_light_0(source_0, i_1, _S7));
            i_1 = i_1 + u32(1);
            radiance_3 = radiance_4;
        }
        (*f_8).color_0 = vec4<f32>(CookTorranceLighting_resolve_0(radiance_3, _S8) + _S8.emissive_2, _S8.opacity_2);
    }
    var _S10 : vec4<f32> = (*f_8).color_0;
    var opaque_0 : bool = PBRSource_opacity_0(source_0, _S8, &(_S10));
    (*f_8).color_0 = _S10;
    IShaderHooks_litx24defaultImpl_0(&((*f_8)));
    (*f_8).color_0 = StandardOutput_finish_0((*f_8).color_0, _S8, _S7);
    IShaderHooks_outputx24defaultImpl_1(&((*f_8)));
    return opaque_0;
}

fn outputDepth_0( f_9 : FragmentContext_0) -> f32
{
    var _S11 : FragmentContext_0 = f_9;
    IShaderHooks_outputx24defaultImpl_0(&(_S11));
    return _S11.depth_0;
}

fn PBRSource_colors_0( this_6 : PBRSource_0,  f_10 : ptr<function, FragmentContext_0>)
{
    var baseRGB_0 : vec3<f32> = params_0.baseColor_0.xyz;
    var baseRGB_1 : vec3<f32>;
    if((params_0.hasColors_0) != u32(0))
    {
        baseRGB_1 = baseRGB_0 * (this_6.r_0.v0_0.color_2 * vec4<f32>(this_6.w_0.x) + this_6.r_0.v1_0.color_2 * vec4<f32>(this_6.w_0.y) + this_6.r_0.v2_0.color_2 * vec4<f32>(this_6.w_0.z)).xyz;
    }
    else
    {
        baseRGB_1 = baseRGB_0;
    }
    (*f_10).diffuseColor_0 = baseRGB_1;
    (*f_10).opacity_0 = params_0.baseColor_0.w;
    return;
}

fn CameraSample_x24init_0( shadingToClip_1 : mat4x4<f32>,  parallel_1 : bool) -> CameraSample_0
{
    var _S12 : CameraSample_0;
    _S12.shadingToClip_0 = shadingToClip_1;
    _S12.parallel_0 = parallel_1;
    return _S12;
}

fn HostMaterial_x24init_0( baseColor_4 : vec3<f32>,  opacity_4 : f32,  metallic_3 : f32,  roughness_4 : f32,  emissive_4 : vec3<f32>,  ambientColor_4 : vec3<f32>,  specularColor_4 : vec3<f32>,  specularPower_4 : f32) -> HostMaterial_0
{
    var _S13 : HostMaterial_0;
    _S13.baseColor_1 = baseColor_4;
    _S13.opacity_1 = opacity_4;
    _S13.metallic_0 = metallic_3;
    _S13.roughness_0 = roughness_4;
    _S13.emissive_1 = emissive_4;
    _S13.ambientColor_1 = ambientColor_4;
    _S13.specularColor_1 = specularColor_4;
    _S13.specularPower_1 = specularPower_4;
    return _S13;
}

fn SurfaceSample_x24init_0( position_2 : vec3<f32>,  normal_2 : vec3<f32>,  tangent_2 : vec3<f32>,  viewDir_1 : vec3<f32>,  uv_4 : vec2<f32>,  scalar_1 : f32,  fragCoord_2 : vec4<f32>,  primitiveId_4 : u32,  cellId_4 : u32,  frontFacing_2 : bool,  userData_1 : vec4<u32>,  camera_1 : CameraSample_0,  host_1 : HostMaterial_0) -> SurfaceSample_0
{
    var _S14 : SurfaceSample_0;
    _S14.position_1 = position_2;
    _S14.normal_1 = normal_2;
    _S14.tangent_1 = tangent_2;
    _S14.viewDir_0 = viewDir_1;
    _S14.uv_3 = uv_4;
    _S14.scalar_0 = scalar_1;
    _S14.fragCoord_1 = fragCoord_2;
    _S14.primitiveId_3 = primitiveId_4;
    _S14.cellId_3 = cellId_4;
    _S14.frontFacing_1 = frontFacing_2;
    _S14.userData_0 = userData_1;
    _S14.camera_0 = camera_1;
    _S14.host_0 = host_1;
    return _S14;
}

fn surfaceSample_0( f_11 : FragmentContext_0) -> SurfaceSample_0
{
    var _S15 : vec3<f32> = vec3<f32>(0.0f);
    const _S16 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 1.0f);
    var s_10 : SurfaceSample_0 = SurfaceSample_x24init_0(_S15, _S16, _S15, _S16, vec2<f32>(0.0f), 0.0f, vec4<f32>(0.0f), u32(0), u32(0), true, vec4<u32>(u32(0)), CameraSample_x24init_0(mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f), false), HostMaterial_x24init_0(vec3<f32>(1.0f), 1.0f, 0.0f, 1.0f, _S15, _S15, _S15, 1.0f));
    var _S17 : vec3<f32> = f_11.positionVC_0.xyz;
    s_10.position_1 = _S17;
    s_10.normal_1 = f_11.normalVC_0;
    s_10.tangent_1 = f_11.tangentVC_0;
    s_10.viewDir_0 = normalize((vec3<f32>(0) - _S17));
    s_10.uv_3 = f_11.uv_0;
    s_10.fragCoord_1 = f_11.fragCoord_0;
    s_10.primitiveId_3 = f_11.primitiveId_0;
    s_10.cellId_3 = f_11.cellId_0;
    s_10.frontFacing_1 = f_11.frontFacing_0;
    s_10.host_0.baseColor_1 = f_11.diffuseColor_0;
    s_10.host_0.ambientColor_1 = f_11.ambientColor_0;
    s_10.host_0.specularColor_1 = f_11.specularColor_0;
    s_10.host_0.specularPower_1 = f_11.specularPower_0;
    s_10.host_0.opacity_1 = f_11.opacity_0;
    return s_10;
}

fn PBRSource_sample_0( this_7 : PBRSource_0,  f_12 : FragmentContext_0) -> SurfaceSample_0
{
    var s_11 : SurfaceSample_0 = surfaceSample_0(f_12);
    var ndcC_0 : vec3<f32> = this_7.r_0.t_0.ndc0_0 * vec3<f32>(this_7.w_0.x) + this_7.r_0.t_0.ndc1_0 * vec3<f32>(this_7.w_0.y) + this_7.r_0.t_0.ndc2_0 * vec3<f32>(this_7.w_0.z);
    s_11.viewDir_0 = normalize(vec3<f32>(- ndcC_0.x, - ndcC_0.y, 1.0f));
    s_11.camera_0.shadingToClip_0 = mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]);
    s_11.host_0.metallic_0 = params_0.metallicRoughness_0.x;
    s_11.host_0.roughness_0 = params_0.metallicRoughness_0.y;
    s_11.host_0.emissive_1 = params_0.emissive_0.xyz * vec3<f32>(params_0.emissive_0.w);
    return s_11;
}

fn StandardOutput_keep_0( s_12 : SurfaceSample_0) -> bool
{
    return true;
}

fn StandardOutput_depth_0( depth_1 : f32,  s_13 : SurfaceSample_0) -> f32
{
    return depth_1;
}

fn surfaceCoverage_0( source_1 : PBRSource_0,  f_13 : ptr<function, FragmentContext_0>,  s_14 : ptr<function, SurfaceSample_0>) -> bool
{
    var _S18 : bool = IShaderHooks_clipx24defaultImpl_0(&((*f_13)));
    PBRSource_colors_0(source_1, &((*f_13)));
    IShaderHooks_colorx24defaultImpl_0(&((*f_13)));
    ISurfaceSource_geometryx24defaultImpl_0(source_1, &((*f_13)));
    IShaderHooks_normalx24defaultImpl_0(&((*f_13)));
    var _S19 : SurfaceSample_0 = PBRSource_sample_0(source_1, (*f_13));
    (*s_14) = _S19;
    var covered_0 : bool;
    if(!StandardOutput_keep_0(_S19))
    {
        covered_0 = false;
    }
    else
    {
        covered_0 = _S18;
    }
    (*f_13).depth_0 = StandardOutput_depth_0(ISurfaceSource_depthx24defaultImpl_0(source_1, (*f_13), (*s_14)), (*s_14));
    return covered_0;
}

fn projectVertex_0( c_1 : ptr<function, VertexContext_0>,  positionClip_1 : vec4<f32>) -> vec4<f32>
{
    (*c_1).positionClip_0 = positionClip_1;
    IShaderHooks_positionx24defaultImpl_0(&((*c_1)));
    return (*c_1).positionClip_0;
}

fn IdentityVertex_modify_0( v_2 : VertexSample_0) -> VertexSample_0
{
    return v_2;
}

fn VertexContext_x24init_0( positionMC_1 : vec4<f32>,  normalMC_1 : vec3<f32>,  tangentMC_1 : vec3<f32>,  uv_5 : vec2<f32>,  color_6 : vec4<f32>,  positionClip_2 : vec4<f32>,  pointId_2 : u32,  cellId_5 : u32,  primitiveId_5 : u32,  modelToClip_2 : mat4x4<f32>,  normalModelToView_1 : mat3x3<f32>) -> VertexContext_0
{
    var _S20 : VertexContext_0;
    _S20.positionMC_0 = positionMC_1;
    _S20.normalMC_0 = normalMC_1;
    _S20.tangentMC_0 = tangentMC_1;
    _S20.uv_1 = uv_5;
    _S20.color_1 = color_6;
    _S20.positionClip_0 = positionClip_2;
    _S20.pointId_0 = pointId_2;
    _S20.cellId_1 = cellId_5;
    _S20.primitiveId_1 = primitiveId_5;
    _S20.modelToClip_0 = modelToClip_2;
    _S20.normalModelToView_0 = normalModelToView_1;
    return _S20;
}

fn vertexContext_0( v_3 : VertexSample_0) -> VertexContext_0
{
    const _S21 : vec4<f32> = vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f);
    var c_2 : VertexContext_0 = VertexContext_x24init_0(_S21, vec3<f32>(0.0f, 0.0f, 1.0f), vec3<f32>(0.0f), vec2<f32>(0.0f), vec4<f32>(1.0f), _S21, u32(0), u32(0), u32(0), mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f), mat3x3<f32>(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    c_2.positionMC_0 = vec4<f32>(v_3.position_0, 1.0f);
    c_2.normalMC_0 = v_3.normal_0;
    c_2.tangentMC_0 = v_3.tangent_0;
    c_2.uv_1 = v_3.uv_2;
    c_2.color_1 = v_3.color_2;
    c_2.pointId_0 = v_3.pointId_1;
    c_2.cellId_1 = v_3.cellId_2;
    c_2.primitiveId_1 = v_3.primitiveId_2;
    c_2.modelToClip_0 = v_3.modelToClip_1;
    return c_2;
}

fn vertexSample_0( c_3 : VertexContext_0,  v_4 : VertexSample_0) -> VertexSample_0
{
    var _S22 : VertexSample_0 = v_4;
    _S22.position_0 = c_3.positionMC_0.xyz;
    _S22.normal_0 = c_3.normalMC_0;
    _S22.tangent_0 = c_3.tangentMC_0;
    _S22.uv_2 = c_3.uv_1;
    _S22.color_2 = c_3.color_1;
    return _S22;
}

fn shadeVertex_0( v_5 : VertexSample_0,  c_4 : ptr<function, VertexContext_0>) -> VertexSample_0
{
    var _S23 : VertexSample_0 = IdentityVertex_modify_0(v_5);
    (*c_4) = vertexContext_0(_S23);
    IShaderHooks_vertexx24defaultImpl_0(&((*c_4)));
    return vertexSample_0((*c_4), _S23);
}

fn VertexSample_x24init_0( position_3 : vec3<f32>,  normal_3 : vec3<f32>,  tangent_3 : vec3<f32>,  uv_6 : vec2<f32>,  color_7 : vec4<f32>,  pointId_3 : u32,  cellId_6 : u32,  primitiveId_6 : u32,  modelToClip_3 : mat4x4<f32>) -> VertexSample_0
{
    var _S24 : VertexSample_0;
    _S24.position_0 = position_3;
    _S24.normal_0 = normal_3;
    _S24.tangent_0 = tangent_3;
    _S24.uv_2 = uv_6;
    _S24.color_2 = color_7;
    _S24.pointId_1 = pointId_3;
    _S24.cellId_2 = cellId_6;
    _S24.primitiveId_2 = primitiveId_6;
    _S24.modelToClip_1 = modelToClip_3;
    return _S24;
}

fn ScreenTriangle_x24init_0( s0_1 : vec3<f32>,  s1_1 : vec3<f32>,  s2_1 : vec3<f32>,  ndc0_1 : vec3<f32>,  ndc1_1 : vec3<f32>,  ndc2_1 : vec3<f32>,  area_1 : f32,  minX_1 : f32,  maxX_1 : f32,  minY_1 : f32,  maxY_1 : f32) -> ScreenTriangle_0
{
    var _S25 : ScreenTriangle_0;
    _S25.s0_0 = s0_1;
    _S25.s1_0 = s1_1;
    _S25.s2_0 = s2_1;
    _S25.ndc0_0 = ndc0_1;
    _S25.ndc1_0 = ndc1_1;
    _S25.ndc2_0 = ndc2_1;
    _S25.area_0 = area_1;
    _S25.minX_0 = minX_1;
    _S25.maxX_0 = maxX_1;
    _S25.minY_0 = minY_1;
    _S25.maxY_0 = maxY_1;
    return _S25;
}

fn PBRTriangle_x24init_0( v0_1 : VertexSample_0,  v1_1 : VertexSample_0,  v2_1 : VertexSample_0,  t_2 : ScreenTriangle_0,  id_1 : u32) -> PBRTriangle_0
{
    var _S26 : PBRTriangle_0;
    _S26.v0_0 = v0_1;
    _S26.v1_0 = v1_1;
    _S26.v2_0 = v2_1;
    _S26.t_0 = t_2;
    _S26.id_0 = id_1;
    return _S26;
}

fn pbr_triangleIndices_0( triIdx_0 : u32) -> vec3<u32>
{
    var _S27 : u32 = triIdx_0 * u32(3);
    return vec3<u32>(indices_0[_S27], indices_0[_S27 + u32(1)], indices_0[_S27 + u32(2)]);
}

fn pbr_getPos_0( i_2 : u32) -> vec3<f32>
{
    var _S28 : u32 = i_2 * u32(3);
    return vec3<f32>(positions_0[_S28], positions_0[_S28 + u32(1)], positions_0[_S28 + u32(2)]);
}

fn pbr_getNorm_0( i_3 : u32) -> vec3<f32>
{
    var _S29 : u32 = i_3 * u32(3);
    return vec3<f32>(normals_0[_S29], normals_0[_S29 + u32(1)], normals_0[_S29 + u32(2)]);
}

fn pbr_getColor_0( i_4 : u32) -> vec4<f32>
{
    var _S30 : u32 = i_4 * u32(4);
    return vec4<f32>(colors_0[_S30], colors_0[_S30 + u32(1)], colors_0[_S30 + u32(2)], colors_0[_S30 + u32(3)]);
}

fn pbr_fetchVertex_0( triIdx_1 : u32,  idx_0 : u32,  c_5 : ptr<function, VertexContext_0>) -> VertexSample_0
{
    var _S31 : vec3<f32> = vec3<f32>(0.0f);
    var v_6 : VertexSample_0 = VertexSample_x24init_0(_S31, vec3<f32>(0.0f, 0.0f, 1.0f), _S31, vec2<f32>(0.0f), vec4<f32>(1.0f), u32(0), u32(0), u32(0), mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    v_6.position_0 = pbr_getPos_0(idx_0);
    if((params_0.hasNormals_0) != u32(0))
    {
        v_6.normal_0 = pbr_getNorm_0(idx_0);
    }
    if((params_0.hasColors_0) != u32(0))
    {
        v_6.color_2 = pbr_getColor_0(idx_0);
    }
    v_6.pointId_1 = idx_0;
    v_6.cellId_2 = triIdx_1;
    v_6.primitiveId_2 = triIdx_1;
    v_6.modelToClip_1 = mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]);
    var _S32 : VertexSample_0 = shadeVertex_0(v_6, &((*c_5)));
    return _S32;
}

fn edgeFunction_0( a_0 : vec2<f32>,  b_0 : vec2<f32>,  c_6 : vec2<f32>) -> f32
{
    var _S33 : f32 = a_0.x;
    var _S34 : f32 = a_0.y;
    return (c_6.x - _S33) * (b_0.y - _S34) - (c_6.y - _S34) * (b_0.x - _S33);
}

fn setupScreenTriangleClip_0( c0_0 : vec4<f32>,  c1_0 : vec4<f32>,  c2_0 : vec4<f32>,  width_1 : u32,  height_1 : u32,  t_3 : ptr<function, ScreenTriangle_0>) -> bool
{
    var _S35 : vec3<f32> = vec3<f32>(0.0f);
    (*t_3) = ScreenTriangle_x24init_0(_S35, _S35, _S35, _S35, _S35, _S35, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    var _S36 : f32 = c0_0.w;
    var _S37 : bool;
    if(_S36 <= 0.0f)
    {
        _S37 = true;
    }
    else
    {
        _S37 = (c1_0.w) <= 0.0f;
    }
    if(_S37)
    {
        _S37 = true;
    }
    else
    {
        _S37 = (c2_0.w) <= 0.0f;
    }
    if(_S37)
    {
        return false;
    }
    var _S38 : vec3<f32> = c0_0.xyz / vec3<f32>(_S36);
    (*t_3).ndc0_0 = _S38;
    var _S39 : vec3<f32> = c1_0.xyz / vec3<f32>(c1_0.w);
    (*t_3).ndc1_0 = _S39;
    var _S40 : vec3<f32> = c2_0.xyz / vec3<f32>(c2_0.w);
    (*t_3).ndc2_0 = _S40;
    var W_0 : f32 = f32(width_1);
    var H_1 : f32 = f32(height_1);
    var _S41 : f32 = (_S38.x * 0.5f + 0.5f) * W_0;
    var _S42 : f32 = (1.0f - (_S38.y * 0.5f + 0.5f)) * H_1;
    (*t_3).s0_0 = vec3<f32>(_S41, _S42, _S38.z);
    var _S43 : f32 = (_S39.x * 0.5f + 0.5f) * W_0;
    var _S44 : f32 = (1.0f - (_S39.y * 0.5f + 0.5f)) * H_1;
    (*t_3).s1_0 = vec3<f32>(_S43, _S44, _S39.z);
    var _S45 : f32 = (_S40.x * 0.5f + 0.5f) * W_0;
    var _S46 : f32 = (1.0f - (_S40.y * 0.5f + 0.5f)) * H_1;
    (*t_3).s2_0 = vec3<f32>(_S45, _S46, _S40.z);
    (*t_3).minX_0 = max(0.0f, min(min(_S41, _S43), _S45));
    (*t_3).maxX_0 = min(W_0 - 1.0f, max(max(_S41, _S43), _S45));
    (*t_3).minY_0 = max(0.0f, min(min(_S42, _S44), _S46));
    (*t_3).maxY_0 = min(H_1 - 1.0f, max(max(_S42, _S44), _S46));
    var _S47 : f32 = edgeFunction_0(vec2<f32>(_S41, _S42), vec2<f32>(_S43, _S44), vec2<f32>(_S45, _S46));
    (*t_3).area_0 = _S47;
    return (abs(_S47)) >= 9.99999997475242708e-07f;
}

fn pbr_setupTriangle_0( triIdx_2 : u32,  r_2 : ptr<function, PBRTriangle_0>) -> bool
{
    var _S48 : vec3<f32> = vec3<f32>(0.0f);
    var _S49 : VertexSample_0 = VertexSample_x24init_0(_S48, vec3<f32>(0.0f, 0.0f, 1.0f), _S48, vec2<f32>(0.0f), vec4<f32>(1.0f), u32(0), u32(0), u32(0), mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    (*r_2) = PBRTriangle_x24init_0(_S49, _S49, _S49, ScreenTriangle_x24init_0(_S48, _S48, _S48, _S48, _S48, _S48, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), u32(0));
    (*r_2).id_0 = triIdx_2;
    var i_5 : vec3<u32> = pbr_triangleIndices_0(triIdx_2);
    var c0_1 : VertexContext_0;
    var _S50 : VertexSample_0 = pbr_fetchVertex_0(triIdx_2, i_5.x, &(c0_1));
    (*r_2).v0_0 = _S50;
    var c1_1 : VertexContext_0;
    var _S51 : VertexSample_0 = pbr_fetchVertex_0(triIdx_2, i_5.y, &(c1_1));
    (*r_2).v1_0 = _S51;
    var c2_1 : VertexContext_0;
    var _S52 : VertexSample_0 = pbr_fetchVertex_0(triIdx_2, i_5.z, &(c2_1));
    (*r_2).v2_0 = _S52;
    var _S53 : vec4<f32> = projectVertex_0(&(c0_1), (((c0_1.positionMC_0) * (mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)])))));
    var _S54 : vec4<f32> = projectVertex_0(&(c1_1), (((c1_1.positionMC_0) * (mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)])))));
    var _S55 : vec4<f32> = projectVertex_0(&(c2_1), (((c2_1.positionMC_0) * (mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)])))));
    var _S56 : ScreenTriangle_0 = (*r_2).t_0;
    var _S57 : bool = setupScreenTriangleClip_0(_S53, _S54, _S55, params_0.width_0, params_0.height_0, &(_S56));
    (*r_2).t_0 = _S56;
    return _S57;
}

fn barycentric_0( t_4 : ScreenTriangle_0,  px_0 : i32,  py_0 : i32) -> vec3<f32>
{
    var p_0 : vec2<f32> = vec2<f32>(f32(px_0) + 0.5f, f32(py_0) + 0.5f);
    var _S58 : vec2<f32> = t_4.s1_0.xy;
    var _S59 : vec2<f32> = t_4.s2_0.xy;
    var _S60 : vec2<f32> = t_4.s0_0.xy;
    return vec3<f32>(edgeFunction_0(_S58, _S59, p_0) / t_4.area_0, edgeFunction_0(_S59, _S60, p_0) / t_4.area_0, edgeFunction_0(_S60, _S58, p_0) / t_4.area_0);
}

fn covered_1( w_1 : vec3<f32>) -> bool
{
    var _S61 : bool;
    if((w_1.x) >= 0.0f)
    {
        _S61 = (w_1.y) >= 0.0f;
    }
    else
    {
        _S61 = false;
    }
    if(_S61)
    {
        _S61 = (w_1.z) >= 0.0f;
    }
    else
    {
        _S61 = false;
    }
    return _S61;
}

fn PBRSource_x24init_0( r_3 : PBRTriangle_0,  w_2 : vec3<f32>) -> PBRSource_0
{
    var _S62 : PBRSource_0;
    _S62.r_0 = r_3;
    _S62.w_0 = w_2;
    return _S62;
}

fn interpolateDepth_0( t_5 : ScreenTriangle_0,  w_3 : vec3<f32>) -> f32
{
    return w_3.x * t_5.s0_0.z + w_3.y * t_5.s1_0.z + w_3.z * t_5.s2_0.z;
}

fn FragmentContext_x24init_0( positionVC_1 : vec4<f32>,  normalVC_1 : vec3<f32>,  tangentVC_1 : vec3<f32>,  uv_7 : vec2<f32>,  ambientColor_5 : vec3<f32>,  diffuseColor_1 : vec3<f32>,  specularColor_5 : vec3<f32>,  specularPower_5 : f32,  opacity_5 : f32,  color_8 : vec4<f32>,  depth_2 : f32,  ids_1 : vec4<u32>,  fragCoord_3 : vec4<f32>,  primitiveId_7 : u32,  cellId_7 : u32,  frontFacing_3 : bool) -> FragmentContext_0
{
    var _S63 : FragmentContext_0;
    _S63.positionVC_0 = positionVC_1;
    _S63.normalVC_0 = normalVC_1;
    _S63.tangentVC_0 = tangentVC_1;
    _S63.uv_0 = uv_7;
    _S63.ambientColor_0 = ambientColor_5;
    _S63.diffuseColor_0 = diffuseColor_1;
    _S63.specularColor_0 = specularColor_5;
    _S63.specularPower_0 = specularPower_5;
    _S63.opacity_0 = opacity_5;
    _S63.color_0 = color_8;
    _S63.depth_0 = depth_2;
    _S63.ids_0 = ids_1;
    _S63.fragCoord_0 = fragCoord_3;
    _S63.primitiveId_0 = primitiveId_7;
    _S63.cellId_0 = cellId_7;
    _S63.frontFacing_0 = frontFacing_3;
    return _S63;
}

fn pbr_fragment_0( r_4 : PBRTriangle_0,  w_4 : vec3<f32>,  px_1 : i32,  py_1 : i32,  depth_3 : f32) -> FragmentContext_0
{
    const _S64 : vec4<f32> = vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f);
    var _S65 : vec3<f32> = vec3<f32>(0.0f);
    var f_14 : FragmentContext_0 = FragmentContext_x24init_0(_S64, vec3<f32>(0.0f, 0.0f, 1.0f), _S65, vec2<f32>(0.0f), _S65, vec3<f32>(1.0f), _S65, 1.0f, 1.0f, _S64, 0.0f, vec4<u32>(u32(0)), vec4<f32>(0.0f), u32(0), u32(0), true);
    var _S66 : f32 = w_4.x;
    var _S67 : vec3<f32> = vec3<f32>(_S66);
    var _S68 : f32 = w_4.y;
    var _S69 : vec3<f32> = vec3<f32>(_S68);
    var _S70 : f32 = w_4.z;
    var _S71 : vec3<f32> = vec3<f32>(_S70);
    f_14.positionVC_0 = vec4<f32>(r_4.v0_0.position_0 * _S67 + r_4.v1_0.position_0 * _S69 + r_4.v2_0.position_0 * _S71, 1.0f);
    if((params_0.hasNormals_0) != u32(0))
    {
        f_14.normalVC_0 = normalize(r_4.v0_0.normal_0 * _S67 + r_4.v1_0.normal_0 * _S69 + r_4.v2_0.normal_0 * _S71);
    }
    else
    {
        f_14.normalVC_0 = normalize(cross(r_4.v1_0.position_0 - r_4.v0_0.position_0, r_4.v2_0.position_0 - r_4.v0_0.position_0));
    }
    f_14.uv_0 = r_4.v0_0.uv_2 * vec2<f32>(_S66) + r_4.v1_0.uv_2 * vec2<f32>(_S68) + r_4.v2_0.uv_2 * vec2<f32>(_S70);
    f_14.fragCoord_0 = vec4<f32>(f32(px_1) + 0.5f, f32(py_1) + 0.5f, depth_3, 1.0f);
    f_14.depth_0 = depth_3;
    f_14.primitiveId_0 = r_4.id_0;
    f_14.cellId_0 = r_4.id_0;
    f_14.frontFacing_0 = (r_4.t_0.area_0) < 0.0f;
    return f_14;
}

fn pbr_fragmentDepth_0( source_2 : PBRSource_0,  f_15 : ptr<function, FragmentContext_0>,  s_15 : ptr<function, SurfaceSample_0>,  depth_4 : ptr<function, f32>) -> bool
{
    (*depth_4) = (*f_15).depth_0;
    var _S72 : bool = surfaceCoverage_0(source_2, &((*f_15)), &((*s_15)));
    if(!_S72)
    {
        return false;
    }
    (*depth_4) = outputDepth_0((*f_15));
    return true;
}

fn depthToU32_0( d_1 : f32) -> u32
{
    return (bitcast<u32>((clamp(d_1, 0.0f, 1.0f))));
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
    var r_5 : PBRTriangle_0;
    var _S73 : bool = pbr_setupTriangle_0(triIdx_3, &(r_5));
    if(!_S73)
    {
        return;
    }
    var py_2 : i32 = i32(r_5.t_0.minY_0);
    for(;;)
    {
        if(py_2 <= i32(r_5.t_0.maxY_0))
        {
        }
        else
        {
            break;
        }
        var px_2 : i32 = i32(r_5.t_0.minX_0);
        for(;;)
        {
            if(px_2 <= i32(r_5.t_0.maxX_0))
            {
            }
            else
            {
                break;
            }
            var w_5 : vec3<f32> = barycentric_0(r_5.t_0, px_2, py_2);
            if(!covered_1(w_5))
            {
                px_2 = px_2 + i32(1);
                continue;
            }
            var source_3 : PBRSource_0 = PBRSource_x24init_0(r_5, w_5);
            var f_16 : FragmentContext_0 = pbr_fragment_0(r_5, w_5, px_2, py_2, interpolateDepth_0(r_5.t_0, w_5));
            var s_16 : SurfaceSample_0;
            var depth_5 : f32;
            var _S74 : bool = pbr_fragmentDepth_0(source_3, &(f_16), &(s_16), &(depth_5));
            if(!_S74)
            {
                px_2 = px_2 + i32(1);
                continue;
            }
            var _S75 : u32 = atomicMin(&(depthBufI_0[u32(py_2) * params_0.width_0 + u32(px_2)]), depthToU32_0(depth_5));
            px_2 = px_2 + i32(1);
        }
        py_2 = py_2 + i32(1);
    }
    return;
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
    var r_6 : PBRTriangle_0;
    var _S76 : bool = pbr_setupTriangle_0(triIdx_4, &(r_6));
    if(!_S76)
    {
        return;
    }
    var py_3 : i32 = i32(r_6.t_0.minY_0);
    for(;;)
    {
        if(py_3 <= i32(r_6.t_0.maxY_0))
        {
        }
        else
        {
            break;
        }
        var px_3 : i32 = i32(r_6.t_0.minX_0);
        for(;;)
        {
            if(px_3 <= i32(r_6.t_0.maxX_0))
            {
            }
            else
            {
                break;
            }
            var w_6 : vec3<f32> = barycentric_0(r_6.t_0, px_3, py_3);
            if(!covered_1(w_6))
            {
                px_3 = px_3 + i32(1);
                continue;
            }
            var source_4 : PBRSource_0 = PBRSource_x24init_0(r_6, w_6);
            var f_17 : FragmentContext_0 = pbr_fragment_0(r_6, w_6, px_3, py_3, interpolateDepth_0(r_6.t_0, w_6));
            var s_17 : SurfaceSample_0;
            var depth_6 : f32;
            var _S77 : bool = pbr_fragmentDepth_0(source_4, &(f_17), &(s_17), &(depth_6));
            if(!_S77)
            {
                px_3 = px_3 + i32(1);
                continue;
            }
            var pixIdx_0 : u32 = u32(py_3) * params_0.width_0 + u32(px_3);
            var _S78 : u32 = depthToU32_0(depth_6);
            var _S79 : u32 = atomicLoad(&(depthBufI_0[pixIdx_0]));
            if(_S78 > (_S79 + u32(256)))
            {
                px_3 = px_3 + i32(1);
                continue;
            }
            var _S80 : bool = surfaceShade_0(source_4, &(f_17), s_17);
            if(!_S80)
            {
                px_3 = px_3 + i32(1);
                continue;
            }
            colorBuf_0[pixIdx_0] = f_17.color_0;
            px_3 = px_3 + i32(1);
        }
        py_3 = py_3 + i32(1);
    }
    return;
}


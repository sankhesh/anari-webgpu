// Generated from shaders/pbr.slang with its default specializations
// by SlangKit (Slang 2026.13.1). Do not edit; regenerate with the
// anari_webgpu_update_wgsl target.

struct _MatrixStorage_float4x4_ColMajorstd140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Image2DParams_std140_0
{
    @align(16) inTransform_0 : _MatrixStorage_float4x4_ColMajorstd140_0,
    @align(16) inOffset_0 : vec4<f32>,
    @align(16) outTransform_0 : _MatrixStorage_float4x4_ColMajorstd140_0,
    @align(16) outOffset_0 : vec4<f32>,
    @align(16) enabled_0 : u32,
    @align(4) width_0 : u32,
    @align(8) height_0 : u32,
    @align(4) linearFilter_0 : u32,
    @align(16) wrap1_0 : u32,
    @align(4) wrap2_0 : u32,
    @align(8) _pad0_0 : u32,
    @align(4) _pad1_0 : u32,
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
    @align(16) width_1 : u32,
    @align(4) height_1 : u32,
    @align(8) numTriangles_0 : u32,
    @align(4) hasNormals_0 : u32,
    @align(16) hasColors_0 : u32,
    @align(4) alphaMode_0 : u32,
    @align(8) alphaCutoff_0 : f32,
    @align(4) _pad_0 : u32,
    @align(16) image_0 : Image2DParams_std140_0,
};

@binding(2) @group(0) var<uniform> params_0 : PBRUniforms_std140_0;
@binding(6) @group(0) var<storage, read> indices_0 : array<u32>;

@binding(3) @group(0) var<storage, read> positions_0 : array<f32>;

@binding(4) @group(0) var<storage, read> normals_0 : array<f32>;

@binding(5) @group(0) var<storage, read> colors_0 : array<f32>;

@binding(1) @group(0) var<storage, read_write> depthBufI_0 : array<atomic<u32>>;

@binding(0) @group(0) var<storage, read_write> colorBuf_0 : array<vec4<f32>>;

@binding(7) @group(0) var<storage, read> texels_0 : array<vec4<f32>>;

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

fn LinearCookTorranceLighting_ambient_0( m_2 : MaterialSample_0,  s_6 : SurfaceSample_0,  ambientRadiance_2 : vec3<f32>) -> vec3<f32>
{
    return CookTorranceLighting_ambient_0(m_2, s_6, ambientRadiance_2);
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

fn PBRSource_light_0( this_4 : PBRSource_0,  i_0 : u32,  s_7 : SurfaceSample_0) -> LightSample_0
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

fn CookTorranceLighting_direct_0( m_3 : MaterialSample_0,  s_8 : SurfaceSample_0,  light_1 : LightSample_0) -> vec3<f32>
{
    var H_0 : vec3<f32> = normalize(s_8.viewDir_0 + light_1.direction_0);
    var _S4 : f32 = max(dot(s_8.normal_1, light_1.direction_0), 0.0f);
    var _S5 : f32 = max(dot(s_8.normal_1, s_8.viewDir_0), 0.00009999999747379f);
    var _S6 : f32 = m_3.roughness_1;
    var F_0 : vec3<f32> = CookTorranceLighting_F_Schlick_0(max(dot(H_0, s_8.viewDir_0), 0.0f), mix(vec3<f32>(0.03999999910593033f), m_3.baseColor_2, vec3<f32>(m_3.metallic_1)));
    return ((vec3<f32>(1.0f) - F_0) * vec3<f32>((1.0f - m_3.metallic_1)) * m_3.baseColor_2 / vec3<f32>(3.14159274101257324f) + vec3<f32>((CookTorranceLighting_D_GGX_0(max(dot(s_8.normal_1, H_0), 0.0f), _S6 * _S6) * (CookTorranceLighting_G_SchlickGGX_0(_S5, m_3.roughness_1) * CookTorranceLighting_G_SchlickGGX_0(_S4, m_3.roughness_1)))) * F_0 / vec3<f32>((4.0f * _S5 * _S4 + 0.00009999999747379f))) * light_1.radiance_0 * vec3<f32>(_S4);
}

fn LinearCookTorranceLighting_direct_0( m_4 : MaterialSample_0,  s_9 : SurfaceSample_0,  light_2 : LightSample_0) -> vec3<f32>
{
    return CookTorranceLighting_direct_0(m_4, s_9, light_2);
}

fn LinearCookTorranceLighting_resolve_0( radiance_2 : vec3<f32>,  m_5 : MaterialSample_0) -> vec3<f32>
{
    return radiance_2;
}

fn PBRSource_opacity_0( this_5 : PBRSource_0,  m_6 : MaterialSample_0,  color_3 : ptr<function, vec4<f32>>) -> bool
{
    if((params_0.alphaMode_0) == u32(2))
    {
        if((m_6.opacity_2) < (params_0.alphaCutoff_0))
        {
            return false;
        }
        (*color_3)[i32(3)] = 1.0f;
    }
    else
    {
        if((params_0.alphaMode_0) != u32(1))
        {
            (*color_3)[i32(3)] = 1.0f;
        }
    }
    return true;
}

fn StandardOutput_finish_0( color_4 : vec4<f32>,  m_7 : MaterialSample_0,  s_10 : SurfaceSample_0) -> vec4<f32>
{
    return color_4;
}

fn surfaceShade_0( source_0 : PBRSource_0,  f_8 : ptr<function, FragmentContext_0>,  s_11 : SurfaceSample_0) -> bool
{
    var _S7 : SurfaceSample_0 = s_11;
    _S7.normal_1 = GeometricNormal_normal_0(s_11);
    var _S8 : MaterialSample_0 = HostDrivenMaterial_evaluate_0(_S7);
    var lights_0 : u32 = PBRSource_lightCount_0(source_0, _S7);
    if(lights_0 == u32(0))
    {
        (*f_8).color_0 = vec4<f32>(_S8.ambientColor_2 + _S8.baseColor_2, _S8.opacity_2);
    }
    else
    {
        var _S9 : vec3<f32> = LinearCookTorranceLighting_ambient_0(_S8, _S7, PBRSource_ambientRadiance_0(source_0, _S7));
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
            var radiance_4 : vec3<f32> = radiance_3 + LinearCookTorranceLighting_direct_0(_S8, _S7, PBRSource_light_0(source_0, i_1, _S7));
            i_1 = i_1 + u32(1);
            radiance_3 = radiance_4;
        }
        (*f_8).color_0 = vec4<f32>(LinearCookTorranceLighting_resolve_0(radiance_3, _S8) + _S8.emissive_2, _S8.opacity_2);
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

fn wrapIndex_0( i_2 : i32,  size_0 : i32,  mode_0 : u32) -> i32
{
    if(mode_0 == u32(1))
    {
        var _S12 : i32 = i_2 % size_0;
        var _S13 : i32 = (_S12 + size_0) % size_0;
        return _S13;
    }
    if(mode_0 == u32(2))
    {
        var _S14 : i32;
        if(i_2 < i32(0))
        {
            _S14 = i32(1);
        }
        else
        {
            _S14 = i32(0);
        }
        var _S15 : i32 = i32(2) * size_0;
        var x_0 : i32 = abs(i_2 + _S14) % _S15;
        if(x_0 >= size_0)
        {
            _S14 = _S15 - x_0 - i32(1);
        }
        else
        {
            _S14 = x_0;
        }
        return _S14;
    }
    return clamp(i_2, i32(0), size_0 - i32(1));
}

fn PBRTexels_texel_0( i_3 : u32) -> vec4<f32>
{
    return texels_0[i_3];
}

fn sampleImage2D_0( _S16 : vec4<f32>) -> vec4<f32>
{
    var a_0 : vec4<f32> = (((_S16) * (mat4x4<f32>(params_0.image_0.inTransform_0.data_0[i32(0)][i32(0)], params_0.image_0.inTransform_0.data_0[i32(1)][i32(0)], params_0.image_0.inTransform_0.data_0[i32(2)][i32(0)], params_0.image_0.inTransform_0.data_0[i32(3)][i32(0)], params_0.image_0.inTransform_0.data_0[i32(0)][i32(1)], params_0.image_0.inTransform_0.data_0[i32(1)][i32(1)], params_0.image_0.inTransform_0.data_0[i32(2)][i32(1)], params_0.image_0.inTransform_0.data_0[i32(3)][i32(1)], params_0.image_0.inTransform_0.data_0[i32(0)][i32(2)], params_0.image_0.inTransform_0.data_0[i32(1)][i32(2)], params_0.image_0.inTransform_0.data_0[i32(2)][i32(2)], params_0.image_0.inTransform_0.data_0[i32(3)][i32(2)], params_0.image_0.inTransform_0.data_0[i32(0)][i32(3)], params_0.image_0.inTransform_0.data_0[i32(1)][i32(3)], params_0.image_0.inTransform_0.data_0[i32(2)][i32(3)], params_0.image_0.inTransform_0.data_0[i32(3)][i32(3)])))) + params_0.image_0.inOffset_0;
    var _S17 : u32 = params_0.image_0.wrap1_0;
    var x_1 : f32;
    if((params_0.image_0.wrap1_0) == u32(1))
    {
        var _S18 : f32 = a_0.x;
        x_1 = _S18 - floor(_S18);
    }
    else
    {
        x_1 = a_0.x;
    }
    var _S19 : u32 = params_0.image_0.wrap2_0;
    var y_0 : f32;
    if((params_0.image_0.wrap2_0) == u32(1))
    {
        var _S20 : f32 = a_0.y;
        y_0 = _S20 - floor(_S20);
    }
    else
    {
        y_0 = a_0.y;
    }
    var w_1 : i32 = i32(params_0.image_0.width_0);
    var h_0 : i32 = i32(params_0.image_0.height_0);
    var _S21 : f32 = f32(w_1);
    var lx_0 : f32 = (x_1 - 0.5f / _S21) * _S21;
    var _S22 : f32 = f32(h_0);
    var ly_0 : f32 = (y_0 - 0.5f / _S22) * _S22;
    var x0_0 : i32 = i32(floor(lx_0));
    var y0_0 : i32 = i32(floor(ly_0));
    var fx_0 : f32 = lx_0 - f32(x0_0);
    var fy_0 : f32 = ly_0 - f32(y0_0);
    var xl_0 : i32 = wrapIndex_0(x0_0, w_1, _S17);
    var xu_0 : i32 = wrapIndex_0(x0_0 + i32(1), w_1, _S17);
    var yl_0 : i32 = wrapIndex_0(y0_0, h_0, _S19);
    var yu_0 : i32 = wrapIndex_0(y0_0 + i32(1), h_0, _S19);
    var _S23 : i32 = yl_0 * w_1;
    var _S24 : vec4<f32> = PBRTexels_texel_0(u32(_S23 + xl_0));
    var _S25 : i32 = yu_0 * w_1;
    var _S26 : vec4<f32> = PBRTexels_texel_0(u32(_S25 + xl_0));
    var _S27 : vec4<f32> = PBRTexels_texel_0(u32(_S23 + xu_0));
    var _S28 : vec4<f32> = PBRTexels_texel_0(u32(_S25 + xu_0));
    var v_2 : vec4<f32>;
    if((params_0.image_0.linearFilter_0) != u32(0))
    {
        var _S29 : vec4<f32> = vec4<f32>(fy_0);
        v_2 = mix(mix(_S24, _S26, _S29), mix(_S27, _S28, _S29), vec4<f32>(fx_0));
    }
    else
    {
        if(fx_0 < 0.5f)
        {
            if(fy_0 < 0.5f)
            {
                v_2 = _S24;
            }
            else
            {
                v_2 = _S26;
            }
        }
        else
        {
            if(fy_0 < 0.5f)
            {
                v_2 = _S27;
            }
            else
            {
                v_2 = _S28;
            }
        }
    }
    return (((v_2) * (mat4x4<f32>(params_0.image_0.outTransform_0.data_0[i32(0)][i32(0)], params_0.image_0.outTransform_0.data_0[i32(1)][i32(0)], params_0.image_0.outTransform_0.data_0[i32(2)][i32(0)], params_0.image_0.outTransform_0.data_0[i32(3)][i32(0)], params_0.image_0.outTransform_0.data_0[i32(0)][i32(1)], params_0.image_0.outTransform_0.data_0[i32(1)][i32(1)], params_0.image_0.outTransform_0.data_0[i32(2)][i32(1)], params_0.image_0.outTransform_0.data_0[i32(3)][i32(1)], params_0.image_0.outTransform_0.data_0[i32(0)][i32(2)], params_0.image_0.outTransform_0.data_0[i32(1)][i32(2)], params_0.image_0.outTransform_0.data_0[i32(2)][i32(2)], params_0.image_0.outTransform_0.data_0[i32(3)][i32(2)], params_0.image_0.outTransform_0.data_0[i32(0)][i32(3)], params_0.image_0.outTransform_0.data_0[i32(1)][i32(3)], params_0.image_0.outTransform_0.data_0[i32(2)][i32(3)], params_0.image_0.outTransform_0.data_0[i32(3)][i32(3)])))) + params_0.image_0.outOffset_0;
}

fn PBRSource_colors_0( this_6 : PBRSource_0,  f_10 : ptr<function, FragmentContext_0>)
{
    var baseRGB_0 : vec3<f32> = params_0.baseColor_0.xyz;
    var baseRGB_1 : vec3<f32>;
    if((params_0.hasColors_0) != u32(0))
    {
        var vc_0 : vec4<f32> = this_6.r_0.v0_0.color_2 * vec4<f32>(this_6.w_0.x) + this_6.r_0.v1_0.color_2 * vec4<f32>(this_6.w_0.y) + this_6.r_0.v2_0.color_2 * vec4<f32>(this_6.w_0.z);
        var vc_1 : vec4<f32>;
        if((params_0.image_0.enabled_0) != u32(0))
        {
            var _S30 : vec4<f32> = sampleImage2D_0(vc_0);
            vc_1 = _S30;
        }
        else
        {
            vc_1 = vc_0;
        }
        baseRGB_1 = baseRGB_0 * vc_1.xyz;
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
    var _S31 : CameraSample_0;
    _S31.shadingToClip_0 = shadingToClip_1;
    _S31.parallel_0 = parallel_1;
    return _S31;
}

fn HostMaterial_x24init_0( baseColor_4 : vec3<f32>,  opacity_4 : f32,  metallic_3 : f32,  roughness_4 : f32,  emissive_4 : vec3<f32>,  ambientColor_4 : vec3<f32>,  specularColor_4 : vec3<f32>,  specularPower_4 : f32) -> HostMaterial_0
{
    var _S32 : HostMaterial_0;
    _S32.baseColor_1 = baseColor_4;
    _S32.opacity_1 = opacity_4;
    _S32.metallic_0 = metallic_3;
    _S32.roughness_0 = roughness_4;
    _S32.emissive_1 = emissive_4;
    _S32.ambientColor_1 = ambientColor_4;
    _S32.specularColor_1 = specularColor_4;
    _S32.specularPower_1 = specularPower_4;
    return _S32;
}

fn SurfaceSample_x24init_0( position_2 : vec3<f32>,  normal_2 : vec3<f32>,  tangent_2 : vec3<f32>,  viewDir_1 : vec3<f32>,  uv_4 : vec2<f32>,  scalar_1 : f32,  fragCoord_2 : vec4<f32>,  primitiveId_4 : u32,  cellId_4 : u32,  frontFacing_2 : bool,  userData_1 : vec4<u32>,  camera_1 : CameraSample_0,  host_1 : HostMaterial_0) -> SurfaceSample_0
{
    var _S33 : SurfaceSample_0;
    _S33.position_1 = position_2;
    _S33.normal_1 = normal_2;
    _S33.tangent_1 = tangent_2;
    _S33.viewDir_0 = viewDir_1;
    _S33.uv_3 = uv_4;
    _S33.scalar_0 = scalar_1;
    _S33.fragCoord_1 = fragCoord_2;
    _S33.primitiveId_3 = primitiveId_4;
    _S33.cellId_3 = cellId_4;
    _S33.frontFacing_1 = frontFacing_2;
    _S33.userData_0 = userData_1;
    _S33.camera_0 = camera_1;
    _S33.host_0 = host_1;
    return _S33;
}

fn surfaceSample_0( f_11 : FragmentContext_0) -> SurfaceSample_0
{
    var _S34 : vec3<f32> = vec3<f32>(0.0f);
    const _S35 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 1.0f);
    var s_12 : SurfaceSample_0 = SurfaceSample_x24init_0(_S34, _S35, _S34, _S35, vec2<f32>(0.0f), 0.0f, vec4<f32>(0.0f), u32(0), u32(0), true, vec4<u32>(u32(0)), CameraSample_x24init_0(mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f), false), HostMaterial_x24init_0(vec3<f32>(1.0f), 1.0f, 0.0f, 1.0f, _S34, _S34, _S34, 1.0f));
    var _S36 : vec3<f32> = f_11.positionVC_0.xyz;
    s_12.position_1 = _S36;
    s_12.normal_1 = f_11.normalVC_0;
    s_12.tangent_1 = f_11.tangentVC_0;
    s_12.viewDir_0 = normalize((vec3<f32>(0) - _S36));
    s_12.uv_3 = f_11.uv_0;
    s_12.fragCoord_1 = f_11.fragCoord_0;
    s_12.primitiveId_3 = f_11.primitiveId_0;
    s_12.cellId_3 = f_11.cellId_0;
    s_12.frontFacing_1 = f_11.frontFacing_0;
    s_12.host_0.baseColor_1 = f_11.diffuseColor_0;
    s_12.host_0.ambientColor_1 = f_11.ambientColor_0;
    s_12.host_0.specularColor_1 = f_11.specularColor_0;
    s_12.host_0.specularPower_1 = f_11.specularPower_0;
    s_12.host_0.opacity_1 = f_11.opacity_0;
    return s_12;
}

fn PBRSource_sample_0( this_7 : PBRSource_0,  f_12 : FragmentContext_0) -> SurfaceSample_0
{
    var s_13 : SurfaceSample_0 = surfaceSample_0(f_12);
    var ndcC_0 : vec3<f32> = this_7.r_0.t_0.ndc0_0 * vec3<f32>(this_7.w_0.x) + this_7.r_0.t_0.ndc1_0 * vec3<f32>(this_7.w_0.y) + this_7.r_0.t_0.ndc2_0 * vec3<f32>(this_7.w_0.z);
    s_13.viewDir_0 = normalize(vec3<f32>(- ndcC_0.x, - ndcC_0.y, 1.0f));
    s_13.camera_0.shadingToClip_0 = mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]);
    s_13.host_0.metallic_0 = params_0.metallicRoughness_0.x;
    s_13.host_0.roughness_0 = params_0.metallicRoughness_0.y;
    s_13.host_0.emissive_1 = params_0.emissive_0.xyz * vec3<f32>(params_0.emissive_0.w);
    return s_13;
}

fn StandardOutput_keep_0( s_14 : SurfaceSample_0) -> bool
{
    return true;
}

fn StandardOutput_depth_0( depth_1 : f32,  s_15 : SurfaceSample_0) -> f32
{
    return depth_1;
}

fn surfaceCoverage_0( source_1 : PBRSource_0,  f_13 : ptr<function, FragmentContext_0>,  s_16 : ptr<function, SurfaceSample_0>) -> bool
{
    var _S37 : bool = IShaderHooks_clipx24defaultImpl_0(&((*f_13)));
    PBRSource_colors_0(source_1, &((*f_13)));
    IShaderHooks_colorx24defaultImpl_0(&((*f_13)));
    ISurfaceSource_geometryx24defaultImpl_0(source_1, &((*f_13)));
    IShaderHooks_normalx24defaultImpl_0(&((*f_13)));
    var _S38 : SurfaceSample_0 = PBRSource_sample_0(source_1, (*f_13));
    (*s_16) = _S38;
    var covered_0 : bool;
    if(!StandardOutput_keep_0(_S38))
    {
        covered_0 = false;
    }
    else
    {
        covered_0 = _S37;
    }
    (*f_13).depth_0 = StandardOutput_depth_0(ISurfaceSource_depthx24defaultImpl_0(source_1, (*f_13), (*s_16)), (*s_16));
    return covered_0;
}

fn projectVertex_0( c_0 : ptr<function, VertexContext_0>,  positionClip_1 : vec4<f32>) -> vec4<f32>
{
    (*c_0).positionClip_0 = positionClip_1;
    IShaderHooks_positionx24defaultImpl_0(&((*c_0)));
    return (*c_0).positionClip_0;
}

fn IdentityVertex_modify_0( v_3 : VertexSample_0) -> VertexSample_0
{
    return v_3;
}

fn VertexContext_x24init_0( positionMC_1 : vec4<f32>,  normalMC_1 : vec3<f32>,  tangentMC_1 : vec3<f32>,  uv_5 : vec2<f32>,  color_5 : vec4<f32>,  positionClip_2 : vec4<f32>,  pointId_2 : u32,  cellId_5 : u32,  primitiveId_5 : u32,  modelToClip_2 : mat4x4<f32>,  normalModelToView_1 : mat3x3<f32>) -> VertexContext_0
{
    var _S39 : VertexContext_0;
    _S39.positionMC_0 = positionMC_1;
    _S39.normalMC_0 = normalMC_1;
    _S39.tangentMC_0 = tangentMC_1;
    _S39.uv_1 = uv_5;
    _S39.color_1 = color_5;
    _S39.positionClip_0 = positionClip_2;
    _S39.pointId_0 = pointId_2;
    _S39.cellId_1 = cellId_5;
    _S39.primitiveId_1 = primitiveId_5;
    _S39.modelToClip_0 = modelToClip_2;
    _S39.normalModelToView_0 = normalModelToView_1;
    return _S39;
}

fn vertexContext_0( v_4 : VertexSample_0) -> VertexContext_0
{
    const _S40 : vec4<f32> = vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f);
    var c_1 : VertexContext_0 = VertexContext_x24init_0(_S40, vec3<f32>(0.0f, 0.0f, 1.0f), vec3<f32>(0.0f), vec2<f32>(0.0f), vec4<f32>(1.0f), _S40, u32(0), u32(0), u32(0), mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f), mat3x3<f32>(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    c_1.positionMC_0 = vec4<f32>(v_4.position_0, 1.0f);
    c_1.normalMC_0 = v_4.normal_0;
    c_1.tangentMC_0 = v_4.tangent_0;
    c_1.uv_1 = v_4.uv_2;
    c_1.color_1 = v_4.color_2;
    c_1.pointId_0 = v_4.pointId_1;
    c_1.cellId_1 = v_4.cellId_2;
    c_1.primitiveId_1 = v_4.primitiveId_2;
    c_1.modelToClip_0 = v_4.modelToClip_1;
    return c_1;
}

fn vertexSample_0( c_2 : VertexContext_0,  v_5 : VertexSample_0) -> VertexSample_0
{
    var _S41 : VertexSample_0 = v_5;
    _S41.position_0 = c_2.positionMC_0.xyz;
    _S41.normal_0 = c_2.normalMC_0;
    _S41.tangent_0 = c_2.tangentMC_0;
    _S41.uv_2 = c_2.uv_1;
    _S41.color_2 = c_2.color_1;
    return _S41;
}

fn shadeVertex_0( v_6 : VertexSample_0,  c_3 : ptr<function, VertexContext_0>) -> VertexSample_0
{
    var _S42 : VertexSample_0 = IdentityVertex_modify_0(v_6);
    (*c_3) = vertexContext_0(_S42);
    IShaderHooks_vertexx24defaultImpl_0(&((*c_3)));
    return vertexSample_0((*c_3), _S42);
}

fn VertexSample_x24init_0( position_3 : vec3<f32>,  normal_3 : vec3<f32>,  tangent_3 : vec3<f32>,  uv_6 : vec2<f32>,  color_6 : vec4<f32>,  pointId_3 : u32,  cellId_6 : u32,  primitiveId_6 : u32,  modelToClip_3 : mat4x4<f32>) -> VertexSample_0
{
    var _S43 : VertexSample_0;
    _S43.position_0 = position_3;
    _S43.normal_0 = normal_3;
    _S43.tangent_0 = tangent_3;
    _S43.uv_2 = uv_6;
    _S43.color_2 = color_6;
    _S43.pointId_1 = pointId_3;
    _S43.cellId_2 = cellId_6;
    _S43.primitiveId_2 = primitiveId_6;
    _S43.modelToClip_1 = modelToClip_3;
    return _S43;
}

fn ScreenTriangle_x24init_0( s0_1 : vec3<f32>,  s1_1 : vec3<f32>,  s2_1 : vec3<f32>,  ndc0_1 : vec3<f32>,  ndc1_1 : vec3<f32>,  ndc2_1 : vec3<f32>,  area_1 : f32,  minX_1 : f32,  maxX_1 : f32,  minY_1 : f32,  maxY_1 : f32) -> ScreenTriangle_0
{
    var _S44 : ScreenTriangle_0;
    _S44.s0_0 = s0_1;
    _S44.s1_0 = s1_1;
    _S44.s2_0 = s2_1;
    _S44.ndc0_0 = ndc0_1;
    _S44.ndc1_0 = ndc1_1;
    _S44.ndc2_0 = ndc2_1;
    _S44.area_0 = area_1;
    _S44.minX_0 = minX_1;
    _S44.maxX_0 = maxX_1;
    _S44.minY_0 = minY_1;
    _S44.maxY_0 = maxY_1;
    return _S44;
}

fn PBRTriangle_x24init_0( v0_1 : VertexSample_0,  v1_1 : VertexSample_0,  v2_1 : VertexSample_0,  t_2 : ScreenTriangle_0,  id_1 : u32) -> PBRTriangle_0
{
    var _S45 : PBRTriangle_0;
    _S45.v0_0 = v0_1;
    _S45.v1_0 = v1_1;
    _S45.v2_0 = v2_1;
    _S45.t_0 = t_2;
    _S45.id_0 = id_1;
    return _S45;
}

fn pbr_triangleIndices_0( triIdx_0 : u32) -> vec3<u32>
{
    var _S46 : u32 = triIdx_0 * u32(3);
    return vec3<u32>(indices_0[_S46], indices_0[_S46 + u32(1)], indices_0[_S46 + u32(2)]);
}

fn pbr_getPos_0( i_4 : u32) -> vec3<f32>
{
    var _S47 : u32 = i_4 * u32(3);
    return vec3<f32>(positions_0[_S47], positions_0[_S47 + u32(1)], positions_0[_S47 + u32(2)]);
}

fn pbr_getNorm_0( i_5 : u32) -> vec3<f32>
{
    var _S48 : u32 = i_5 * u32(3);
    return vec3<f32>(normals_0[_S48], normals_0[_S48 + u32(1)], normals_0[_S48 + u32(2)]);
}

fn pbr_getColor_0( i_6 : u32) -> vec4<f32>
{
    var _S49 : u32 = i_6 * u32(4);
    return vec4<f32>(colors_0[_S49], colors_0[_S49 + u32(1)], colors_0[_S49 + u32(2)], colors_0[_S49 + u32(3)]);
}

fn pbr_fetchVertex_0( triIdx_1 : u32,  idx_0 : u32,  c_4 : ptr<function, VertexContext_0>) -> VertexSample_0
{
    var _S50 : vec3<f32> = vec3<f32>(0.0f);
    var v_7 : VertexSample_0 = VertexSample_x24init_0(_S50, vec3<f32>(0.0f, 0.0f, 1.0f), _S50, vec2<f32>(0.0f), vec4<f32>(1.0f), u32(0), u32(0), u32(0), mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    v_7.position_0 = pbr_getPos_0(idx_0);
    if((params_0.hasNormals_0) != u32(0))
    {
        v_7.normal_0 = pbr_getNorm_0(idx_0);
    }
    if((params_0.hasColors_0) != u32(0))
    {
        var _S51 : u32;
        if((params_0.hasColors_0) == u32(2))
        {
            _S51 = triIdx_1;
        }
        else
        {
            _S51 = idx_0;
        }
        v_7.color_2 = pbr_getColor_0(_S51);
    }
    v_7.pointId_1 = idx_0;
    v_7.cellId_2 = triIdx_1;
    v_7.primitiveId_2 = triIdx_1;
    v_7.modelToClip_1 = mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]);
    var _S52 : VertexSample_0 = shadeVertex_0(v_7, &((*c_4)));
    return _S52;
}

fn edgeFunction_0( a_1 : vec2<f32>,  b_0 : vec2<f32>,  c_5 : vec2<f32>) -> f32
{
    var _S53 : f32 = a_1.x;
    var _S54 : f32 = b_0.x;
    var swap_0 : bool;
    if(_S53 > _S54)
    {
        swap_0 = true;
    }
    else
    {
        if(_S53 == _S54)
        {
            swap_0 = (a_1.y) > (b_0.y);
        }
        else
        {
            swap_0 = false;
        }
    }
    var p_0 : vec2<f32>;
    if(swap_0)
    {
        p_0 = b_0;
    }
    else
    {
        p_0 = a_1;
    }
    var q_0 : vec2<f32>;
    if(swap_0)
    {
        q_0 = a_1;
    }
    else
    {
        q_0 = b_0;
    }
    var _S55 : f32 = p_0.x;
    var _S56 : f32 = p_0.y;
    var e_0 : f32 = (c_5.x - _S55) * (q_0.y - _S56) - (c_5.y - _S56) * (q_0.x - _S55);
    var _S57 : f32;
    if(swap_0)
    {
        _S57 = - e_0;
    }
    else
    {
        _S57 = e_0;
    }
    return _S57;
}

fn setupScreenTriangleClip_0( c0_0 : vec4<f32>,  c1_0 : vec4<f32>,  c2_0 : vec4<f32>,  width_2 : u32,  height_2 : u32,  t_3 : ptr<function, ScreenTriangle_0>) -> bool
{
    var _S58 : vec3<f32> = vec3<f32>(0.0f);
    (*t_3) = ScreenTriangle_x24init_0(_S58, _S58, _S58, _S58, _S58, _S58, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    var _S59 : f32 = c0_0.w;
    var _S60 : bool;
    if(_S59 <= 0.0f)
    {
        _S60 = true;
    }
    else
    {
        _S60 = (c1_0.w) <= 0.0f;
    }
    if(_S60)
    {
        _S60 = true;
    }
    else
    {
        _S60 = (c2_0.w) <= 0.0f;
    }
    if(_S60)
    {
        return false;
    }
    var _S61 : vec3<f32> = c0_0.xyz / vec3<f32>(_S59);
    (*t_3).ndc0_0 = _S61;
    var _S62 : vec3<f32> = c1_0.xyz / vec3<f32>(c1_0.w);
    (*t_3).ndc1_0 = _S62;
    var _S63 : vec3<f32> = c2_0.xyz / vec3<f32>(c2_0.w);
    (*t_3).ndc2_0 = _S63;
    var W_0 : f32 = f32(width_2);
    var H_1 : f32 = f32(height_2);
    var _S64 : f32 = (_S61.x * 0.5f + 0.5f) * W_0;
    var _S65 : f32 = (1.0f - (_S61.y * 0.5f + 0.5f)) * H_1;
    (*t_3).s0_0 = vec3<f32>(_S64, _S65, _S61.z);
    var _S66 : f32 = (_S62.x * 0.5f + 0.5f) * W_0;
    var _S67 : f32 = (1.0f - (_S62.y * 0.5f + 0.5f)) * H_1;
    (*t_3).s1_0 = vec3<f32>(_S66, _S67, _S62.z);
    var _S68 : f32 = (_S63.x * 0.5f + 0.5f) * W_0;
    var _S69 : f32 = (1.0f - (_S63.y * 0.5f + 0.5f)) * H_1;
    (*t_3).s2_0 = vec3<f32>(_S68, _S69, _S63.z);
    (*t_3).minX_0 = max(0.0f, min(min(_S64, _S66), _S68));
    (*t_3).maxX_0 = min(W_0 - 1.0f, max(max(_S64, _S66), _S68));
    (*t_3).minY_0 = max(0.0f, min(min(_S65, _S67), _S69));
    (*t_3).maxY_0 = min(H_1 - 1.0f, max(max(_S65, _S67), _S69));
    var _S70 : f32 = edgeFunction_0(vec2<f32>(_S64, _S65), vec2<f32>(_S66, _S67), vec2<f32>(_S68, _S69));
    (*t_3).area_0 = _S70;
    return (abs(_S70)) >= 9.99999997475242708e-07f;
}

fn pbr_setupTriangle_0( triIdx_2 : u32,  r_2 : ptr<function, PBRTriangle_0>) -> bool
{
    var _S71 : vec3<f32> = vec3<f32>(0.0f);
    var _S72 : VertexSample_0 = VertexSample_x24init_0(_S71, vec3<f32>(0.0f, 0.0f, 1.0f), _S71, vec2<f32>(0.0f), vec4<f32>(1.0f), u32(0), u32(0), u32(0), mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    (*r_2) = PBRTriangle_x24init_0(_S72, _S72, _S72, ScreenTriangle_x24init_0(_S71, _S71, _S71, _S71, _S71, _S71, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), u32(0));
    (*r_2).id_0 = triIdx_2;
    var i_7 : vec3<u32> = pbr_triangleIndices_0(triIdx_2);
    var c0_1 : VertexContext_0;
    var _S73 : VertexSample_0 = pbr_fetchVertex_0(triIdx_2, i_7.x, &(c0_1));
    (*r_2).v0_0 = _S73;
    var c1_1 : VertexContext_0;
    var _S74 : VertexSample_0 = pbr_fetchVertex_0(triIdx_2, i_7.y, &(c1_1));
    (*r_2).v1_0 = _S74;
    var c2_1 : VertexContext_0;
    var _S75 : VertexSample_0 = pbr_fetchVertex_0(triIdx_2, i_7.z, &(c2_1));
    (*r_2).v2_0 = _S75;
    var _S76 : vec4<f32> = projectVertex_0(&(c0_1), (((c0_1.positionMC_0) * (mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)])))));
    var _S77 : vec4<f32> = projectVertex_0(&(c1_1), (((c1_1.positionMC_0) * (mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)])))));
    var _S78 : vec4<f32> = projectVertex_0(&(c2_1), (((c2_1.positionMC_0) * (mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)])))));
    var _S79 : ScreenTriangle_0 = (*r_2).t_0;
    var _S80 : bool = setupScreenTriangleClip_0(_S76, _S77, _S78, params_0.width_1, params_0.height_1, &(_S79));
    (*r_2).t_0 = _S79;
    return _S80;
}

fn barycentric_0( t_4 : ScreenTriangle_0,  px_0 : i32,  py_0 : i32) -> vec3<f32>
{
    var p_1 : vec2<f32> = vec2<f32>(f32(px_0) + 0.5f, f32(py_0) + 0.5f);
    var _S81 : vec2<f32> = t_4.s1_0.xy;
    var _S82 : vec2<f32> = t_4.s2_0.xy;
    var _S83 : vec2<f32> = t_4.s0_0.xy;
    return vec3<f32>(edgeFunction_0(_S81, _S82, p_1) / t_4.area_0, edgeFunction_0(_S82, _S83, p_1) / t_4.area_0, edgeFunction_0(_S83, _S81, p_1) / t_4.area_0);
}

fn covered_1( w_2 : vec3<f32>) -> bool
{
    var _S84 : bool;
    if((w_2.x) >= 0.0f)
    {
        _S84 = (w_2.y) >= 0.0f;
    }
    else
    {
        _S84 = false;
    }
    if(_S84)
    {
        _S84 = (w_2.z) >= 0.0f;
    }
    else
    {
        _S84 = false;
    }
    return _S84;
}

fn PBRSource_x24init_0( r_3 : PBRTriangle_0,  w_3 : vec3<f32>) -> PBRSource_0
{
    var _S85 : PBRSource_0;
    _S85.r_0 = r_3;
    _S85.w_0 = w_3;
    return _S85;
}

fn interpolateDepth_0( t_5 : ScreenTriangle_0,  w_4 : vec3<f32>) -> f32
{
    return w_4.x * t_5.s0_0.z + w_4.y * t_5.s1_0.z + w_4.z * t_5.s2_0.z;
}

fn FragmentContext_x24init_0( positionVC_1 : vec4<f32>,  normalVC_1 : vec3<f32>,  tangentVC_1 : vec3<f32>,  uv_7 : vec2<f32>,  ambientColor_5 : vec3<f32>,  diffuseColor_1 : vec3<f32>,  specularColor_5 : vec3<f32>,  specularPower_5 : f32,  opacity_5 : f32,  color_7 : vec4<f32>,  depth_2 : f32,  ids_1 : vec4<u32>,  fragCoord_3 : vec4<f32>,  primitiveId_7 : u32,  cellId_7 : u32,  frontFacing_3 : bool) -> FragmentContext_0
{
    var _S86 : FragmentContext_0;
    _S86.positionVC_0 = positionVC_1;
    _S86.normalVC_0 = normalVC_1;
    _S86.tangentVC_0 = tangentVC_1;
    _S86.uv_0 = uv_7;
    _S86.ambientColor_0 = ambientColor_5;
    _S86.diffuseColor_0 = diffuseColor_1;
    _S86.specularColor_0 = specularColor_5;
    _S86.specularPower_0 = specularPower_5;
    _S86.opacity_0 = opacity_5;
    _S86.color_0 = color_7;
    _S86.depth_0 = depth_2;
    _S86.ids_0 = ids_1;
    _S86.fragCoord_0 = fragCoord_3;
    _S86.primitiveId_0 = primitiveId_7;
    _S86.cellId_0 = cellId_7;
    _S86.frontFacing_0 = frontFacing_3;
    return _S86;
}

fn pbr_fragment_0( r_4 : PBRTriangle_0,  w_5 : vec3<f32>,  px_1 : i32,  py_1 : i32,  depth_3 : f32) -> FragmentContext_0
{
    const _S87 : vec4<f32> = vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f);
    var _S88 : vec3<f32> = vec3<f32>(0.0f);
    var f_14 : FragmentContext_0 = FragmentContext_x24init_0(_S87, vec3<f32>(0.0f, 0.0f, 1.0f), _S88, vec2<f32>(0.0f), _S88, vec3<f32>(1.0f), _S88, 1.0f, 1.0f, _S87, 0.0f, vec4<u32>(u32(0)), vec4<f32>(0.0f), u32(0), u32(0), true);
    var _S89 : f32 = w_5.x;
    var _S90 : vec3<f32> = vec3<f32>(_S89);
    var _S91 : f32 = w_5.y;
    var _S92 : vec3<f32> = vec3<f32>(_S91);
    var _S93 : f32 = w_5.z;
    var _S94 : vec3<f32> = vec3<f32>(_S93);
    f_14.positionVC_0 = vec4<f32>(r_4.v0_0.position_0 * _S90 + r_4.v1_0.position_0 * _S92 + r_4.v2_0.position_0 * _S94, 1.0f);
    if((params_0.hasNormals_0) != u32(0))
    {
        f_14.normalVC_0 = normalize(r_4.v0_0.normal_0 * _S90 + r_4.v1_0.normal_0 * _S92 + r_4.v2_0.normal_0 * _S94);
    }
    else
    {
        f_14.normalVC_0 = normalize(cross(r_4.v1_0.position_0 - r_4.v0_0.position_0, r_4.v2_0.position_0 - r_4.v0_0.position_0));
    }
    f_14.uv_0 = r_4.v0_0.uv_2 * vec2<f32>(_S89) + r_4.v1_0.uv_2 * vec2<f32>(_S91) + r_4.v2_0.uv_2 * vec2<f32>(_S93);
    f_14.fragCoord_0 = vec4<f32>(f32(px_1) + 0.5f, f32(py_1) + 0.5f, depth_3, 1.0f);
    f_14.depth_0 = depth_3;
    f_14.primitiveId_0 = r_4.id_0;
    f_14.cellId_0 = r_4.id_0;
    f_14.frontFacing_0 = (r_4.t_0.area_0) < 0.0f;
    return f_14;
}

fn pbr_fragmentDepth_0( source_2 : PBRSource_0,  f_15 : ptr<function, FragmentContext_0>,  s_17 : ptr<function, SurfaceSample_0>,  depth_4 : ptr<function, f32>) -> bool
{
    (*depth_4) = (*f_15).depth_0;
    var _S95 : bool = surfaceCoverage_0(source_2, &((*f_15)), &((*s_17)));
    if(!_S95)
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
    var _S96 : bool = pbr_setupTriangle_0(triIdx_3, &(r_5));
    if(!_S96)
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
            var w_6 : vec3<f32> = barycentric_0(r_5.t_0, px_2, py_2);
            if(!covered_1(w_6))
            {
                px_2 = px_2 + i32(1);
                continue;
            }
            var source_3 : PBRSource_0 = PBRSource_x24init_0(r_5, w_6);
            var f_16 : FragmentContext_0 = pbr_fragment_0(r_5, w_6, px_2, py_2, interpolateDepth_0(r_5.t_0, w_6));
            var s_18 : SurfaceSample_0;
            var depth_5 : f32;
            var _S97 : bool = pbr_fragmentDepth_0(source_3, &(f_16), &(s_18), &(depth_5));
            if(!_S97)
            {
                px_2 = px_2 + i32(1);
                continue;
            }
            var _S98 : u32 = atomicMin(&(depthBufI_0[u32(py_2) * params_0.width_1 + u32(px_2)]), depthToU32_0(depth_5));
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
    var _S99 : bool = pbr_setupTriangle_0(triIdx_4, &(r_6));
    if(!_S99)
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
            var w_7 : vec3<f32> = barycentric_0(r_6.t_0, px_3, py_3);
            if(!covered_1(w_7))
            {
                px_3 = px_3 + i32(1);
                continue;
            }
            var source_4 : PBRSource_0 = PBRSource_x24init_0(r_6, w_7);
            var f_17 : FragmentContext_0 = pbr_fragment_0(r_6, w_7, px_3, py_3, interpolateDepth_0(r_6.t_0, w_7));
            var s_19 : SurfaceSample_0;
            var depth_6 : f32;
            var _S100 : bool = pbr_fragmentDepth_0(source_4, &(f_17), &(s_19), &(depth_6));
            if(!_S100)
            {
                px_3 = px_3 + i32(1);
                continue;
            }
            var pixIdx_0 : u32 = u32(py_3) * params_0.width_1 + u32(px_3);
            var _S101 : u32 = depthToU32_0(depth_6);
            var _S102 : u32 = atomicLoad(&(depthBufI_0[pixIdx_0]));
            if(_S101 > (_S102 + u32(256)))
            {
                px_3 = px_3 + i32(1);
                continue;
            }
            var _S103 : bool = surfaceShade_0(source_4, &(f_17), s_19);
            if(!_S103)
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


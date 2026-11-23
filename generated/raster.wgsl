// Generated from shaders/raster.slang with its default specializations
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

struct RasterUniforms_std140_0
{
    @align(16) mvp_0 : _MatrixStorage_float4x4_ColMajorstd140_0,
    @align(16) materialColor_0 : vec4<f32>,
    @align(16) lightDir_0 : vec3<f32>,
    @align(4) ambientRadiance_0 : f32,
    @align(16) width_1 : u32,
    @align(4) height_1 : u32,
    @align(8) numTriangles_0 : u32,
    @align(4) hasNormals_0 : u32,
    @align(16) hasColors_0 : u32,
    @align(4) alphaMode_0 : u32,
    @align(8) alphaCutoff_0 : f32,
    @align(4) _pad0_1 : u32,
    @align(16) image_0 : Image2DParams_std140_0,
};

@binding(2) @group(0) var<uniform> params_0 : RasterUniforms_std140_0;
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

struct Triangle_0
{
     v0_0 : VertexSample_0,
     v1_0 : VertexSample_0,
     v2_0 : VertexSample_0,
     t_0 : ScreenTriangle_0,
     faceNormal_0 : vec3<f32>,
     id_0 : u32,
};

struct RasterSource_0
{
     r_0 : Triangle_0,
     w_0 : vec3<f32>,
};

struct CameraSample_0
{
     shadingToClip_0 : mat4x4<f32>,
     parallel_0 : bool,
};

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

fn ISurfaceSource_depthx24defaultImpl_0( this_0 : RasterSource_0,  f_6 : FragmentContext_0,  s_0 : SurfaceSample_0) -> f32
{
    return f_6.depth_0;
}

fn ISurfaceSource_geometryx24defaultImpl_0( this_1 : RasterSource_0,  f_7 : ptr<function, FragmentContext_0>)
{
    return;
}

fn GeometricNormal_normal_0( s_1 : SurfaceSample_0) -> vec3<f32>
{
    return s_1.normal_1;
}

struct MaterialSample_0
{
     baseColor_1 : vec3<f32>,
     opacity_2 : f32,
     metallic_1 : f32,
     roughness_1 : f32,
     emissive_1 : vec3<f32>,
     ambientColor_2 : vec3<f32>,
     specularColor_2 : vec3<f32>,
     specularPower_2 : f32,
};

fn MaterialSample_x24init_0( baseColor_2 : vec3<f32>,  opacity_3 : f32,  metallic_2 : f32,  roughness_2 : f32,  emissive_2 : vec3<f32>,  ambientColor_3 : vec3<f32>,  specularColor_3 : vec3<f32>,  specularPower_3 : f32) -> MaterialSample_0
{
    var _S1 : MaterialSample_0;
    _S1.baseColor_1 = baseColor_2;
    _S1.opacity_2 = opacity_3;
    _S1.metallic_1 = metallic_2;
    _S1.roughness_1 = roughness_2;
    _S1.emissive_1 = emissive_2;
    _S1.ambientColor_2 = ambientColor_3;
    _S1.specularColor_2 = specularColor_3;
    _S1.specularPower_2 = specularPower_3;
    return _S1;
}

fn HostDrivenMaterial_evaluate_0( s_2 : SurfaceSample_0) -> MaterialSample_0
{
    var _S2 : vec3<f32> = vec3<f32>(0.0f);
    var m_0 : MaterialSample_0 = MaterialSample_x24init_0(vec3<f32>(1.0f), 1.0f, 0.0f, 1.0f, _S2, _S2, _S2, 1.0f);
    m_0.baseColor_1 = s_2.host_0.baseColor_0;
    m_0.opacity_2 = s_2.host_0.opacity_1;
    m_0.metallic_1 = s_2.host_0.metallic_0;
    m_0.roughness_1 = s_2.host_0.roughness_0;
    m_0.emissive_1 = s_2.host_0.emissive_0;
    m_0.ambientColor_2 = s_2.host_0.ambientColor_1;
    m_0.specularColor_2 = s_2.host_0.specularColor_1;
    m_0.specularPower_2 = s_2.host_0.specularPower_1;
    return m_0;
}

fn RasterSource_lightCount_0( this_2 : RasterSource_0,  s_3 : SurfaceSample_0) -> u32
{
    return u32(1);
}

fn RasterSource_ambientRadiance_0( this_3 : RasterSource_0,  s_4 : SurfaceSample_0) -> vec3<f32>
{
    return vec3<f32>(params_0.ambientRadiance_0);
}

fn LambertLighting_ambient_0( m_1 : MaterialSample_0,  s_5 : SurfaceSample_0,  ambientRadiance_1 : vec3<f32>) -> vec3<f32>
{
    return m_1.baseColor_1 * ambientRadiance_1;
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

fn RasterSource_light_0( this_4 : RasterSource_0,  i_0 : u32,  s_6 : SurfaceSample_0) -> LightSample_0
{
    var _S4 : vec3<f32> = vec3<f32>(1.0f);
    var light_0 : LightSample_0 = LightSample_x24init_0(vec3<f32>(0.0f, 0.0f, 1.0f), _S4);
    light_0.direction_0 = normalize((vec3<f32>(0) - params_0.lightDir_0));
    light_0.radiance_0 = _S4;
    return light_0;
}

fn LambertLighting_direct_0( m_2 : MaterialSample_0,  s_7 : SurfaceSample_0,  light_1 : LightSample_0) -> vec3<f32>
{
    return m_2.baseColor_1 * light_1.radiance_0 * vec3<f32>(max(dot(s_7.normal_1, light_1.direction_0), 0.0f));
}

fn LambertLighting_resolve_0( radiance_2 : vec3<f32>,  m_3 : MaterialSample_0) -> vec3<f32>
{
    return min(radiance_2, m_3.baseColor_1);
}

fn RasterSource_opacity_0( this_5 : RasterSource_0,  m_4 : MaterialSample_0,  color_3 : ptr<function, vec4<f32>>) -> bool
{
    if((params_0.alphaMode_0) == u32(2))
    {
        if((m_4.opacity_2) < (params_0.alphaCutoff_0))
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

fn StandardOutput_finish_0( color_4 : vec4<f32>,  m_5 : MaterialSample_0,  s_8 : SurfaceSample_0) -> vec4<f32>
{
    return color_4;
}

fn surfaceShade_0( source_0 : RasterSource_0,  f_8 : ptr<function, FragmentContext_0>,  s_9 : SurfaceSample_0) -> bool
{
    var _S5 : SurfaceSample_0 = s_9;
    _S5.normal_1 = GeometricNormal_normal_0(s_9);
    var _S6 : MaterialSample_0 = HostDrivenMaterial_evaluate_0(_S5);
    var lights_0 : u32 = RasterSource_lightCount_0(source_0, _S5);
    if(lights_0 == u32(0))
    {
        (*f_8).color_0 = vec4<f32>(_S6.ambientColor_2 + _S6.baseColor_1, _S6.opacity_2);
    }
    else
    {
        var _S7 : vec3<f32> = LambertLighting_ambient_0(_S6, _S5, RasterSource_ambientRadiance_0(source_0, _S5));
        var i_1 : u32 = u32(0);
        var radiance_3 : vec3<f32> = _S7;
        for(;;)
        {
            if(i_1 < lights_0)
            {
            }
            else
            {
                break;
            }
            var radiance_4 : vec3<f32> = radiance_3 + LambertLighting_direct_0(_S6, _S5, RasterSource_light_0(source_0, i_1, _S5));
            i_1 = i_1 + u32(1);
            radiance_3 = radiance_4;
        }
        (*f_8).color_0 = vec4<f32>(LambertLighting_resolve_0(radiance_3, _S6) + _S6.emissive_1, _S6.opacity_2);
    }
    var _S8 : vec4<f32> = (*f_8).color_0;
    var opaque_0 : bool = RasterSource_opacity_0(source_0, _S6, &(_S8));
    (*f_8).color_0 = _S8;
    IShaderHooks_litx24defaultImpl_0(&((*f_8)));
    (*f_8).color_0 = StandardOutput_finish_0((*f_8).color_0, _S6, _S5);
    IShaderHooks_outputx24defaultImpl_1(&((*f_8)));
    return opaque_0;
}

fn outputDepth_0( f_9 : FragmentContext_0) -> f32
{
    var _S9 : FragmentContext_0 = f_9;
    IShaderHooks_outputx24defaultImpl_0(&(_S9));
    return _S9.depth_0;
}

fn wrapIndex_0( i_2 : i32,  size_0 : i32,  mode_0 : u32) -> i32
{
    if(mode_0 == u32(1))
    {
        var _S10 : i32 = i_2 % size_0;
        var _S11 : i32 = (_S10 + size_0) % size_0;
        return _S11;
    }
    if(mode_0 == u32(2))
    {
        var _S12 : i32;
        if(i_2 < i32(0))
        {
            _S12 = i32(1);
        }
        else
        {
            _S12 = i32(0);
        }
        var _S13 : i32 = i32(2) * size_0;
        var x_0 : i32 = abs(i_2 + _S12) % _S13;
        if(x_0 >= size_0)
        {
            _S12 = _S13 - x_0 - i32(1);
        }
        else
        {
            _S12 = x_0;
        }
        return _S12;
    }
    return clamp(i_2, i32(0), size_0 - i32(1));
}

fn Texels_texel_0( i_3 : u32) -> vec4<f32>
{
    return texels_0[i_3];
}

fn sampleImage2D_0( _S14 : vec4<f32>) -> vec4<f32>
{
    var a_0 : vec4<f32> = (((_S14) * (mat4x4<f32>(params_0.image_0.inTransform_0.data_0[i32(0)][i32(0)], params_0.image_0.inTransform_0.data_0[i32(1)][i32(0)], params_0.image_0.inTransform_0.data_0[i32(2)][i32(0)], params_0.image_0.inTransform_0.data_0[i32(3)][i32(0)], params_0.image_0.inTransform_0.data_0[i32(0)][i32(1)], params_0.image_0.inTransform_0.data_0[i32(1)][i32(1)], params_0.image_0.inTransform_0.data_0[i32(2)][i32(1)], params_0.image_0.inTransform_0.data_0[i32(3)][i32(1)], params_0.image_0.inTransform_0.data_0[i32(0)][i32(2)], params_0.image_0.inTransform_0.data_0[i32(1)][i32(2)], params_0.image_0.inTransform_0.data_0[i32(2)][i32(2)], params_0.image_0.inTransform_0.data_0[i32(3)][i32(2)], params_0.image_0.inTransform_0.data_0[i32(0)][i32(3)], params_0.image_0.inTransform_0.data_0[i32(1)][i32(3)], params_0.image_0.inTransform_0.data_0[i32(2)][i32(3)], params_0.image_0.inTransform_0.data_0[i32(3)][i32(3)])))) + params_0.image_0.inOffset_0;
    var _S15 : u32 = params_0.image_0.wrap1_0;
    var x_1 : f32;
    if((params_0.image_0.wrap1_0) == u32(1))
    {
        var _S16 : f32 = a_0.x;
        x_1 = _S16 - floor(_S16);
    }
    else
    {
        x_1 = a_0.x;
    }
    var _S17 : u32 = params_0.image_0.wrap2_0;
    var y_0 : f32;
    if((params_0.image_0.wrap2_0) == u32(1))
    {
        var _S18 : f32 = a_0.y;
        y_0 = _S18 - floor(_S18);
    }
    else
    {
        y_0 = a_0.y;
    }
    var w_1 : i32 = i32(params_0.image_0.width_0);
    var h_0 : i32 = i32(params_0.image_0.height_0);
    var _S19 : f32 = f32(w_1);
    var lx_0 : f32 = (x_1 - 0.5f / _S19) * _S19;
    var _S20 : f32 = f32(h_0);
    var ly_0 : f32 = (y_0 - 0.5f / _S20) * _S20;
    var x0_0 : i32 = i32(floor(lx_0));
    var y0_0 : i32 = i32(floor(ly_0));
    var fx_0 : f32 = lx_0 - f32(x0_0);
    var fy_0 : f32 = ly_0 - f32(y0_0);
    var xl_0 : i32 = wrapIndex_0(x0_0, w_1, _S15);
    var xu_0 : i32 = wrapIndex_0(x0_0 + i32(1), w_1, _S15);
    var yl_0 : i32 = wrapIndex_0(y0_0, h_0, _S17);
    var yu_0 : i32 = wrapIndex_0(y0_0 + i32(1), h_0, _S17);
    var _S21 : i32 = yl_0 * w_1;
    var _S22 : vec4<f32> = Texels_texel_0(u32(_S21 + xl_0));
    var _S23 : i32 = yu_0 * w_1;
    var _S24 : vec4<f32> = Texels_texel_0(u32(_S23 + xl_0));
    var _S25 : vec4<f32> = Texels_texel_0(u32(_S21 + xu_0));
    var _S26 : vec4<f32> = Texels_texel_0(u32(_S23 + xu_0));
    var v_2 : vec4<f32>;
    if((params_0.image_0.linearFilter_0) != u32(0))
    {
        var _S27 : vec4<f32> = vec4<f32>(fy_0);
        v_2 = mix(mix(_S22, _S24, _S27), mix(_S25, _S26, _S27), vec4<f32>(fx_0));
    }
    else
    {
        if(fx_0 < 0.5f)
        {
            if(fy_0 < 0.5f)
            {
                v_2 = _S22;
            }
            else
            {
                v_2 = _S24;
            }
        }
        else
        {
            if(fy_0 < 0.5f)
            {
                v_2 = _S25;
            }
            else
            {
                v_2 = _S26;
            }
        }
    }
    return (((v_2) * (mat4x4<f32>(params_0.image_0.outTransform_0.data_0[i32(0)][i32(0)], params_0.image_0.outTransform_0.data_0[i32(1)][i32(0)], params_0.image_0.outTransform_0.data_0[i32(2)][i32(0)], params_0.image_0.outTransform_0.data_0[i32(3)][i32(0)], params_0.image_0.outTransform_0.data_0[i32(0)][i32(1)], params_0.image_0.outTransform_0.data_0[i32(1)][i32(1)], params_0.image_0.outTransform_0.data_0[i32(2)][i32(1)], params_0.image_0.outTransform_0.data_0[i32(3)][i32(1)], params_0.image_0.outTransform_0.data_0[i32(0)][i32(2)], params_0.image_0.outTransform_0.data_0[i32(1)][i32(2)], params_0.image_0.outTransform_0.data_0[i32(2)][i32(2)], params_0.image_0.outTransform_0.data_0[i32(3)][i32(2)], params_0.image_0.outTransform_0.data_0[i32(0)][i32(3)], params_0.image_0.outTransform_0.data_0[i32(1)][i32(3)], params_0.image_0.outTransform_0.data_0[i32(2)][i32(3)], params_0.image_0.outTransform_0.data_0[i32(3)][i32(3)])))) + params_0.image_0.outOffset_0;
}

fn RasterSource_colors_0( this_6 : RasterSource_0,  f_10 : ptr<function, FragmentContext_0>)
{
    var color_5 : vec4<f32> = this_6.r_0.v0_0.color_2 * vec4<f32>(this_6.w_0.x) + this_6.r_0.v1_0.color_2 * vec4<f32>(this_6.w_0.y) + this_6.r_0.v2_0.color_2 * vec4<f32>(this_6.w_0.z);
    var color_6 : vec4<f32>;
    if((params_0.image_0.enabled_0) != u32(0))
    {
        var _S28 : vec4<f32> = sampleImage2D_0(color_5);
        color_6 = _S28;
    }
    else
    {
        color_6 = color_5;
    }
    var color_7 : vec4<f32> = color_6 * params_0.materialColor_0;
    (*f_10).diffuseColor_0 = color_7.xyz;
    (*f_10).opacity_0 = color_7.w;
    return;
}

fn CameraSample_x24init_0( shadingToClip_1 : mat4x4<f32>,  parallel_1 : bool) -> CameraSample_0
{
    var _S29 : CameraSample_0;
    _S29.shadingToClip_0 = shadingToClip_1;
    _S29.parallel_0 = parallel_1;
    return _S29;
}

fn HostMaterial_x24init_0( baseColor_3 : vec3<f32>,  opacity_4 : f32,  metallic_3 : f32,  roughness_3 : f32,  emissive_3 : vec3<f32>,  ambientColor_4 : vec3<f32>,  specularColor_4 : vec3<f32>,  specularPower_4 : f32) -> HostMaterial_0
{
    var _S30 : HostMaterial_0;
    _S30.baseColor_0 = baseColor_3;
    _S30.opacity_1 = opacity_4;
    _S30.metallic_0 = metallic_3;
    _S30.roughness_0 = roughness_3;
    _S30.emissive_0 = emissive_3;
    _S30.ambientColor_1 = ambientColor_4;
    _S30.specularColor_1 = specularColor_4;
    _S30.specularPower_1 = specularPower_4;
    return _S30;
}

fn SurfaceSample_x24init_0( position_2 : vec3<f32>,  normal_2 : vec3<f32>,  tangent_2 : vec3<f32>,  viewDir_1 : vec3<f32>,  uv_4 : vec2<f32>,  scalar_1 : f32,  fragCoord_2 : vec4<f32>,  primitiveId_4 : u32,  cellId_4 : u32,  frontFacing_2 : bool,  userData_1 : vec4<u32>,  camera_1 : CameraSample_0,  host_1 : HostMaterial_0) -> SurfaceSample_0
{
    var _S31 : SurfaceSample_0;
    _S31.position_1 = position_2;
    _S31.normal_1 = normal_2;
    _S31.tangent_1 = tangent_2;
    _S31.viewDir_0 = viewDir_1;
    _S31.uv_3 = uv_4;
    _S31.scalar_0 = scalar_1;
    _S31.fragCoord_1 = fragCoord_2;
    _S31.primitiveId_3 = primitiveId_4;
    _S31.cellId_3 = cellId_4;
    _S31.frontFacing_1 = frontFacing_2;
    _S31.userData_0 = userData_1;
    _S31.camera_0 = camera_1;
    _S31.host_0 = host_1;
    return _S31;
}

fn surfaceSample_0( f_11 : FragmentContext_0) -> SurfaceSample_0
{
    var _S32 : vec3<f32> = vec3<f32>(0.0f);
    const _S33 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 1.0f);
    var s_10 : SurfaceSample_0 = SurfaceSample_x24init_0(_S32, _S33, _S32, _S33, vec2<f32>(0.0f), 0.0f, vec4<f32>(0.0f), u32(0), u32(0), true, vec4<u32>(u32(0)), CameraSample_x24init_0(mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f), false), HostMaterial_x24init_0(vec3<f32>(1.0f), 1.0f, 0.0f, 1.0f, _S32, _S32, _S32, 1.0f));
    var _S34 : vec3<f32> = f_11.positionVC_0.xyz;
    s_10.position_1 = _S34;
    s_10.normal_1 = f_11.normalVC_0;
    s_10.tangent_1 = f_11.tangentVC_0;
    s_10.viewDir_0 = normalize((vec3<f32>(0) - _S34));
    s_10.uv_3 = f_11.uv_0;
    s_10.fragCoord_1 = f_11.fragCoord_0;
    s_10.primitiveId_3 = f_11.primitiveId_0;
    s_10.cellId_3 = f_11.cellId_0;
    s_10.frontFacing_1 = f_11.frontFacing_0;
    s_10.host_0.baseColor_0 = f_11.diffuseColor_0;
    s_10.host_0.ambientColor_1 = f_11.ambientColor_0;
    s_10.host_0.specularColor_1 = f_11.specularColor_0;
    s_10.host_0.specularPower_1 = f_11.specularPower_0;
    s_10.host_0.opacity_1 = f_11.opacity_0;
    return s_10;
}

fn RasterSource_sample_0( this_7 : RasterSource_0,  f_12 : FragmentContext_0) -> SurfaceSample_0
{
    var s_11 : SurfaceSample_0 = surfaceSample_0(f_12);
    s_11.viewDir_0 = vec3<f32>(0.0f, 0.0f, 1.0f);
    s_11.camera_0.shadingToClip_0 = mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]);
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

fn surfaceCoverage_0( source_1 : RasterSource_0,  f_13 : ptr<function, FragmentContext_0>,  s_14 : ptr<function, SurfaceSample_0>) -> bool
{
    var _S35 : bool = IShaderHooks_clipx24defaultImpl_0(&((*f_13)));
    RasterSource_colors_0(source_1, &((*f_13)));
    IShaderHooks_colorx24defaultImpl_0(&((*f_13)));
    ISurfaceSource_geometryx24defaultImpl_0(source_1, &((*f_13)));
    IShaderHooks_normalx24defaultImpl_0(&((*f_13)));
    var _S36 : SurfaceSample_0 = RasterSource_sample_0(source_1, (*f_13));
    (*s_14) = _S36;
    var covered_0 : bool;
    if(!StandardOutput_keep_0(_S36))
    {
        covered_0 = false;
    }
    else
    {
        covered_0 = _S35;
    }
    (*f_13).depth_0 = StandardOutput_depth_0(ISurfaceSource_depthx24defaultImpl_0(source_1, (*f_13), (*s_14)), (*s_14));
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

fn VertexContext_x24init_0( positionMC_1 : vec4<f32>,  normalMC_1 : vec3<f32>,  tangentMC_1 : vec3<f32>,  uv_5 : vec2<f32>,  color_8 : vec4<f32>,  positionClip_2 : vec4<f32>,  pointId_2 : u32,  cellId_5 : u32,  primitiveId_5 : u32,  modelToClip_2 : mat4x4<f32>,  normalModelToView_1 : mat3x3<f32>) -> VertexContext_0
{
    var _S37 : VertexContext_0;
    _S37.positionMC_0 = positionMC_1;
    _S37.normalMC_0 = normalMC_1;
    _S37.tangentMC_0 = tangentMC_1;
    _S37.uv_1 = uv_5;
    _S37.color_1 = color_8;
    _S37.positionClip_0 = positionClip_2;
    _S37.pointId_0 = pointId_2;
    _S37.cellId_1 = cellId_5;
    _S37.primitiveId_1 = primitiveId_5;
    _S37.modelToClip_0 = modelToClip_2;
    _S37.normalModelToView_0 = normalModelToView_1;
    return _S37;
}

fn vertexContext_0( v_4 : VertexSample_0) -> VertexContext_0
{
    const _S38 : vec4<f32> = vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f);
    var c_1 : VertexContext_0 = VertexContext_x24init_0(_S38, vec3<f32>(0.0f, 0.0f, 1.0f), vec3<f32>(0.0f), vec2<f32>(0.0f), vec4<f32>(1.0f), _S38, u32(0), u32(0), u32(0), mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f), mat3x3<f32>(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
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
    var _S39 : VertexSample_0 = v_5;
    _S39.position_0 = c_2.positionMC_0.xyz;
    _S39.normal_0 = c_2.normalMC_0;
    _S39.tangent_0 = c_2.tangentMC_0;
    _S39.uv_2 = c_2.uv_1;
    _S39.color_2 = c_2.color_1;
    return _S39;
}

fn shadeVertex_0( v_6 : VertexSample_0,  c_3 : ptr<function, VertexContext_0>) -> VertexSample_0
{
    var _S40 : VertexSample_0 = IdentityVertex_modify_0(v_6);
    (*c_3) = vertexContext_0(_S40);
    IShaderHooks_vertexx24defaultImpl_0(&((*c_3)));
    return vertexSample_0((*c_3), _S40);
}

fn VertexSample_x24init_0( position_3 : vec3<f32>,  normal_3 : vec3<f32>,  tangent_3 : vec3<f32>,  uv_6 : vec2<f32>,  color_9 : vec4<f32>,  pointId_3 : u32,  cellId_6 : u32,  primitiveId_6 : u32,  modelToClip_3 : mat4x4<f32>) -> VertexSample_0
{
    var _S41 : VertexSample_0;
    _S41.position_0 = position_3;
    _S41.normal_0 = normal_3;
    _S41.tangent_0 = tangent_3;
    _S41.uv_2 = uv_6;
    _S41.color_2 = color_9;
    _S41.pointId_1 = pointId_3;
    _S41.cellId_2 = cellId_6;
    _S41.primitiveId_2 = primitiveId_6;
    _S41.modelToClip_1 = modelToClip_3;
    return _S41;
}

fn ScreenTriangle_x24init_0( s0_1 : vec3<f32>,  s1_1 : vec3<f32>,  s2_1 : vec3<f32>,  ndc0_1 : vec3<f32>,  ndc1_1 : vec3<f32>,  ndc2_1 : vec3<f32>,  area_1 : f32,  minX_1 : f32,  maxX_1 : f32,  minY_1 : f32,  maxY_1 : f32) -> ScreenTriangle_0
{
    var _S42 : ScreenTriangle_0;
    _S42.s0_0 = s0_1;
    _S42.s1_0 = s1_1;
    _S42.s2_0 = s2_1;
    _S42.ndc0_0 = ndc0_1;
    _S42.ndc1_0 = ndc1_1;
    _S42.ndc2_0 = ndc2_1;
    _S42.area_0 = area_1;
    _S42.minX_0 = minX_1;
    _S42.maxX_0 = maxX_1;
    _S42.minY_0 = minY_1;
    _S42.maxY_0 = maxY_1;
    return _S42;
}

fn Triangle_x24init_0( v0_1 : VertexSample_0,  v1_1 : VertexSample_0,  v2_1 : VertexSample_0,  t_1 : ScreenTriangle_0,  faceNormal_1 : vec3<f32>,  id_1 : u32) -> Triangle_0
{
    var _S43 : Triangle_0;
    _S43.v0_0 = v0_1;
    _S43.v1_0 = v1_1;
    _S43.v2_0 = v2_1;
    _S43.t_0 = t_1;
    _S43.faceNormal_0 = faceNormal_1;
    _S43.id_0 = id_1;
    return _S43;
}

fn triangleIndices_0( triIdx_0 : u32) -> vec3<u32>
{
    var _S44 : u32 = triIdx_0 * u32(3);
    return vec3<u32>(indices_0[_S44], indices_0[_S44 + u32(1)], indices_0[_S44 + u32(2)]);
}

fn getPosition_0( i_4 : u32) -> vec3<f32>
{
    var _S45 : u32 = i_4 * u32(3);
    return vec3<f32>(positions_0[_S45], positions_0[_S45 + u32(1)], positions_0[_S45 + u32(2)]);
}

fn getNormal_0( i_5 : u32) -> vec3<f32>
{
    var _S46 : u32 = i_5 * u32(3);
    return vec3<f32>(normals_0[_S46], normals_0[_S46 + u32(1)], normals_0[_S46 + u32(2)]);
}

fn getColor_0( i_6 : u32) -> vec4<f32>
{
    var _S47 : u32 = i_6 * u32(4);
    return vec4<f32>(colors_0[_S47], colors_0[_S47 + u32(1)], colors_0[_S47 + u32(2)], colors_0[_S47 + u32(3)]);
}

fn fetchVertex_0( triIdx_1 : u32,  idx_0 : u32,  c_4 : ptr<function, VertexContext_0>) -> VertexSample_0
{
    var _S48 : vec3<f32> = vec3<f32>(0.0f);
    var v_7 : VertexSample_0 = VertexSample_x24init_0(_S48, vec3<f32>(0.0f, 0.0f, 1.0f), _S48, vec2<f32>(0.0f), vec4<f32>(1.0f), u32(0), u32(0), u32(0), mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    v_7.position_0 = getPosition_0(idx_0);
    if((params_0.hasNormals_0) != u32(0))
    {
        v_7.normal_0 = getNormal_0(idx_0);
    }
    if((params_0.hasColors_0) != u32(0))
    {
        var _S49 : u32;
        if((params_0.hasColors_0) == u32(2))
        {
            _S49 = triIdx_1;
        }
        else
        {
            _S49 = idx_0;
        }
        v_7.color_2 = getColor_0(_S49);
    }
    v_7.pointId_1 = idx_0;
    v_7.cellId_2 = triIdx_1;
    v_7.primitiveId_2 = triIdx_1;
    v_7.modelToClip_1 = mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)]);
    var _S50 : VertexSample_0 = shadeVertex_0(v_7, &((*c_4)));
    return _S50;
}

fn edgeFunction_0( a_1 : vec2<f32>,  b_0 : vec2<f32>,  c_5 : vec2<f32>) -> f32
{
    var _S51 : f32 = a_1.x;
    var _S52 : f32 = b_0.x;
    var swap_0 : bool;
    if(_S51 > _S52)
    {
        swap_0 = true;
    }
    else
    {
        if(_S51 == _S52)
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
    var _S53 : f32 = p_0.x;
    var _S54 : f32 = p_0.y;
    var e_0 : f32 = (c_5.x - _S53) * (q_0.y - _S54) - (c_5.y - _S54) * (q_0.x - _S53);
    var _S55 : f32;
    if(swap_0)
    {
        _S55 = - e_0;
    }
    else
    {
        _S55 = e_0;
    }
    return _S55;
}

fn setupScreenTriangleClip_0( c0_0 : vec4<f32>,  c1_0 : vec4<f32>,  c2_0 : vec4<f32>,  width_2 : u32,  height_2 : u32,  t_2 : ptr<function, ScreenTriangle_0>) -> bool
{
    var _S56 : vec3<f32> = vec3<f32>(0.0f);
    (*t_2) = ScreenTriangle_x24init_0(_S56, _S56, _S56, _S56, _S56, _S56, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    var _S57 : f32 = c0_0.w;
    var _S58 : bool;
    if(_S57 <= 0.0f)
    {
        _S58 = true;
    }
    else
    {
        _S58 = (c1_0.w) <= 0.0f;
    }
    if(_S58)
    {
        _S58 = true;
    }
    else
    {
        _S58 = (c2_0.w) <= 0.0f;
    }
    if(_S58)
    {
        return false;
    }
    var _S59 : vec3<f32> = c0_0.xyz / vec3<f32>(_S57);
    (*t_2).ndc0_0 = _S59;
    var _S60 : vec3<f32> = c1_0.xyz / vec3<f32>(c1_0.w);
    (*t_2).ndc1_0 = _S60;
    var _S61 : vec3<f32> = c2_0.xyz / vec3<f32>(c2_0.w);
    (*t_2).ndc2_0 = _S61;
    var W_0 : f32 = f32(width_2);
    var H_0 : f32 = f32(height_2);
    var _S62 : f32 = (_S59.x * 0.5f + 0.5f) * W_0;
    var _S63 : f32 = (1.0f - (_S59.y * 0.5f + 0.5f)) * H_0;
    (*t_2).s0_0 = vec3<f32>(_S62, _S63, _S59.z);
    var _S64 : f32 = (_S60.x * 0.5f + 0.5f) * W_0;
    var _S65 : f32 = (1.0f - (_S60.y * 0.5f + 0.5f)) * H_0;
    (*t_2).s1_0 = vec3<f32>(_S64, _S65, _S60.z);
    var _S66 : f32 = (_S61.x * 0.5f + 0.5f) * W_0;
    var _S67 : f32 = (1.0f - (_S61.y * 0.5f + 0.5f)) * H_0;
    (*t_2).s2_0 = vec3<f32>(_S66, _S67, _S61.z);
    (*t_2).minX_0 = max(0.0f, min(min(_S62, _S64), _S66));
    (*t_2).maxX_0 = min(W_0 - 1.0f, max(max(_S62, _S64), _S66));
    (*t_2).minY_0 = max(0.0f, min(min(_S63, _S65), _S67));
    (*t_2).maxY_0 = min(H_0 - 1.0f, max(max(_S63, _S65), _S67));
    var _S68 : f32 = edgeFunction_0(vec2<f32>(_S62, _S63), vec2<f32>(_S64, _S65), vec2<f32>(_S66, _S67));
    (*t_2).area_0 = _S68;
    return (abs(_S68)) >= 9.99999997475242708e-07f;
}

fn setupTriangle_0( triIdx_2 : u32,  r_1 : ptr<function, Triangle_0>) -> bool
{
    var _S69 : vec3<f32> = vec3<f32>(0.0f);
    var _S70 : VertexSample_0 = VertexSample_x24init_0(_S69, vec3<f32>(0.0f, 0.0f, 1.0f), _S69, vec2<f32>(0.0f), vec4<f32>(1.0f), u32(0), u32(0), u32(0), mat4x4<f32>(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    (*r_1) = Triangle_x24init_0(_S70, _S70, _S70, ScreenTriangle_x24init_0(_S69, _S69, _S69, _S69, _S69, _S69, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), _S69, u32(0));
    (*r_1).id_0 = triIdx_2;
    var i_7 : vec3<u32> = triangleIndices_0(triIdx_2);
    var c0_1 : VertexContext_0;
    var _S71 : VertexSample_0 = fetchVertex_0(triIdx_2, i_7.x, &(c0_1));
    (*r_1).v0_0 = _S71;
    var c1_1 : VertexContext_0;
    var _S72 : VertexSample_0 = fetchVertex_0(triIdx_2, i_7.y, &(c1_1));
    (*r_1).v1_0 = _S72;
    var c2_1 : VertexContext_0;
    var _S73 : VertexSample_0 = fetchVertex_0(triIdx_2, i_7.z, &(c2_1));
    (*r_1).v2_0 = _S73;
    var _S74 : vec4<f32> = projectVertex_0(&(c0_1), (((c0_1.positionMC_0) * (mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)])))));
    var _S75 : vec4<f32> = projectVertex_0(&(c1_1), (((c1_1.positionMC_0) * (mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)])))));
    var _S76 : vec4<f32> = projectVertex_0(&(c2_1), (((c2_1.positionMC_0) * (mat4x4<f32>(params_0.mvp_0.data_0[i32(0)][i32(0)], params_0.mvp_0.data_0[i32(1)][i32(0)], params_0.mvp_0.data_0[i32(2)][i32(0)], params_0.mvp_0.data_0[i32(3)][i32(0)], params_0.mvp_0.data_0[i32(0)][i32(1)], params_0.mvp_0.data_0[i32(1)][i32(1)], params_0.mvp_0.data_0[i32(2)][i32(1)], params_0.mvp_0.data_0[i32(3)][i32(1)], params_0.mvp_0.data_0[i32(0)][i32(2)], params_0.mvp_0.data_0[i32(1)][i32(2)], params_0.mvp_0.data_0[i32(2)][i32(2)], params_0.mvp_0.data_0[i32(3)][i32(2)], params_0.mvp_0.data_0[i32(0)][i32(3)], params_0.mvp_0.data_0[i32(1)][i32(3)], params_0.mvp_0.data_0[i32(2)][i32(3)], params_0.mvp_0.data_0[i32(3)][i32(3)])))));
    var _S77 : ScreenTriangle_0 = (*r_1).t_0;
    var _S78 : bool = setupScreenTriangleClip_0(_S74, _S75, _S76, params_0.width_1, params_0.height_1, &(_S77));
    (*r_1).t_0 = _S77;
    if(!_S78)
    {
        return false;
    }
    if((params_0.hasNormals_0) != u32(0))
    {
        (*r_1).faceNormal_0 = normalize(((*r_1).v0_0.normal_0 + (*r_1).v1_0.normal_0 + (*r_1).v2_0.normal_0) / vec3<f32>(3.0f));
    }
    else
    {
        (*r_1).faceNormal_0 = normalize(cross((*r_1).v1_0.position_0 - (*r_1).v0_0.position_0, (*r_1).v2_0.position_0 - (*r_1).v0_0.position_0));
    }
    return true;
}

fn barycentric_0( t_3 : ScreenTriangle_0,  px_0 : i32,  py_0 : i32) -> vec3<f32>
{
    var p_1 : vec2<f32> = vec2<f32>(f32(px_0) + 0.5f, f32(py_0) + 0.5f);
    var _S79 : vec2<f32> = t_3.s1_0.xy;
    var _S80 : vec2<f32> = t_3.s2_0.xy;
    var _S81 : vec2<f32> = t_3.s0_0.xy;
    return vec3<f32>(edgeFunction_0(_S79, _S80, p_1) / t_3.area_0, edgeFunction_0(_S80, _S81, p_1) / t_3.area_0, edgeFunction_0(_S81, _S79, p_1) / t_3.area_0);
}

fn covered_1( w_2 : vec3<f32>) -> bool
{
    var _S82 : bool;
    if((w_2.x) >= 0.0f)
    {
        _S82 = (w_2.y) >= 0.0f;
    }
    else
    {
        _S82 = false;
    }
    if(_S82)
    {
        _S82 = (w_2.z) >= 0.0f;
    }
    else
    {
        _S82 = false;
    }
    return _S82;
}

fn RasterSource_x24init_0( r_2 : Triangle_0,  w_3 : vec3<f32>) -> RasterSource_0
{
    var _S83 : RasterSource_0;
    _S83.r_0 = r_2;
    _S83.w_0 = w_3;
    return _S83;
}

fn interpolateDepth_0( t_4 : ScreenTriangle_0,  w_4 : vec3<f32>) -> f32
{
    return w_4.x * t_4.s0_0.z + w_4.y * t_4.s1_0.z + w_4.z * t_4.s2_0.z;
}

fn FragmentContext_x24init_0( positionVC_1 : vec4<f32>,  normalVC_1 : vec3<f32>,  tangentVC_1 : vec3<f32>,  uv_7 : vec2<f32>,  ambientColor_5 : vec3<f32>,  diffuseColor_1 : vec3<f32>,  specularColor_5 : vec3<f32>,  specularPower_5 : f32,  opacity_5 : f32,  color_10 : vec4<f32>,  depth_2 : f32,  ids_1 : vec4<u32>,  fragCoord_3 : vec4<f32>,  primitiveId_7 : u32,  cellId_7 : u32,  frontFacing_3 : bool) -> FragmentContext_0
{
    var _S84 : FragmentContext_0;
    _S84.positionVC_0 = positionVC_1;
    _S84.normalVC_0 = normalVC_1;
    _S84.tangentVC_0 = tangentVC_1;
    _S84.uv_0 = uv_7;
    _S84.ambientColor_0 = ambientColor_5;
    _S84.diffuseColor_0 = diffuseColor_1;
    _S84.specularColor_0 = specularColor_5;
    _S84.specularPower_0 = specularPower_5;
    _S84.opacity_0 = opacity_5;
    _S84.color_0 = color_10;
    _S84.depth_0 = depth_2;
    _S84.ids_0 = ids_1;
    _S84.fragCoord_0 = fragCoord_3;
    _S84.primitiveId_0 = primitiveId_7;
    _S84.cellId_0 = cellId_7;
    _S84.frontFacing_0 = frontFacing_3;
    return _S84;
}

fn rasterFragment_0( r_3 : Triangle_0,  w_5 : vec3<f32>,  px_1 : i32,  py_1 : i32,  depth_3 : f32) -> FragmentContext_0
{
    const _S85 : vec4<f32> = vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f);
    var _S86 : vec3<f32> = vec3<f32>(0.0f);
    var f_14 : FragmentContext_0 = FragmentContext_x24init_0(_S85, vec3<f32>(0.0f, 0.0f, 1.0f), _S86, vec2<f32>(0.0f), _S86, vec3<f32>(1.0f), _S86, 1.0f, 1.0f, _S85, 0.0f, vec4<u32>(u32(0)), vec4<f32>(0.0f), u32(0), u32(0), true);
    var _S87 : f32 = w_5.x;
    var _S88 : f32 = w_5.y;
    var _S89 : f32 = w_5.z;
    f_14.positionVC_0 = vec4<f32>(r_3.v0_0.position_0 * vec3<f32>(_S87) + r_3.v1_0.position_0 * vec3<f32>(_S88) + r_3.v2_0.position_0 * vec3<f32>(_S89), 1.0f);
    f_14.normalVC_0 = r_3.faceNormal_0;
    f_14.uv_0 = r_3.v0_0.uv_2 * vec2<f32>(_S87) + r_3.v1_0.uv_2 * vec2<f32>(_S88) + r_3.v2_0.uv_2 * vec2<f32>(_S89);
    f_14.fragCoord_0 = vec4<f32>(f32(px_1) + 0.5f, f32(py_1) + 0.5f, depth_3, 1.0f);
    f_14.depth_0 = depth_3;
    f_14.primitiveId_0 = r_3.id_0;
    f_14.cellId_0 = r_3.id_0;
    f_14.frontFacing_0 = (r_3.t_0.area_0) < 0.0f;
    return f_14;
}

fn fragmentDepth_0( source_2 : RasterSource_0,  f_15 : ptr<function, FragmentContext_0>,  s_15 : ptr<function, SurfaceSample_0>,  depth_4 : ptr<function, f32>) -> bool
{
    (*depth_4) = (*f_15).depth_0;
    var _S90 : bool = surfaceCoverage_0(source_2, &((*f_15)), &((*s_15)));
    if(!_S90)
    {
        return false;
    }
    (*depth_4) = outputDepth_0((*f_15));
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
    var r_4 : Triangle_0;
    var _S91 : bool = setupTriangle_0(triIdx_3, &(r_4));
    if(!_S91)
    {
        return;
    }
    var py_2 : i32 = i32(r_4.t_0.minY_0);
    for(;;)
    {
        if(py_2 <= i32(r_4.t_0.maxY_0))
        {
        }
        else
        {
            break;
        }
        var px_2 : i32 = i32(r_4.t_0.minX_0);
        for(;;)
        {
            if(px_2 <= i32(r_4.t_0.maxX_0))
            {
            }
            else
            {
                break;
            }
            var w_6 : vec3<f32> = barycentric_0(r_4.t_0, px_2, py_2);
            if(!covered_1(w_6))
            {
                px_2 = px_2 + i32(1);
                continue;
            }
            var source_3 : RasterSource_0 = RasterSource_x24init_0(r_4, w_6);
            var f_16 : FragmentContext_0 = rasterFragment_0(r_4, w_6, px_2, py_2, interpolateDepth_0(r_4.t_0, w_6));
            var s_16 : SurfaceSample_0;
            var depth_5 : f32;
            var _S92 : bool = fragmentDepth_0(source_3, &(f_16), &(s_16), &(depth_5));
            if(!_S92)
            {
                px_2 = px_2 + i32(1);
                continue;
            }
            var _S93 : u32 = atomicMin(&(depthBufI_0[u32(py_2) * params_0.width_1 + u32(px_2)]), depthToU32_0(depth_5));
            px_2 = px_2 + i32(1);
        }
        py_2 = py_2 + i32(1);
    }
    return;
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
    var r_5 : Triangle_0;
    var _S94 : bool = setupTriangle_0(triIdx_4, &(r_5));
    if(!_S94)
    {
        return;
    }
    var py_3 : i32 = i32(r_5.t_0.minY_0);
    for(;;)
    {
        if(py_3 <= i32(r_5.t_0.maxY_0))
        {
        }
        else
        {
            break;
        }
        var px_3 : i32 = i32(r_5.t_0.minX_0);
        for(;;)
        {
            if(px_3 <= i32(r_5.t_0.maxX_0))
            {
            }
            else
            {
                break;
            }
            var w_7 : vec3<f32> = barycentric_0(r_5.t_0, px_3, py_3);
            if(!covered_1(w_7))
            {
                px_3 = px_3 + i32(1);
                continue;
            }
            var source_4 : RasterSource_0 = RasterSource_x24init_0(r_5, w_7);
            var f_17 : FragmentContext_0 = rasterFragment_0(r_5, w_7, px_3, py_3, interpolateDepth_0(r_5.t_0, w_7));
            var s_17 : SurfaceSample_0;
            var depth_6 : f32;
            var _S95 : bool = fragmentDepth_0(source_4, &(f_17), &(s_17), &(depth_6));
            if(!_S95)
            {
                px_3 = px_3 + i32(1);
                continue;
            }
            var pixIdx_0 : u32 = u32(py_3) * params_0.width_1 + u32(px_3);
            var _S96 : u32 = depthToU32_0(depth_6);
            var _S97 : u32 = atomicLoad(&(depthBufI_0[pixIdx_0]));
            if(_S96 > (_S97 + u32(256)))
            {
                px_3 = px_3 + i32(1);
                continue;
            }
            var _S98 : bool = surfaceShade_0(source_4, &(f_17), s_17);
            if(!_S98)
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


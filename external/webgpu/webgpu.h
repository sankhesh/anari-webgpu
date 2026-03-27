// Minimal WebGPU header stub for compilation without a WebGPU implementation.
// This provides type definitions and function declarations matching the
// webgpu-native/webgpu-headers specification from
// https://github.com/webgpu-native/webgpu-headers
//
// When building with an actual WebGPU implementation (wgpu-native, Dawn, or
// Emscripten), this header is NOT used - the real header from the
// implementation is used instead.
//
// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: BSD-3-Clause

#ifndef WEBGPU_H_
#define WEBGPU_H_

#if defined(WGPU_SHARED_LIBRARY)
#    if defined(_WIN32)
#        if defined(WGPU_IMPLEMENTATION)
#            define WGPU_EXPORT __declspec(dllexport)
#        else
#            define WGPU_EXPORT __declspec(dllimport)
#        endif
#    else
#        if defined(WGPU_IMPLEMENTATION)
#            define WGPU_EXPORT __attribute__((visibility("default")))
#        else
#            define WGPU_EXPORT
#        endif
#    endif
#else
#    define WGPU_EXPORT
#endif

#include <stdint.h>
#include <stddef.h>

#define WGPU_DEPTH_SLICE_UNDEFINED (0xffffffffUL)

typedef uint32_t WGPUFlags;

// Opaque handle types
typedef struct WGPUAdapterImpl* WGPUAdapter;
typedef struct WGPUBindGroupImpl* WGPUBindGroup;
typedef struct WGPUBindGroupLayoutImpl* WGPUBindGroupLayout;
typedef struct WGPUBufferImpl* WGPUBuffer;
typedef struct WGPUCommandBufferImpl* WGPUCommandBuffer;
typedef struct WGPUCommandEncoderImpl* WGPUCommandEncoder;
typedef struct WGPUDeviceImpl* WGPUDevice;
typedef struct WGPUInstanceImpl* WGPUInstance;
typedef struct WGPUPipelineLayoutImpl* WGPUPipelineLayout;
typedef struct WGPUQuerySetImpl* WGPUQuerySet;
typedef struct WGPUQueueImpl* WGPUQueue;
typedef struct WGPURenderBundleImpl* WGPURenderBundle;
typedef struct WGPURenderBundleEncoderImpl* WGPURenderBundleEncoder;
typedef struct WGPURenderPassEncoderImpl* WGPURenderPassEncoder;
typedef struct WGPURenderPipelineImpl* WGPURenderPipeline;
typedef struct WGPUComputePipelineImpl* WGPUComputePipeline;
typedef struct WGPUSamplerImpl* WGPUSampler;
typedef struct WGPUShaderModuleImpl* WGPUShaderModule;
typedef struct WGPUSurfaceImpl* WGPUSurface;
typedef struct WGPUTextureImpl* WGPUTexture;
typedef struct WGPUTextureViewImpl* WGPUTextureView;

// Enums
typedef enum WGPURequestAdapterStatus {
    WGPURequestAdapterStatus_Success = 0x00000000,
    WGPURequestAdapterStatus_Unavailable = 0x00000001,
    WGPURequestAdapterStatus_Error = 0x00000002,
    WGPURequestAdapterStatus_Unknown = 0x00000003,
    WGPURequestAdapterStatus_Force32 = 0x7FFFFFFF
} WGPURequestAdapterStatus;

typedef enum WGPURequestDeviceStatus {
    WGPURequestDeviceStatus_Success = 0x00000000,
    WGPURequestDeviceStatus_Error = 0x00000001,
    WGPURequestDeviceStatus_Unknown = 0x00000002,
    WGPURequestDeviceStatus_Force32 = 0x7FFFFFFF
} WGPURequestDeviceStatus;

typedef enum WGPUBufferMapAsyncStatus {
    WGPUBufferMapAsyncStatus_Success = 0x00000000,
    WGPUBufferMapAsyncStatus_ValidationError = 0x00000001,
    WGPUBufferMapAsyncStatus_Unknown = 0x00000002,
    WGPUBufferMapAsyncStatus_DeviceLost = 0x00000003,
    WGPUBufferMapAsyncStatus_DestroyedBeforeCallback = 0x00000004,
    WGPUBufferMapAsyncStatus_UnmappedBeforeCallback = 0x00000005,
    WGPUBufferMapAsyncStatus_MappingAlreadyPending = 0x00000006,
    WGPUBufferMapAsyncStatus_OffsetOutOfRange = 0x00000007,
    WGPUBufferMapAsyncStatus_SizeOutOfRange = 0x00000008,
    WGPUBufferMapAsyncStatus_Force32 = 0x7FFFFFFF
} WGPUBufferMapAsyncStatus;

typedef enum WGPUErrorType {
    WGPUErrorType_NoError = 0x00000000,
    WGPUErrorType_Validation = 0x00000001,
    WGPUErrorType_OutOfMemory = 0x00000002,
    WGPUErrorType_Internal = 0x00000003,
    WGPUErrorType_Unknown = 0x00000004,
    WGPUErrorType_DeviceLost = 0x00000005,
    WGPUErrorType_Force32 = 0x7FFFFFFF
} WGPUErrorType;

typedef enum WGPUTextureFormat {
    WGPUTextureFormat_Undefined = 0x00000000,
    WGPUTextureFormat_R8Unorm = 0x00000001,
    WGPUTextureFormat_RGBA8Unorm = 0x00000012,
    WGPUTextureFormat_BGRA8Unorm = 0x00000017,
    WGPUTextureFormat_Depth24Plus = 0x00000028,
    WGPUTextureFormat_Depth32Float = 0x00000029,
    WGPUTextureFormat_Force32 = 0x7FFFFFFF
} WGPUTextureFormat;

typedef enum WGPUTextureDimension {
    WGPUTextureDimension_1D = 0x00000000,
    WGPUTextureDimension_2D = 0x00000001,
    WGPUTextureDimension_3D = 0x00000002,
    WGPUTextureDimension_Force32 = 0x7FFFFFFF
} WGPUTextureDimension;

typedef enum WGPUTextureViewDimension {
    WGPUTextureViewDimension_Undefined = 0x00000000,
    WGPUTextureViewDimension_1D = 0x00000001,
    WGPUTextureViewDimension_2D = 0x00000002,
    WGPUTextureViewDimension_2DArray = 0x00000003,
    WGPUTextureViewDimension_Cube = 0x00000004,
    WGPUTextureViewDimension_CubeArray = 0x00000005,
    WGPUTextureViewDimension_3D = 0x00000006,
    WGPUTextureViewDimension_Force32 = 0x7FFFFFFF
} WGPUTextureViewDimension;

typedef enum WGPUTextureAspect {
    WGPUTextureAspect_All = 0x00000000,
    WGPUTextureAspect_StencilOnly = 0x00000001,
    WGPUTextureAspect_DepthOnly = 0x00000002,
    WGPUTextureAspect_Force32 = 0x7FFFFFFF
} WGPUTextureAspect;

typedef enum WGPUTextureUsage {
    WGPUTextureUsage_None = 0x00000000,
    WGPUTextureUsage_CopySrc = 0x00000001,
    WGPUTextureUsage_CopyDst = 0x00000002,
    WGPUTextureUsage_TextureBinding = 0x00000004,
    WGPUTextureUsage_StorageBinding = 0x00000008,
    WGPUTextureUsage_RenderAttachment = 0x00000010,
    WGPUTextureUsage_Force32 = 0x7FFFFFFF
} WGPUTextureUsage;
typedef WGPUFlags WGPUTextureUsageFlags;

typedef enum WGPUBufferUsage {
    WGPUBufferUsage_None = 0x00000000,
    WGPUBufferUsage_MapRead = 0x00000001,
    WGPUBufferUsage_MapWrite = 0x00000002,
    WGPUBufferUsage_CopySrc = 0x00000004,
    WGPUBufferUsage_CopyDst = 0x00000008,
    WGPUBufferUsage_Index = 0x00000010,
    WGPUBufferUsage_Vertex = 0x00000020,
    WGPUBufferUsage_Uniform = 0x00000040,
    WGPUBufferUsage_Storage = 0x00000080,
    WGPUBufferUsage_Indirect = 0x00000100,
    WGPUBufferUsage_QueryResolve = 0x00000200,
    WGPUBufferUsage_Force32 = 0x7FFFFFFF
} WGPUBufferUsage;
typedef WGPUFlags WGPUBufferUsageFlags;

typedef enum WGPUMapMode {
    WGPUMapMode_None = 0x00000000,
    WGPUMapMode_Read = 0x00000001,
    WGPUMapMode_Write = 0x00000002,
    WGPUMapMode_Force32 = 0x7FFFFFFF
} WGPUMapMode;
typedef WGPUFlags WGPUMapModeFlags;

typedef enum WGPUVertexFormat {
    WGPUVertexFormat_Float32x2 = 0x00000011,
    WGPUVertexFormat_Float32x3 = 0x00000012,
    WGPUVertexFormat_Float32x4 = 0x00000013,
    WGPUVertexFormat_Force32 = 0x7FFFFFFF
} WGPUVertexFormat;

typedef enum WGPUVertexStepMode {
    WGPUVertexStepMode_Vertex = 0x00000000,
    WGPUVertexStepMode_Instance = 0x00000001,
    WGPUVertexStepMode_Force32 = 0x7FFFFFFF
} WGPUVertexStepMode;

typedef enum WGPUPrimitiveTopology {
    WGPUPrimitiveTopology_PointList = 0x00000000,
    WGPUPrimitiveTopology_LineList = 0x00000001,
    WGPUPrimitiveTopology_LineStrip = 0x00000002,
    WGPUPrimitiveTopology_TriangleList = 0x00000003,
    WGPUPrimitiveTopology_TriangleStrip = 0x00000004,
    WGPUPrimitiveTopology_Force32 = 0x7FFFFFFF
} WGPUPrimitiveTopology;

typedef enum WGPUIndexFormat {
    WGPUIndexFormat_Undefined = 0x00000000,
    WGPUIndexFormat_Uint16 = 0x00000001,
    WGPUIndexFormat_Uint32 = 0x00000002,
    WGPUIndexFormat_Force32 = 0x7FFFFFFF
} WGPUIndexFormat;

typedef enum WGPUFrontFace {
    WGPUFrontFace_CCW = 0x00000000,
    WGPUFrontFace_CW = 0x00000001,
    WGPUFrontFace_Force32 = 0x7FFFFFFF
} WGPUFrontFace;

typedef enum WGPUCullMode {
    WGPUCullMode_None = 0x00000000,
    WGPUCullMode_Front = 0x00000001,
    WGPUCullMode_Back = 0x00000002,
    WGPUCullMode_Force32 = 0x7FFFFFFF
} WGPUCullMode;

typedef enum WGPUCompareFunction {
    WGPUCompareFunction_Undefined = 0x00000000,
    WGPUCompareFunction_Never = 0x00000001,
    WGPUCompareFunction_Less = 0x00000002,
    WGPUCompareFunction_LessEqual = 0x00000003,
    WGPUCompareFunction_Greater = 0x00000004,
    WGPUCompareFunction_GreaterEqual = 0x00000005,
    WGPUCompareFunction_Equal = 0x00000006,
    WGPUCompareFunction_NotEqual = 0x00000007,
    WGPUCompareFunction_Always = 0x00000008,
    WGPUCompareFunction_Force32 = 0x7FFFFFFF
} WGPUCompareFunction;

typedef enum WGPUStencilOperation {
    WGPUStencilOperation_Keep = 0x00000000,
    WGPUStencilOperation_Zero = 0x00000001,
    WGPUStencilOperation_Replace = 0x00000002,
    WGPUStencilOperation_Invert = 0x00000003,
    WGPUStencilOperation_IncrementClamp = 0x00000004,
    WGPUStencilOperation_DecrementClamp = 0x00000005,
    WGPUStencilOperation_IncrementWrap = 0x00000006,
    WGPUStencilOperation_DecrementWrap = 0x00000007,
    WGPUStencilOperation_Force32 = 0x7FFFFFFF
} WGPUStencilOperation;

typedef enum WGPULoadOp {
    WGPULoadOp_Undefined = 0x00000000,
    WGPULoadOp_Clear = 0x00000001,
    WGPULoadOp_Load = 0x00000002,
    WGPULoadOp_Force32 = 0x7FFFFFFF
} WGPULoadOp;

typedef enum WGPUStoreOp {
    WGPUStoreOp_Undefined = 0x00000000,
    WGPUStoreOp_Store = 0x00000001,
    WGPUStoreOp_Discard = 0x00000002,
    WGPUStoreOp_Force32 = 0x7FFFFFFF
} WGPUStoreOp;

typedef enum WGPUBufferBindingType {
    WGPUBufferBindingType_Undefined = 0x00000000,
    WGPUBufferBindingType_Uniform = 0x00000001,
    WGPUBufferBindingType_Storage = 0x00000002,
    WGPUBufferBindingType_ReadOnlyStorage = 0x00000003,
    WGPUBufferBindingType_Force32 = 0x7FFFFFFF
} WGPUBufferBindingType;

typedef enum WGPUShaderStage {
    WGPUShaderStage_None = 0x00000000,
    WGPUShaderStage_Vertex = 0x00000001,
    WGPUShaderStage_Fragment = 0x00000002,
    WGPUShaderStage_Compute = 0x00000004,
    WGPUShaderStage_Force32 = 0x7FFFFFFF
} WGPUShaderStage;
typedef WGPUFlags WGPUShaderStageFlags;

typedef enum WGPUColorWriteMask {
    WGPUColorWriteMask_None = 0x00000000,
    WGPUColorWriteMask_Red = 0x00000001,
    WGPUColorWriteMask_Green = 0x00000002,
    WGPUColorWriteMask_Blue = 0x00000004,
    WGPUColorWriteMask_Alpha = 0x00000008,
    WGPUColorWriteMask_All = 0x0000000F,
    WGPUColorWriteMask_Force32 = 0x7FFFFFFF
} WGPUColorWriteMask;
typedef WGPUFlags WGPUColorWriteMaskFlags;

typedef enum WGPUPowerPreference {
    WGPUPowerPreference_Undefined = 0x00000000,
    WGPUPowerPreference_LowPower = 0x00000001,
    WGPUPowerPreference_HighPerformance = 0x00000002,
    WGPUPowerPreference_Force32 = 0x7FFFFFFF
} WGPUPowerPreference;

typedef enum WGPUSType {
    WGPUSType_Invalid = 0x00000000,
    WGPUSType_ShaderModuleWGSLDescriptor = 0x00000006,
    WGPUSType_Force32 = 0x7FFFFFFF
} WGPUSType;

// Callback types
typedef void (*WGPURequestAdapterCallback)(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const * message, void * userdata);
typedef void (*WGPURequestDeviceCallback)(WGPURequestDeviceStatus status, WGPUDevice device, char const * message, void * userdata);
typedef void (*WGPUBufferMapCallback)(WGPUBufferMapAsyncStatus status, void * userdata);
typedef void (*WGPUErrorCallback)(WGPUErrorType type, char const * message, void * userdata);

// Structs
typedef struct WGPUChainedStruct {
    struct WGPUChainedStruct const * next;
    WGPUSType sType;
} WGPUChainedStruct;

typedef struct WGPUChainedStructOut {
    struct WGPUChainedStructOut * next;
    WGPUSType sType;
} WGPUChainedStructOut;

typedef struct WGPUColor {
    double r;
    double g;
    double b;
    double a;
} WGPUColor;

typedef struct WGPUExtent3D {
    uint32_t width;
    uint32_t height;
    uint32_t depthOrArrayLayers;
} WGPUExtent3D;

typedef struct WGPUOrigin3D {
    uint32_t x;
    uint32_t y;
    uint32_t z;
} WGPUOrigin3D;

typedef struct WGPULimits {
    uint32_t maxTextureDimension1D;
    uint32_t maxTextureDimension2D;
    uint32_t maxTextureDimension3D;
    // ... additional limits omitted for brevity
} WGPULimits;

typedef struct WGPURequiredLimits {
    WGPUChainedStruct const * nextInChain;
    WGPULimits limits;
} WGPURequiredLimits;

typedef struct WGPUInstanceDescriptor {
    WGPUChainedStruct const * nextInChain;
} WGPUInstanceDescriptor;

typedef struct WGPURequestAdapterOptions {
    WGPUChainedStruct const * nextInChain;
    WGPUSurface compatibleSurface;
    WGPUPowerPreference powerPreference;
    int forceFallbackAdapter;  // WGPUBool
} WGPURequestAdapterOptions;

typedef struct WGPUQueueDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label;
} WGPUQueueDescriptor;

typedef struct WGPUDeviceDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label;
    size_t requiredFeatureCount;
    void const * requiredFeatures;
    WGPURequiredLimits const * requiredLimits;
    WGPUQueueDescriptor defaultQueue;
} WGPUDeviceDescriptor;

typedef struct WGPUTextureDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label;
    WGPUTextureUsageFlags usage;
    WGPUTextureDimension dimension;
    WGPUExtent3D size;
    WGPUTextureFormat format;
    uint32_t mipLevelCount;
    uint32_t sampleCount;
    size_t viewFormatCount;
    WGPUTextureFormat const * viewFormats;
} WGPUTextureDescriptor;

typedef struct WGPUTextureViewDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label;
    WGPUTextureFormat format;
    WGPUTextureViewDimension dimension;
    uint32_t baseMipLevel;
    uint32_t mipLevelCount;
    uint32_t baseArrayLayer;
    uint32_t arrayLayerCount;
    WGPUTextureAspect aspect;
} WGPUTextureViewDescriptor;

typedef struct WGPUBufferDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label;
    WGPUBufferUsageFlags usage;
    uint64_t size;
    int mappedAtCreation;  // WGPUBool
} WGPUBufferDescriptor;

typedef struct WGPUShaderModuleWGSLDescriptor {
    WGPUChainedStruct chain;
    char const * code;
} WGPUShaderModuleWGSLDescriptor;

typedef struct WGPUShaderModuleDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label;
} WGPUShaderModuleDescriptor;

typedef struct WGPUVertexAttribute {
    WGPUVertexFormat format;
    uint64_t offset;
    uint32_t shaderLocation;
} WGPUVertexAttribute;

typedef struct WGPUVertexBufferLayout {
    uint64_t arrayStride;
    WGPUVertexStepMode stepMode;
    size_t attributeCount;
    WGPUVertexAttribute const * attributes;
} WGPUVertexBufferLayout;

typedef struct WGPUConstantEntry {
    WGPUChainedStruct const * nextInChain;
    char const * key;
    double value;
} WGPUConstantEntry;

typedef struct WGPUVertexState {
    WGPUChainedStruct const * nextInChain;
    WGPUShaderModule module;
    char const * entryPoint;
    size_t constantCount;
    WGPUConstantEntry const * constants;
    size_t bufferCount;
    WGPUVertexBufferLayout const * buffers;
} WGPUVertexState;

typedef struct WGPUBlendComponent {
    int operation;
    int srcFactor;
    int dstFactor;
} WGPUBlendComponent;

typedef struct WGPUBlendState {
    WGPUBlendComponent color;
    WGPUBlendComponent alpha;
} WGPUBlendState;

typedef struct WGPUColorTargetState {
    WGPUChainedStruct const * nextInChain;
    WGPUTextureFormat format;
    WGPUBlendState const * blend;
    WGPUColorWriteMaskFlags writeMask;
} WGPUColorTargetState;

typedef struct WGPUFragmentState {
    WGPUChainedStruct const * nextInChain;
    WGPUShaderModule module;
    char const * entryPoint;
    size_t constantCount;
    WGPUConstantEntry const * constants;
    size_t targetCount;
    WGPUColorTargetState const * targets;
} WGPUFragmentState;

typedef struct WGPUStencilFaceState {
    WGPUCompareFunction compare;
    WGPUStencilOperation failOp;
    WGPUStencilOperation depthFailOp;
    WGPUStencilOperation passOp;
} WGPUStencilFaceState;

typedef struct WGPUDepthStencilState {
    WGPUChainedStruct const * nextInChain;
    WGPUTextureFormat format;
    int depthWriteEnabled;  // WGPUBool
    WGPUCompareFunction depthCompare;
    WGPUStencilFaceState stencilFront;
    WGPUStencilFaceState stencilBack;
    uint32_t stencilReadMask;
    uint32_t stencilWriteMask;
    int32_t depthBias;
    float depthBiasSlopeScale;
    float depthBiasClamp;
} WGPUDepthStencilState;

typedef struct WGPUPrimitiveState {
    WGPUChainedStruct const * nextInChain;
    WGPUPrimitiveTopology topology;
    WGPUIndexFormat stripIndexFormat;
    WGPUFrontFace frontFace;
    WGPUCullMode cullMode;
} WGPUPrimitiveState;

typedef struct WGPUMultisampleState {
    WGPUChainedStruct const * nextInChain;
    uint32_t count;
    uint32_t mask;
    int alphaToCoverageEnabled;  // WGPUBool
} WGPUMultisampleState;

typedef struct WGPURenderPipelineDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label;
    WGPUPipelineLayout layout;
    WGPUVertexState vertex;
    WGPUPrimitiveState primitive;
    WGPUDepthStencilState const * depthStencil;
    WGPUMultisampleState multisample;
    WGPUFragmentState const * fragment;
} WGPURenderPipelineDescriptor;

typedef struct WGPUBufferBindingLayout {
    WGPUChainedStruct const * nextInChain;
    WGPUBufferBindingType type;
    int hasDynamicOffset;  // WGPUBool
    uint64_t minBindingSize;
} WGPUBufferBindingLayout;

typedef struct WGPUSamplerBindingLayout {
    WGPUChainedStruct const * nextInChain;
    int type;
} WGPUSamplerBindingLayout;

typedef struct WGPUTextureBindingLayout {
    WGPUChainedStruct const * nextInChain;
    int sampleType;
    WGPUTextureViewDimension viewDimension;
    int multisampled;  // WGPUBool
} WGPUTextureBindingLayout;

typedef struct WGPUStorageTextureBindingLayout {
    WGPUChainedStruct const * nextInChain;
    int access;
    WGPUTextureFormat format;
    WGPUTextureViewDimension viewDimension;
} WGPUStorageTextureBindingLayout;

typedef struct WGPUBindGroupLayoutEntry {
    WGPUChainedStruct const * nextInChain;
    uint32_t binding;
    WGPUShaderStageFlags visibility;
    WGPUBufferBindingLayout buffer;
    WGPUSamplerBindingLayout sampler;
    WGPUTextureBindingLayout texture;
    WGPUStorageTextureBindingLayout storageTexture;
} WGPUBindGroupLayoutEntry;

typedef struct WGPUBindGroupLayoutDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label;
    size_t entryCount;
    WGPUBindGroupLayoutEntry const * entries;
} WGPUBindGroupLayoutDescriptor;

typedef struct WGPUPipelineLayoutDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label;
    size_t bindGroupLayoutCount;
    WGPUBindGroupLayout const * bindGroupLayouts;
} WGPUPipelineLayoutDescriptor;

typedef struct WGPUBindGroupEntry {
    WGPUChainedStruct const * nextInChain;
    uint32_t binding;
    WGPUBuffer buffer;
    uint64_t offset;
    uint64_t size;
    WGPUSampler sampler;
    WGPUTextureView textureView;
} WGPUBindGroupEntry;

typedef struct WGPUBindGroupDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label;
    WGPUBindGroupLayout layout;
    size_t entryCount;
    WGPUBindGroupEntry const * entries;
} WGPUBindGroupDescriptor;

typedef struct WGPUCommandEncoderDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label;
} WGPUCommandEncoderDescriptor;

typedef struct WGPUCommandBufferDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label;
} WGPUCommandBufferDescriptor;

typedef struct WGPURenderPassTimestampWrites {
    WGPUQuerySet querySet;
    uint32_t beginningOfPassWriteIndex;
    uint32_t endOfPassWriteIndex;
} WGPURenderPassTimestampWrites;

typedef struct WGPURenderPassColorAttachment {
    WGPUChainedStruct const * nextInChain;
    WGPUTextureView view;
    uint32_t depthSlice;
    WGPUTextureView resolveTarget;
    WGPULoadOp loadOp;
    WGPUStoreOp storeOp;
    WGPUColor clearValue;
} WGPURenderPassColorAttachment;

typedef struct WGPURenderPassDepthStencilAttachment {
    WGPUTextureView view;
    WGPULoadOp depthLoadOp;
    WGPUStoreOp depthStoreOp;
    float depthClearValue;
    int depthReadOnly;  // WGPUBool
    WGPULoadOp stencilLoadOp;
    WGPUStoreOp stencilStoreOp;
    uint32_t stencilClearValue;
    int stencilReadOnly;  // WGPUBool
} WGPURenderPassDepthStencilAttachment;

typedef struct WGPURenderPassDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label;
    size_t colorAttachmentCount;
    WGPURenderPassColorAttachment const * colorAttachments;
    WGPURenderPassDepthStencilAttachment const * depthStencilAttachment;
    WGPUQuerySet occlusionQuerySet;
    WGPURenderPassTimestampWrites const * timestampWrites;
} WGPURenderPassDescriptor;

typedef struct WGPUTextureDataLayout {
    WGPUChainedStruct const * nextInChain;
    uint64_t offset;
    uint32_t bytesPerRow;
    uint32_t rowsPerImage;
} WGPUTextureDataLayout;

typedef struct WGPUImageCopyTexture {
    WGPUChainedStruct const * nextInChain;
    WGPUTexture texture;
    uint32_t mipLevel;
    WGPUOrigin3D origin;
    WGPUTextureAspect aspect;
} WGPUImageCopyTexture;

typedef struct WGPUImageCopyBuffer {
    WGPUChainedStruct const * nextInChain;
    WGPUTextureDataLayout layout;
    WGPUBuffer buffer;
} WGPUImageCopyBuffer;

#ifdef __cplusplus
extern "C" {
#endif

// Instance
WGPU_EXPORT WGPUInstance wgpuCreateInstance(WGPUInstanceDescriptor const * descriptor);
WGPU_EXPORT void wgpuInstanceRelease(WGPUInstance instance);
WGPU_EXPORT void wgpuInstanceRequestAdapter(WGPUInstance instance, WGPURequestAdapterOptions const * options, WGPURequestAdapterCallback callback, void * userdata);
WGPU_EXPORT void wgpuInstanceProcessEvents(WGPUInstance instance);

// Adapter
WGPU_EXPORT void wgpuAdapterRelease(WGPUAdapter adapter);
WGPU_EXPORT void wgpuAdapterRequestDevice(WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor, WGPURequestDeviceCallback callback, void * userdata);

// Device
WGPU_EXPORT void wgpuDeviceRelease(WGPUDevice device);
WGPU_EXPORT WGPUQueue wgpuDeviceGetQueue(WGPUDevice device);
WGPU_EXPORT WGPUTexture wgpuDeviceCreateTexture(WGPUDevice device, WGPUTextureDescriptor const * descriptor);
WGPU_EXPORT WGPUBuffer wgpuDeviceCreateBuffer(WGPUDevice device, WGPUBufferDescriptor const * descriptor);
WGPU_EXPORT WGPUShaderModule wgpuDeviceCreateShaderModule(WGPUDevice device, WGPUShaderModuleDescriptor const * descriptor);
WGPU_EXPORT WGPURenderPipeline wgpuDeviceCreateRenderPipeline(WGPUDevice device, WGPURenderPipelineDescriptor const * descriptor);
WGPU_EXPORT WGPUBindGroupLayout wgpuDeviceCreateBindGroupLayout(WGPUDevice device, WGPUBindGroupLayoutDescriptor const * descriptor);
WGPU_EXPORT WGPUPipelineLayout wgpuDeviceCreatePipelineLayout(WGPUDevice device, WGPUPipelineLayoutDescriptor const * descriptor);
WGPU_EXPORT WGPUBindGroup wgpuDeviceCreateBindGroup(WGPUDevice device, WGPUBindGroupDescriptor const * descriptor);
WGPU_EXPORT WGPUCommandEncoder wgpuDeviceCreateCommandEncoder(WGPUDevice device, WGPUCommandEncoderDescriptor const * descriptor);
WGPU_EXPORT void wgpuDeviceSetUncapturedErrorCallback(WGPUDevice device, WGPUErrorCallback callback, void * userdata);
WGPU_EXPORT void wgpuDeviceTick(WGPUDevice device);

// Queue
WGPU_EXPORT void wgpuQueueRelease(WGPUQueue queue);
WGPU_EXPORT void wgpuQueueSubmit(WGPUQueue queue, size_t commandCount, WGPUCommandBuffer const * commands);
WGPU_EXPORT void wgpuQueueWriteBuffer(WGPUQueue queue, WGPUBuffer buffer, uint64_t bufferOffset, void const * data, size_t size);

// Texture
WGPU_EXPORT void wgpuTextureRelease(WGPUTexture texture);
WGPU_EXPORT WGPUTextureView wgpuTextureCreateView(WGPUTexture texture, WGPUTextureViewDescriptor const * descriptor);

// TextureView
WGPU_EXPORT void wgpuTextureViewRelease(WGPUTextureView textureView);

// Buffer
WGPU_EXPORT void wgpuBufferRelease(WGPUBuffer buffer);
WGPU_EXPORT void wgpuBufferMapAsync(WGPUBuffer buffer, WGPUMapModeFlags mode, size_t offset, size_t size, WGPUBufferMapCallback callback, void * userdata);
WGPU_EXPORT void const * wgpuBufferGetConstMappedRange(WGPUBuffer buffer, size_t offset, size_t size);
WGPU_EXPORT void wgpuBufferUnmap(WGPUBuffer buffer);

// ShaderModule
WGPU_EXPORT void wgpuShaderModuleRelease(WGPUShaderModule shaderModule);

// RenderPipeline
WGPU_EXPORT void wgpuRenderPipelineRelease(WGPURenderPipeline renderPipeline);
WGPU_EXPORT WGPUBindGroupLayout wgpuRenderPipelineGetBindGroupLayout(WGPURenderPipeline renderPipeline, uint32_t groupIndex);

// BindGroupLayout
WGPU_EXPORT void wgpuBindGroupLayoutRelease(WGPUBindGroupLayout bindGroupLayout);

// BindGroup
WGPU_EXPORT void wgpuBindGroupRelease(WGPUBindGroup bindGroup);

// PipelineLayout
WGPU_EXPORT void wgpuPipelineLayoutRelease(WGPUPipelineLayout pipelineLayout);

// CommandEncoder
WGPU_EXPORT void wgpuCommandEncoderRelease(WGPUCommandEncoder commandEncoder);
WGPU_EXPORT WGPURenderPassEncoder wgpuCommandEncoderBeginRenderPass(WGPUCommandEncoder commandEncoder, WGPURenderPassDescriptor const * descriptor);
WGPU_EXPORT void wgpuCommandEncoderCopyTextureToBuffer(WGPUCommandEncoder commandEncoder, WGPUImageCopyTexture const * source, WGPUImageCopyBuffer const * destination, WGPUExtent3D const * copySize);
WGPU_EXPORT WGPUCommandBuffer wgpuCommandEncoderFinish(WGPUCommandEncoder commandEncoder, WGPUCommandBufferDescriptor const * descriptor);

// CommandBuffer
WGPU_EXPORT void wgpuCommandBufferRelease(WGPUCommandBuffer commandBuffer);

// RenderPassEncoder
WGPU_EXPORT void wgpuRenderPassEncoderRelease(WGPURenderPassEncoder renderPassEncoder);
WGPU_EXPORT void wgpuRenderPassEncoderSetPipeline(WGPURenderPassEncoder renderPassEncoder, WGPURenderPipeline pipeline);
WGPU_EXPORT void wgpuRenderPassEncoderSetBindGroup(WGPURenderPassEncoder renderPassEncoder, uint32_t groupIndex, WGPUBindGroup group, size_t dynamicOffsetCount, uint32_t const * dynamicOffsets);
WGPU_EXPORT void wgpuRenderPassEncoderSetVertexBuffer(WGPURenderPassEncoder renderPassEncoder, uint32_t slot, WGPUBuffer buffer, uint64_t offset, uint64_t size);
WGPU_EXPORT void wgpuRenderPassEncoderDraw(WGPURenderPassEncoder renderPassEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
WGPU_EXPORT void wgpuRenderPassEncoderEnd(WGPURenderPassEncoder renderPassEncoder);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // WEBGPU_H_

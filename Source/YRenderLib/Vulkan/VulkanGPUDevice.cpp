#include "YBaseLib/Assert.h"
#include "YRenderLib/Vulkan/VulkanImports.h"
#include "YRenderLib/Vulkan/VulkanGPUDevice.h"

VulkanGPUDevice::VulkanGPUDevice(VkInstance vkInstance, VkDevice vkDevice)
    : m_vkInstance(vkInstance)
    , m_vkDevice(vkDevice)
{

}

VulkanGPUDevice::~VulkanGPUDevice()
{
    vkDestroyDevice(m_vkDevice, nullptr);
    vkDestroyInstance(m_vkInstance, nullptr);
}

RENDERER_PLATFORM VulkanGPUDevice::GetPlatform() const
{
    return RENDERER_PLATFORM_VULKAN;
}

RENDERER_FEATURE_LEVEL VulkanGPUDevice::GetFeatureLevel() const
{
    Panic("The method or operation is not implemented.");
    return RENDERER_FEATURE_LEVEL_SM5;
}

TEXTURE_PLATFORM VulkanGPUDevice::GetTexturePlatform() const
{
    Panic("The method or operation is not implemented.");
    return TEXTURE_PLATFORM_DXTC;
}

SHADER_PROGRAM_BYTECODE_TYPE VulkanGPUDevice::GetShaderProgramType() const
{
    Panic("The method or operation is not implemented.");
    return SHADER_PROGRAM_BYTECODE_TYPE_GLSL_ES_300;
}

void VulkanGPUDevice::GetCapabilities(RendererCapabilities* pCapabilities) const
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUDevice::GetCounters(RendererCounters* pCounters) const
{
    Panic("The method or operation is not implemented.");
}

bool VulkanGPUDevice::CheckTexturePixelFormatCompatibility(PIXEL_FORMAT PixelFormat, PIXEL_FORMAT* CompatibleFormat /*= nullptr*/) const
{
    Panic("The method or operation is not implemented.");
    return false;
}

void VulkanGPUDevice::CorrectProjectionMatrix(float* projectionMatrix) const
{
    Panic("The method or operation is not implemented.");
}

float VulkanGPUDevice::GetTexelOffset() const
{
    Panic("The method or operation is not implemented.");
    return 0.0f;
}

GPUOutputBuffer* VulkanGPUDevice::CreateOutputBuffer(RenderSystemWindowHandle hWnd, RENDERER_VSYNC_TYPE vsyncType)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUOutputBuffer* VulkanGPUDevice::CreateOutputBuffer(SDL_Window* pSDLWindow, RENDERER_VSYNC_TYPE vsyncType)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUDepthStencilState* VulkanGPUDevice::CreateDepthStencilState(const RENDERER_DEPTHSTENCIL_STATE_DESC* pDepthStencilStateDesc)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPURasterizerState* VulkanGPUDevice::CreateRasterizerState(const RENDERER_RASTERIZER_STATE_DESC* pRasterizerStateDesc)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUBlendState* VulkanGPUDevice::CreateBlendState(const RENDERER_BLEND_STATE_DESC* pBlendStateDesc)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUQuery* VulkanGPUDevice::CreateQuery(GPU_QUERY_TYPE type)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUBuffer* VulkanGPUDevice::CreateBuffer(const GPU_BUFFER_DESC* pDesc, const void* pInitialData /*= NULL*/)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUTexture1D* VulkanGPUDevice::CreateTexture1D(const GPU_TEXTURE1D_DESC* pTextureDesc, const GPU_SAMPLER_STATE_DESC* pSamplerStateDesc, const void* *ppInitialData /*= NULL*/, const uint32* pInitialDataPitch /*= NULL*/)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUTexture1DArray* VulkanGPUDevice::CreateTexture1DArray(const GPU_TEXTURE1DARRAY_DESC* pTextureDesc, const GPU_SAMPLER_STATE_DESC* pSamplerStateDesc, const void* *ppInitialData /*= NULL*/, const uint32* pInitialDataPitch /*= NULL*/)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUTexture2D* VulkanGPUDevice::CreateTexture2D(const GPU_TEXTURE2D_DESC* pTextureDesc, const GPU_SAMPLER_STATE_DESC* pSamplerStateDesc, const void* *ppInitialData /*= NULL*/, const uint32* pInitialDataPitch /*= NULL*/)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUTexture2DArray* VulkanGPUDevice::CreateTexture2DArray(const GPU_TEXTURE2DARRAY_DESC* pTextureDesc, const GPU_SAMPLER_STATE_DESC* pSamplerStateDesc, const void* *ppInitialData /*= NULL*/, const uint32* pInitialDataPitch /*= NULL*/)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUTexture3D* VulkanGPUDevice::CreateTexture3D(const GPU_TEXTURE3D_DESC* pTextureDesc, const GPU_SAMPLER_STATE_DESC* pSamplerStateDesc, const void* *ppInitialData /*= NULL*/, const uint32* pInitialDataPitch /*= NULL*/, const uint32* pInitialDataSlicePitch /*= NULL*/)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUTextureCube* VulkanGPUDevice::CreateTextureCube(const GPU_TEXTURECUBE_DESC* pTextureDesc, const GPU_SAMPLER_STATE_DESC* pSamplerStateDesc, const void* *ppInitialData /*= NULL*/, const uint32* pInitialDataPitch /*= NULL*/)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUTextureCubeArray* VulkanGPUDevice::CreateTextureCubeArray(const GPU_TEXTURECUBEARRAY_DESC* pTextureDesc, const GPU_SAMPLER_STATE_DESC* pSamplerStateDesc, const void* *ppInitialData /*= NULL*/, const uint32* pInitialDataPitch /*= NULL*/)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUDepthTexture* VulkanGPUDevice::CreateDepthTexture(const GPU_DEPTH_TEXTURE_DESC* pTextureDesc)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUSamplerState* VulkanGPUDevice::CreateSamplerState(const GPU_SAMPLER_STATE_DESC* pSamplerStateDesc)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPURenderTargetView* VulkanGPUDevice::CreateRenderTargetView(GPUTexture* pTexture, const GPU_RENDER_TARGET_VIEW_DESC* pDesc)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUDepthStencilBufferView* VulkanGPUDevice::CreateDepthStencilBufferView(GPUTexture* pTexture, const GPU_DEPTH_STENCIL_BUFFER_VIEW_DESC* pDesc)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUInputLayout* VulkanGPUDevice::CreateInputLayout(const GPU_VERTEX_ELEMENT_DESC*pElements, uint32 nElements)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUShaderProgram* VulkanGPUDevice::CreateGraphicsProgram(ByteStream* pByteCodeStream)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

GPUShaderProgram* VulkanGPUDevice::CreateComputeProgram(ByteStream* pByteCodeStream)
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

void VulkanGPUDevice::BeginResourceBatchUpload()
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUDevice::EndResourceBatchUpload()
{
    Panic("The method or operation is not implemented.");
}


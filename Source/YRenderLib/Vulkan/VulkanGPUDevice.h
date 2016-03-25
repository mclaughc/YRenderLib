#include "YRenderLib/Renderer.h"
#include "YRenderLib/Vulkan/VulkanCommon.h"
#include "YRenderLib/Vulkan/VulkanDefines.h"
#include "YRenderLib/Vulkan/VulkanImports.h"

class VulkanGPUDevice : public GPUDevice
{
public:
    VulkanGPUDevice(VkInstance vkInstance, VkDevice vkDevice);
    ~VulkanGPUDevice();

    VkInstance GetVkInstance() const { return m_vkInstance; }
    VkDevice GetVkDevice() const { return m_vkDevice; }

    virtual RENDERER_PLATFORM GetPlatform() const override;
    virtual RENDERER_FEATURE_LEVEL GetFeatureLevel() const override;
    virtual TEXTURE_PLATFORM GetTexturePlatform() const override;
    virtual SHADER_PROGRAM_BYTECODE_TYPE GetShaderProgramType() const override;

    virtual void GetCapabilities(RendererCapabilities* pCapabilities) const override;
    virtual void GetCounters(RendererCounters* pCounters) const override;
    virtual bool CheckTexturePixelFormatCompatibility(PIXEL_FORMAT PixelFormat, PIXEL_FORMAT* CompatibleFormat = nullptr) const override;
    virtual void CorrectProjectionMatrix(float* projectionMatrix) const override;
    virtual float GetTexelOffset() const override;

    virtual GPUOutputBuffer* CreateOutputBuffer(RenderSystemWindowHandle hWnd, RENDERER_VSYNC_TYPE vsyncType) override;
    virtual GPUOutputBuffer* CreateOutputBuffer(SDL_Window* pSDLWindow, RENDERER_VSYNC_TYPE vsyncType) override;
    virtual GPUDepthStencilState* CreateDepthStencilState(const RENDERER_DEPTHSTENCIL_STATE_DESC* pDepthStencilStateDesc) override;
    virtual GPURasterizerState* CreateRasterizerState(const RENDERER_RASTERIZER_STATE_DESC* pRasterizerStateDesc) override;
    virtual GPUBlendState* CreateBlendState(const RENDERER_BLEND_STATE_DESC* pBlendStateDesc) override;
    virtual GPUQuery* CreateQuery(GPU_QUERY_TYPE type) override;
    virtual GPUBuffer* CreateBuffer(const GPU_BUFFER_DESC* pDesc, const void* pInitialData = NULL) override;
    virtual GPUTexture1D* CreateTexture1D(const GPU_TEXTURE1D_DESC* pTextureDesc, const GPU_SAMPLER_STATE_DESC* pSamplerStateDesc, const void* *ppInitialData = NULL, const uint32* pInitialDataPitch = NULL) override;
    virtual GPUTexture1DArray* CreateTexture1DArray(const GPU_TEXTURE1DARRAY_DESC* pTextureDesc, const GPU_SAMPLER_STATE_DESC* pSamplerStateDesc, const void* *ppInitialData = NULL, const uint32* pInitialDataPitch = NULL) override;
    virtual GPUTexture2D* CreateTexture2D(const GPU_TEXTURE2D_DESC* pTextureDesc, const GPU_SAMPLER_STATE_DESC* pSamplerStateDesc, const void* *ppInitialData = NULL, const uint32* pInitialDataPitch = NULL) override;
    virtual GPUTexture2DArray* CreateTexture2DArray(const GPU_TEXTURE2DARRAY_DESC* pTextureDesc, const GPU_SAMPLER_STATE_DESC* pSamplerStateDesc, const void* *ppInitialData = NULL, const uint32* pInitialDataPitch = NULL) override;
    virtual GPUTexture3D* CreateTexture3D(const GPU_TEXTURE3D_DESC* pTextureDesc, const GPU_SAMPLER_STATE_DESC* pSamplerStateDesc, const void* *ppInitialData = NULL, const uint32* pInitialDataPitch = NULL, const uint32* pInitialDataSlicePitch = NULL) override;
    virtual GPUTextureCube* CreateTextureCube(const GPU_TEXTURECUBE_DESC* pTextureDesc, const GPU_SAMPLER_STATE_DESC* pSamplerStateDesc, const void* *ppInitialData = NULL, const uint32* pInitialDataPitch = NULL) override;
    virtual GPUTextureCubeArray* CreateTextureCubeArray(const GPU_TEXTURECUBEARRAY_DESC* pTextureDesc, const GPU_SAMPLER_STATE_DESC* pSamplerStateDesc, const void* *ppInitialData = NULL, const uint32* pInitialDataPitch = NULL) override;
    virtual GPUDepthTexture* CreateDepthTexture(const GPU_DEPTH_TEXTURE_DESC* pTextureDesc) override;
    virtual GPUSamplerState* CreateSamplerState(const GPU_SAMPLER_STATE_DESC* pSamplerStateDesc) override;
    virtual GPURenderTargetView* CreateRenderTargetView(GPUTexture* pTexture, const GPU_RENDER_TARGET_VIEW_DESC* pDesc) override;
    virtual GPUDepthStencilBufferView* CreateDepthStencilBufferView(GPUTexture* pTexture, const GPU_DEPTH_STENCIL_BUFFER_VIEW_DESC* pDesc) override;
    virtual GPUInputLayout*CreateInputLayout(const GPU_VERTEX_ELEMENT_DESC*pElements, uint32 nElements) override;
    virtual GPUShaderProgram* CreateGraphicsProgram(ByteStream* pByteCodeStream) override;
    virtual GPUShaderProgram* CreateComputeProgram(ByteStream* pByteCodeStream) override;

    virtual void BeginResourceBatchUpload() override;
    virtual void EndResourceBatchUpload() override;

private:
    VkInstance m_vkInstance;
    VkDevice m_vkDevice;
};
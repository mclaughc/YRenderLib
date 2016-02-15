#pragma once
#include "YRenderLib/D3D11/D3D11Common.h"
#include "YRenderLib/D3D11/D3D11GPUContext.h"
#include "YRenderLib/Renderer.h"

class D3D11GPUSamplerState : public GPUSamplerState
{
public:
    D3D11GPUSamplerState(const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc, ID3D11SamplerState *pD3DSamplerState);
    virtual ~D3D11GPUSamplerState();

    virtual void GetMemoryUsage(uint32 *cpuMemoryUsage, uint32 *gpuMemoryUsage) const override;
    virtual void SetDebugName(const char *name) override;

    ID3D11SamplerState *GetD3DSamplerState() { return m_pD3DSamplerState; }

private:
    ID3D11SamplerState *m_pD3DSamplerState;
};

class D3D11GPURasterizerState : public GPURasterizerState
{
public:
    D3D11GPURasterizerState(const RENDERER_RASTERIZER_STATE_DESC *pRasterizerStateDesc, ID3D11RasterizerState *pD3DRasterizerState);
    virtual ~D3D11GPURasterizerState();

    virtual void GetMemoryUsage(uint32 *cpuMemoryUsage, uint32 *gpuMemoryUsage) const override;
    virtual void SetDebugName(const char *name) override;

    ID3D11RasterizerState *GetD3DRasterizerState() { return m_pD3DRasterizerState; }

private:
    ID3D11RasterizerState *m_pD3DRasterizerState;
};

class D3D11GPUDepthStencilState : public GPUDepthStencilState
{
public:
    D3D11GPUDepthStencilState(const RENDERER_DEPTHSTENCIL_STATE_DESC *pDepthStencilStateDesc, ID3D11DepthStencilState *pD3DDepthStencilState);
    virtual ~D3D11GPUDepthStencilState();

    virtual void GetMemoryUsage(uint32 *cpuMemoryUsage, uint32 *gpuMemoryUsage) const override;
    virtual void SetDebugName(const char *name) override;

    ID3D11DepthStencilState *GetD3DDepthStencilState() { return m_pD3DDepthStencilState; }

private:
    ID3D11DepthStencilState *m_pD3DDepthStencilState;
};

class D3D11GPUBlendState : public GPUBlendState
{
public:
    D3D11GPUBlendState(const RENDERER_BLEND_STATE_DESC *pBlendStateDesc, ID3D11BlendState *pD3DBlendState);
    virtual ~D3D11GPUBlendState();

    virtual void GetMemoryUsage(uint32 *cpuMemoryUsage, uint32 *gpuMemoryUsage) const override;
    virtual void SetDebugName(const char *name) override;

    ID3D11BlendState *GetD3DBlendState() { return m_pD3DBlendState; }

private:
    ID3D11BlendState *m_pD3DBlendState;
};

class D3D11GPUDevice : public GPUDevice
{
public:
    D3D11GPUDevice(IDXGIFactory *pDXGIFactory, IDXGIAdapter *pDXGIAdapter, ID3D11Device *pD3DDevice, ID3D11Device1 *pD3DDevice1,
                   D3D_FEATURE_LEVEL D3DFeatureLevel, RENDERER_FEATURE_LEVEL featureLevel, TEXTURE_PLATFORM texturePlatform,
                   SHADER_PROGRAM_BYTECODE_TYPE shaderProgramType, DXGI_FORMAT windowBackBufferFormat, DXGI_FORMAT windowDepthStencilFormat);

    virtual ~D3D11GPUDevice();

    // private methods
    IDXGIFactory *GetDXGIFactory() const { return m_pDXGIFactory; }
    IDXGIAdapter *GetDXGIAdapter() const { return m_pDXGIAdapter; }
    ID3D11Device *GetD3DDevice() const { return m_pD3DDevice; }
    ID3D11Device1 *GetD3DDevice1() const { return m_pD3DDevice1; }
    DXGI_FORMAT GetSwapChainBackBufferFormat() const { return m_swapChainBackBufferFormat; }
    DXGI_FORMAT GetSwapChainDepthStencilBufferFormat() const { return m_swapChainDepthStencilBufferFormat; }
    RendererCounters* GetCounters() { return &m_counters; }
    ID3D11RasterizerState* GetDefaultRasterizerState() const { return m_pDefaultRasterizerState; }
    ID3D11DepthStencilState* GetDefaultDepthStencilState() const { return m_pDefaultDepthStencilState; }
    ID3D11BlendState* GetDefaultBlendState() const { return m_pDefaultBlendState; }

    // create default states, etc.
    bool Create();

    // Device queries.
    virtual RENDERER_PLATFORM GetPlatform() const override final;
    virtual RENDERER_FEATURE_LEVEL GetFeatureLevel() const override final;
    virtual TEXTURE_PLATFORM GetTexturePlatform() const override final;
    virtual SHADER_PROGRAM_BYTECODE_TYPE GetShaderProgramType() const override final;
    virtual void GetCounters(RendererCounters* pCounters) const override final;
    virtual void GetCapabilities(RendererCapabilities *pCapabilities) const override final;
    virtual bool CheckTexturePixelFormatCompatibility(PIXEL_FORMAT PixelFormat, PIXEL_FORMAT *CompatibleFormat = nullptr) const override final;
    virtual void CorrectProjectionMatrix(float *projectionMatrix) const override final;
    virtual float GetTexelOffset() const override final;

    // Creates a swap chain on an existing window.
    virtual GPUOutputBuffer *CreateOutputBuffer(RenderSystemWindowHandle hWnd, RENDERER_VSYNC_TYPE vsyncType) override final;
    virtual GPUOutputBuffer *CreateOutputBuffer(SDL_Window *pSDLWindow, RENDERER_VSYNC_TYPE vsyncType) override final;

    // Resource creation
    virtual GPUQuery *CreateQuery(GPU_QUERY_TYPE type) override final;
    virtual GPUBuffer *CreateBuffer(const GPU_BUFFER_DESC *pDesc, const void *pInitialData = nullptr) override final;
    virtual GPUTexture1D *CreateTexture1D(const GPU_TEXTURE1D_DESC *pTextureDesc, const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc, const void **ppInitialData = nullptr, const uint32 *pInitialDataPitch = nullptr) override final;
    virtual GPUTexture1DArray *CreateTexture1DArray(const GPU_TEXTURE1DARRAY_DESC *pTextureDesc, const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc, const void **ppInitialData = nullptr, const uint32 *pInitialDataPitch = nullptr) override final;
    virtual GPUTexture2D *CreateTexture2D(const GPU_TEXTURE2D_DESC *pTextureDesc, const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc, const void **ppInitialData = nullptr, const uint32 *pInitialDataPitch = nullptr) override final;
    virtual GPUTexture2DArray *CreateTexture2DArray(const GPU_TEXTURE2DARRAY_DESC *pTextureDesc, const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc, const void **ppInitialData = nullptr, const uint32 *pInitialDataPitch = nullptr) override final;
    virtual GPUTexture3D *CreateTexture3D(const GPU_TEXTURE3D_DESC *pTextureDesc, const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc, const void **ppInitialData = nullptr, const uint32 *pInitialDataPitch = nullptr, const uint32 *pInitialDataSlicePitch = nullptr) override final;
    virtual GPUTextureCube *CreateTextureCube(const GPU_TEXTURECUBE_DESC *pTextureDesc, const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc, const void **ppInitialData = nullptr, const uint32 *pInitialDataPitch = nullptr) override final;
    virtual GPUTextureCubeArray *CreateTextureCubeArray(const GPU_TEXTURECUBEARRAY_DESC *pTextureDesc, const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc, const void **ppInitialData = nullptr, const uint32 *pInitialDataPitch = nullptr) override final;
    virtual GPUDepthTexture *CreateDepthTexture(const GPU_DEPTH_TEXTURE_DESC *pTextureDesc) override final;
    virtual GPUSamplerState *CreateSamplerState(const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc) override final;
    virtual GPURenderTargetView *CreateRenderTargetView(GPUTexture *pTexture, const GPU_RENDER_TARGET_VIEW_DESC *pDesc) override final;
    virtual GPUDepthStencilBufferView *CreateDepthStencilBufferView(GPUTexture *pTexture, const GPU_DEPTH_STENCIL_BUFFER_VIEW_DESC *pDesc) override final;
    virtual GPUDepthStencilState *CreateDepthStencilState(const RENDERER_DEPTHSTENCIL_STATE_DESC *pDepthStencilStateDesc) override final;
    virtual GPURasterizerState *CreateRasterizerState(const RENDERER_RASTERIZER_STATE_DESC *pRasterizerStateDesc) override final;
    virtual GPUBlendState *CreateBlendState(const RENDERER_BLEND_STATE_DESC *pBlendStateDesc) override final;
    virtual GPUInputLayout* CreateInputLayout(const GPU_VERTEX_ELEMENT_DESC* pElements, uint32 nElements) override final;
    virtual GPUShaderProgram *CreateGraphicsProgram(ByteStream *pByteCodeStream) override final;
    virtual GPUShaderProgram *CreateComputeProgram(ByteStream *pByteCodeStream) override final;

    // off-thread resource creation
    virtual void BeginResourceBatchUpload() override final;
    virtual void EndResourceBatchUpload() override final;

private:
    IDXGIFactory *m_pDXGIFactory;
    IDXGIAdapter *m_pDXGIAdapter;

    ID3D11Device *m_pD3DDevice;
    ID3D11Device1 *m_pD3DDevice1;

    D3D_FEATURE_LEVEL m_D3DFeatureLevel;
    RENDERER_FEATURE_LEVEL m_featureLevel;
    TEXTURE_PLATFORM m_texturePlatform;
    SHADER_PROGRAM_BYTECODE_TYPE m_shaderProgramType;

    DXGI_FORMAT m_swapChainBackBufferFormat;
    DXGI_FORMAT m_swapChainDepthStencilBufferFormat;

    RendererCounters m_counters;

    ID3D11RasterizerState* m_pDefaultRasterizerState;
    ID3D11DepthStencilState* m_pDefaultDepthStencilState;
    ID3D11BlendState* m_pDefaultBlendState;
};

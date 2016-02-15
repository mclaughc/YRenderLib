#include "YBaseLib/Log.h"
#include "YRenderLib/D3D11/D3D11Defines.h"
#include "YRenderLib/D3D11/D3D11GPUBuffer.h"
#include "YRenderLib/D3D11/D3D11GPUContext.h"
#include "YRenderLib/D3D11/D3D11GPUDevice.h"
#include "YRenderLib/D3D11/D3D11GPUQuery.h"
#include "YRenderLib/D3D11/D3D11GPUOutputBuffer.h"
#include "YRenderLib/D3D11/D3D11GPUShaderProgram.h"
#include "YRenderLib/D3D11/D3D11GPUTexture.h"
Log_SetChannel(D3D11GPUDevice);

D3D11GPUDevice::D3D11GPUDevice(IDXGIFactory *pDXGIFactory, IDXGIAdapter *pDXGIAdapter, ID3D11Device *pD3DDevice, ID3D11Device1 *pD3DDevice1, D3D_FEATURE_LEVEL D3DFeatureLevel, RENDERER_FEATURE_LEVEL featureLevel, TEXTURE_PLATFORM texturePlatform, SHADER_PROGRAM_BYTECODE_TYPE shaderProgramType, DXGI_FORMAT windowBackBufferFormat, DXGI_FORMAT windowDepthStencilFormat)
    : m_pDXGIFactory(pDXGIFactory)
    , m_pDXGIAdapter(pDXGIAdapter)
    , m_pD3DDevice(pD3DDevice)
    , m_pD3DDevice1(pD3DDevice1)
    , m_D3DFeatureLevel(D3DFeatureLevel)
    , m_featureLevel(featureLevel)
    , m_texturePlatform(texturePlatform)
    , m_shaderProgramType(shaderProgramType)
    , m_swapChainBackBufferFormat(windowBackBufferFormat)
    , m_swapChainDepthStencilBufferFormat(windowDepthStencilFormat)
{
    m_pDXGIFactory->AddRef();
    m_pDXGIAdapter->AddRef();
    m_pD3DDevice->AddRef();
}

D3D11GPUDevice::~D3D11GPUDevice()
{
    SAFE_RELEASE(m_pDefaultRasterizerState);
    SAFE_RELEASE(m_pDefaultDepthStencilState);
    SAFE_RELEASE(m_pDefaultBlendState);

    SAFE_RELEASE(m_pD3DDevice);
    SAFE_RELEASE(m_pDXGIAdapter);
    SAFE_RELEASE(m_pDXGIFactory);

#ifdef Y_BUILD_CONFIG_DEBUG
    // dump remaining objects
    {
        HMODULE hDXGIDebugModule = GetModuleHandleA("dxgidebug.dll");
        if (hDXGIDebugModule != NULL)
        {
            HRESULT(WINAPI *pDXGIGetDebugInterface)(REFIID riid, void **ppDebug);
            pDXGIGetDebugInterface = (HRESULT(WINAPI *)(REFIID, void **))GetProcAddress(hDXGIDebugModule, "DXGIGetDebugInterface");
            if (pDXGIGetDebugInterface != NULL)
            {
                IDXGIDebug *pDXGIDebug;
                if (SUCCEEDED(pDXGIGetDebugInterface(__uuidof(pDXGIDebug), reinterpret_cast<void **>(&pDXGIDebug))))
                {
                    Log_DevPrint("=== Begin remaining DXGI and D3D object dump ===");
                    pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
                    Log_DevPrint("=== End remaining DXGI and D3D object dump ===");
                    pDXGIDebug->Release();
                }
            }
        }
    }
#endif
}

bool D3D11GPUDevice::Create()
{
    RENDERER_RASTERIZER_STATE_DESC defaultRasterizerStateDesc;
    defaultRasterizerStateDesc.SetDefault();
    m_pDefaultRasterizerState = D3D11Helpers::CreateD3D11RasterizerState(m_pD3DDevice, &defaultRasterizerStateDesc);
    if (m_pDefaultRasterizerState == nullptr)
        return false;

    RENDERER_DEPTHSTENCIL_STATE_DESC defaultDepthStencilStateDesc;
    defaultDepthStencilStateDesc.SetDefault();
    m_pDefaultDepthStencilState = D3D11Helpers::CreateD3D11DepthStencilState(m_pD3DDevice, &defaultDepthStencilStateDesc);
    if (m_pDefaultDepthStencilState == nullptr)
        return false;

    RENDERER_BLEND_STATE_DESC defaultBlendStateDesc;
    defaultBlendStateDesc.SetDefault();
    m_pDefaultBlendState = D3D11Helpers::CreateD3D11BlendState(m_pD3DDevice, &defaultBlendStateDesc);
    if (m_pDefaultBlendState == nullptr)
        return false;

    return true;
}

RENDERER_PLATFORM D3D11GPUDevice::GetPlatform() const
{
    return RENDERER_PLATFORM_D3D11;
}

RENDERER_FEATURE_LEVEL D3D11GPUDevice::GetFeatureLevel() const
{
    return m_featureLevel;
}

TEXTURE_PLATFORM D3D11GPUDevice::GetTexturePlatform() const
{
    return m_texturePlatform;
}

SHADER_PROGRAM_BYTECODE_TYPE D3D11GPUDevice::GetShaderProgramType() const
{
    return m_shaderProgramType;
}

void D3D11GPUDevice::GetCounters(RendererCounters* pCounters) const
{
    memcpy(pCounters, &m_counters, sizeof(m_counters));
}

void D3D11GPUDevice::GetCapabilities(RendererCapabilities *pCapabilities) const
{
    pCapabilities->MaxTextureAnisotropy = D3D11_MAX_MAXANISOTROPY;
    pCapabilities->MaximumVertexBuffers = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
    pCapabilities->MaximumConstantBuffers = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
    pCapabilities->MaximumTextureUnits = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
    pCapabilities->MaximumSamplers = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
    pCapabilities->MaximumRenderTargets = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
    pCapabilities->SupportsCommandLists = false;
    pCapabilities->SupportsMultithreadedResourceCreation = true;
    pCapabilities->SupportsDrawBaseVertex = true;
    pCapabilities->SupportsDepthTextures = true;
    pCapabilities->SupportsTextureArrays = (m_featureLevel >= D3D_FEATURE_LEVEL_10_0);
    pCapabilities->SupportsCubeMapTextureArrays = (m_featureLevel >= D3D_FEATURE_LEVEL_10_1);
    pCapabilities->SupportsGeometryShaders = (m_featureLevel >= D3D_FEATURE_LEVEL_10_0);
    pCapabilities->SupportsSinglePassCubeMaps = (m_featureLevel >= D3D_FEATURE_LEVEL_10_0);
    pCapabilities->SupportsInstancing = (m_featureLevel >= D3D_FEATURE_LEVEL_10_0);
}

bool D3D11GPUDevice::CheckTexturePixelFormatCompatibility(PIXEL_FORMAT PixelFormat, PIXEL_FORMAT *CompatibleFormat /*= NULL*/) const
{
    DXGI_FORMAT DXGIFormat = D3D11TypeConversion::PixelFormatToDXGIFormat(PixelFormat);
    if (DXGIFormat == DXGI_FORMAT_UNKNOWN)
    {
        // use r8g8b8a8
        if (CompatibleFormat != NULL)
            *CompatibleFormat = PIXEL_FORMAT_R8G8B8A8_UNORM;

        return false;
    }

    if (CompatibleFormat != NULL)
        *CompatibleFormat = PixelFormat;

    return true;
}

void D3D11GPUDevice::CorrectProjectionMatrix(float *projectionMatrix) const
{

}

float D3D11GPUDevice::GetTexelOffset() const
{
    return 0.0f;
}

void D3D11GPUDevice::BeginResourceBatchUpload()
{

}

void D3D11GPUDevice::EndResourceBatchUpload()
{

}

D3D11GPUSamplerState::D3D11GPUSamplerState(const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc, ID3D11SamplerState *pD3DSamplerState)
    : GPUSamplerState(pSamplerStateDesc), m_pD3DSamplerState(pD3DSamplerState)
{

}

D3D11GPUSamplerState::~D3D11GPUSamplerState()
{
    m_pD3DSamplerState->Release();
}

void D3D11GPUSamplerState::GetMemoryUsage(uint32 *cpuMemoryUsage, uint32 *gpuMemoryUsage) const
{
    if (cpuMemoryUsage != nullptr)
        *cpuMemoryUsage = sizeof(*this) + sizeof(ID3D11SamplerState);

    // approximation
    if (gpuMemoryUsage != nullptr)
        *gpuMemoryUsage = 128;
}

void D3D11GPUSamplerState::SetDebugName(const char *name)
{
    D3D11Helpers::SetD3D11DeviceChildDebugName(m_pD3DSamplerState, name);
}

GPUSamplerState *D3D11GPUDevice::CreateSamplerState(const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc)
{
    ID3D11SamplerState *pD3DSamplerState = D3D11Helpers::CreateD3D11SamplerState(m_pD3DDevice, pSamplerStateDesc);
    if (pD3DSamplerState == NULL)
        return NULL;

    D3D11GPUSamplerState *pSamplerState = new D3D11GPUSamplerState(pSamplerStateDesc, pD3DSamplerState);
    return pSamplerState;
}

D3D11GPURasterizerState::D3D11GPURasterizerState(const RENDERER_RASTERIZER_STATE_DESC *pRasterizerStateDesc, ID3D11RasterizerState *pD3DRasterizerState)
    : GPURasterizerState(pRasterizerStateDesc), m_pD3DRasterizerState(pD3DRasterizerState)
{

}

D3D11GPURasterizerState::~D3D11GPURasterizerState()
{
    m_pD3DRasterizerState->Release();
}

void D3D11GPURasterizerState::GetMemoryUsage(uint32 *cpuMemoryUsage, uint32 *gpuMemoryUsage) const
{
    if (cpuMemoryUsage != nullptr)
        *cpuMemoryUsage = sizeof(*this) + sizeof(ID3D11RasterizerState);

    // approximation
    if (gpuMemoryUsage != nullptr)
        *gpuMemoryUsage = 128;
}

void D3D11GPURasterizerState::SetDebugName(const char *name)
{
    D3D11Helpers::SetD3D11DeviceChildDebugName(m_pD3DRasterizerState, name);
}

GPURasterizerState *D3D11GPUDevice::CreateRasterizerState(const RENDERER_RASTERIZER_STATE_DESC *pRasterizerStateDesc)
{
    ID3D11RasterizerState *pD3DRasterizerState = D3D11Helpers::CreateD3D11RasterizerState(m_pD3DDevice, pRasterizerStateDesc);
    if (pD3DRasterizerState == NULL)
        return NULL;

    D3D11GPURasterizerState *pRasterizerState = new D3D11GPURasterizerState(pRasterizerStateDesc, pD3DRasterizerState);
    return pRasterizerState;
}

D3D11GPUDepthStencilState::D3D11GPUDepthStencilState(const RENDERER_DEPTHSTENCIL_STATE_DESC *pDepthStencilStateDesc, ID3D11DepthStencilState *pD3DDepthStencilState)
    : GPUDepthStencilState(pDepthStencilStateDesc), m_pD3DDepthStencilState(pD3DDepthStencilState)
{

}

D3D11GPUDepthStencilState::~D3D11GPUDepthStencilState()
{
    m_pD3DDepthStencilState->Release();
}

void D3D11GPUDepthStencilState::GetMemoryUsage(uint32 *cpuMemoryUsage, uint32 *gpuMemoryUsage) const
{
    if (cpuMemoryUsage != nullptr)
        *cpuMemoryUsage = sizeof(*this) + sizeof(ID3D11DepthStencilState);

    // approximation
    if (gpuMemoryUsage != nullptr)
        *gpuMemoryUsage = 128;
}

void D3D11GPUDepthStencilState::SetDebugName(const char *name)
{
    D3D11Helpers::SetD3D11DeviceChildDebugName(m_pD3DDepthStencilState, name);
}

GPUDepthStencilState *D3D11GPUDevice::CreateDepthStencilState(const RENDERER_DEPTHSTENCIL_STATE_DESC *pDepthStencilStateDesc)
{
    ID3D11DepthStencilState *pD3DDepthStencilState = D3D11Helpers::CreateD3D11DepthStencilState(m_pD3DDevice, pDepthStencilStateDesc);
    if (pD3DDepthStencilState == NULL)
        return NULL;

    D3D11GPUDepthStencilState *pDepthStencilState = new D3D11GPUDepthStencilState(pDepthStencilStateDesc, pD3DDepthStencilState);
    return pDepthStencilState;
}

D3D11GPUBlendState::D3D11GPUBlendState(const RENDERER_BLEND_STATE_DESC *pBlendStateDesc, ID3D11BlendState *pD3DBlendState)
    : GPUBlendState(pBlendStateDesc), m_pD3DBlendState(pD3DBlendState)
{

}

D3D11GPUBlendState::~D3D11GPUBlendState()
{
    m_pD3DBlendState->Release();
}

void D3D11GPUBlendState::GetMemoryUsage(uint32 *cpuMemoryUsage, uint32 *gpuMemoryUsage) const
{
    if (cpuMemoryUsage != nullptr)
        *cpuMemoryUsage = sizeof(*this) + sizeof(ID3D11BlendState);

    // approximation
    if (gpuMemoryUsage != nullptr)
        *gpuMemoryUsage = 128;
}

void D3D11GPUBlendState::SetDebugName(const char *name)
{
    D3D11Helpers::SetD3D11DeviceChildDebugName(m_pD3DBlendState, name);
}

GPUBlendState *D3D11GPUDevice::CreateBlendState(const RENDERER_BLEND_STATE_DESC *pBlendStateDesc)
{
    ID3D11BlendState *pD3DBlendState = D3D11Helpers::CreateD3D11BlendState(m_pD3DDevice, pBlendStateDesc);
    if (pD3DBlendState == NULL)
        return NULL;

    D3D11GPUBlendState *pBlendState = new D3D11GPUBlendState(pBlendStateDesc, pD3DBlendState);
    return pBlendState;
}


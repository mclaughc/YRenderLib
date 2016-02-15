#include "YBaseLib/Log.h"
#include "YRenderLib/D3D11/D3D11Defines.h"
#include "YRenderLib/D3D11/D3D11GPUBuffer.h"
#include "YRenderLib/D3D11/D3D11GPUContext.h"
#include "YRenderLib/D3D11/D3D11GPUDevice.h"
#include "YRenderLib/D3D11/D3D11GPUInputLayout.h"
#include "YRenderLib/D3D11/D3D11GPUQuery.h"
#include "YRenderLib/D3D11/D3D11GPUOutputBuffer.h"
#include "YRenderLib/D3D11/D3D11GPUShaderProgram.h"
#include "YRenderLib/D3D11/D3D11GPUTexture.h"
#include "YRenderLib/Util.h"
Log_SetChannel(D3D11GPUContext);

D3D11GPUContext::D3D11GPUContext(D3D11GPUDevice *pDevice, ID3D11Device *pD3DDevice, ID3D11Device1 *pD3DDevice1, ID3D11DeviceContext *pImmediateContext)
    : m_pDevice(pDevice)
    , m_pD3DDevice(pD3DDevice)
    , m_pD3DDevice1(pD3DDevice1)
    , m_pD3DContext(pImmediateContext)
    , m_pD3DContext1(nullptr)
{
    // add references
    m_pDevice->AddRef();
    m_pD3DDevice->AddRef();
    m_pD3DContext->AddRef();

    // null memory
    Y_memzero(&m_currentViewport, sizeof(m_currentViewport));
    Y_memzero(&m_scissorRect, sizeof(m_scissorRect));
    m_currentTopology = DRAW_TOPOLOGY_UNDEFINED;

    // null current states
    Y_memzero(m_pCurrentVertexBuffers, sizeof(m_pCurrentVertexBuffers));
    Y_memzero(m_currentVertexBufferOffsets, sizeof(m_currentVertexBufferOffsets));
    Y_memzero(m_currentVertexBufferStrides, sizeof(m_currentVertexBufferStrides));
    m_currentVertexBufferBindCount = 0;

    m_pCurrentIndexBuffer = nullptr;
    m_currentIndexFormat = GPU_INDEX_FORMAT_COUNT;
    m_currentIndexBufferOffset = 0;

    m_pCurrentShaderProgram = nullptr;
    Y_memzero(&m_shaderBindings, sizeof(m_shaderBindings));
    m_shaderBindings.FirstDirtyConstantBuffer = m_shaderBindings.LastDirtyConstantBuffer = -1;
    m_shaderBindings.FirstDirtySampler = m_shaderBindings.LastDirtySampler = -1;
    m_shaderBindings.FirstDirtyResource = m_shaderBindings.LastDirtyResource = -1;
    m_shaderBindings.FirstDirtyRWResource = m_shaderBindings.LastDirtyRWResource = -1;

    m_pCurrentRasterizerState = nullptr;
    m_pCurrentDepthStencilState = nullptr;
    m_currentDepthStencilRef = 0;
    m_pCurrentBlendState = nullptr;
    m_currentBlendStateBlendFactors.SetZero();

    m_pCurrentSwapChain = nullptr;

    Y_memzero(m_pCurrentRenderTargetViews, sizeof(m_pCurrentRenderTargetViews));
    m_pCurrentDepthBufferView = nullptr;
    m_nCurrentRenderTargets = 0;

    m_pCurrentPredicate = nullptr;
    m_pCurrentPredicateD3D = nullptr;
    m_predicateBypassCount = 0;

    m_pUserVertexBuffer = nullptr;
    m_userVertexBufferSize = 16 * 1024 * 1024;  // 16MB
    m_userVertexBufferPosition = 0;

    Y_memzero(m_shaderStates, sizeof(m_shaderStates));
    for (uint32 i = 0; i < SHADER_PROGRAM_STAGE_COUNT; i++)
    {
        ShaderStageState *state = &m_shaderStates[i];
        state->ConstantBufferDirtyLowerBounds = state->ConstantBufferDirtyUpperBounds = -1;
        state->ResourceDirtyLowerBounds = state->ResourceDirtyUpperBounds = -1;
        state->SamplerDirtyLowerBounds = state->SamplerDirtyUpperBounds = -1;
        state->UAVDirtyLowerBounds = state->UAVDirtyUpperBounds = -1;
    }
}

D3D11GPUContext::~D3D11GPUContext()
{
    // clear any state
    ClearState(true, true, false, true);
    m_pD3DContext->RSSetState(nullptr);
    m_pD3DContext->OMSetDepthStencilState(nullptr, 0);
    m_pD3DContext->OMSetBlendState(nullptr, nullptr, 0);
    m_pD3DContext->OMSetRenderTargets(0, nullptr, nullptr);
    m_pD3DContext->ClearState();
    m_pD3DContext->Flush();

    DebugAssert(m_predicateBypassCount == 0);
    if (m_pCurrentPredicateD3D != nullptr)
    {
        m_pD3DContext->SetPredication(nullptr, FALSE);
        m_pCurrentPredicateD3D = nullptr;
    }
    SAFE_RELEASE(m_pCurrentPredicate);

    // release our references to states and targets. d3d references will die when the device goes down.
    for (uint32 i = 0; i < countof(m_pCurrentVertexBuffers); i++) {
        SAFE_RELEASE(m_pCurrentVertexBuffers[i]);
    }
    SAFE_RELEASE(m_pCurrentIndexBuffer);

    SAFE_RELEASE(m_pCurrentInputLayout);
    SAFE_RELEASE(m_pCurrentShaderProgram);
    for (uint32 stage = 0; stage < SHADER_PROGRAM_STAGE_COUNT; stage++)
    {
        ShaderStageState &state = m_shaderStates[stage];

        for (uint32 i = 0; i < state.ConstantBufferBindCount; i++) {
            SAFE_RELEASE(state.ConstantBuffers[i]);
        }

        for (uint32 i = 0; i < state.ResourceBindCount; i++) {
            SAFE_RELEASE(state.Resources[i]);
        }

        for (uint32 i = 0; i < state.SamplerBindCount; i++) {
            SAFE_RELEASE(state.Samplers[i]);
        }

        for (uint32 i = 0; i < state.UAVBindCount; i++) {
            SAFE_RELEASE(state.UAVs[i]);
        }
    }

    for (uint32 i = 0; i < m_shaderBindings.ConstantBuffers.GetSize(); i++)
        SAFE_RELEASE(m_shaderBindings.ConstantBuffers[i]);

    for (uint32 i = 0; i < m_shaderBindings.Samplers.GetSize(); i++)
        SAFE_RELEASE(m_shaderBindings.Samplers[i]);

    for (uint32 i = 0; i < m_shaderBindings.Resources.GetSize(); i++)
        SAFE_RELEASE(m_shaderBindings.Resources[i]);

    for (uint32 i = 0; i < m_shaderBindings.RWResources.GetSize(); i++)
        SAFE_RELEASE(m_shaderBindings.RWResources[i]);

    SAFE_RELEASE(m_pCurrentRasterizerState);
    SAFE_RELEASE(m_pCurrentDepthStencilState);
    SAFE_RELEASE(m_pCurrentBlendState);

    for (uint32 i = 0; i < countof(m_pCurrentRenderTargetViews); i++) {
        SAFE_RELEASE(m_pCurrentRenderTargetViews[i]);
    }
    SAFE_RELEASE(m_pCurrentDepthBufferView);

    SAFE_RELEASE(m_pUserVertexBuffer);

    // clear swapchain last, like gl
    SAFE_RELEASE(m_pCurrentSwapChain);
    m_pD3DContext->Release();
    m_pD3DDevice->Release();
    m_pDevice->Release();
}

bool D3D11GPUContext::Create()
{
    HRESULT hResult;

    // set initial state
    m_pD3DContext->RSSetState(m_pDevice->GetDefaultRasterizerState());
    m_pD3DContext->OMSetDepthStencilState(m_pDevice->GetDefaultDepthStencilState(), 0);
    m_pD3DContext->OMSetBlendState(m_pDevice->GetDefaultBlendState(), nullptr, 0xFFFFFFFF);

    // create user buffers
    {
        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;

        // vertex buffer
        bufferDesc.ByteWidth = m_userVertexBufferSize;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        if (FAILED(hResult = m_pD3DDevice->CreateBuffer(&bufferDesc, NULL, &m_pUserVertexBuffer)))
        {
            Log_ErrorPrintf("Failed to create user vertex buffer %08X", hResult);
            return false;
        }
    }

    // initialize binding sizes
    m_shaderBindings.ConstantBuffers.Resize(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
    m_shaderBindings.Samplers.Resize(D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);
    m_shaderBindings.Resources.Resize(D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
    m_shaderBindings.RWResources.Resize(D3D11_PS_CS_UAV_REGISTER_COUNT);
    return true;
}

void D3D11GPUContext::ClearState(bool clearShaders /* = true */, bool clearBuffers /* = true */, bool clearStates /* = true */, bool clearRenderTargets /* = true */)
{
    if (clearShaders)
    {
        SetInputLayout(nullptr);
        SetShaderProgram(nullptr);

        for (uint32 i = 0; i < m_shaderBindings.ConstantBuffers.GetSize(); i++)
            SAFE_RELEASE(m_shaderBindings.ConstantBuffers[i]);
        m_shaderBindings.FirstDirtyConstantBuffer = 0;
        m_shaderBindings.LastDirtyConstantBuffer = m_shaderBindings.ConstantBuffers.GetSize() - 1;

        for (uint32 i = 0; i < m_shaderBindings.Samplers.GetSize(); i++)
            SAFE_RELEASE(m_shaderBindings.Samplers[i]);
        m_shaderBindings.FirstDirtySampler = 0;
        m_shaderBindings.LastDirtySampler = m_shaderBindings.Samplers.GetSize() - 1;

        for (uint32 i = 0; i < m_shaderBindings.Resources.GetSize(); i++)
            SAFE_RELEASE(m_shaderBindings.Resources[i]);
        m_shaderBindings.FirstDirtyResource = 0;
        m_shaderBindings.LastDirtyResource = m_shaderBindings.Resources.GetSize() - 1;

        for (uint32 i = 0; i < m_shaderBindings.RWResources.GetSize(); i++)
            SAFE_RELEASE(m_shaderBindings.RWResources[i]);
        m_shaderBindings.FirstDirtyRWResource = 0;
        m_shaderBindings.LastDirtyRWResource = m_shaderBindings.RWResources.GetSize() - 1;

        for (uint32 stage = 0; stage < SHADER_PROGRAM_STAGE_COUNT; stage++)
        {
            ShaderStageState &state = m_shaderStates[stage];

            if (state.ConstantBufferBindCount > 0)
            {
                for (uint32 i = 0; i < state.ConstantBufferBindCount; i++) {
                    SAFE_RELEASE(state.ConstantBuffers[i]);
                }
                state.ConstantBufferDirtyLowerBounds = 0;
                state.ConstantBufferDirtyUpperBounds = state.ConstantBufferBindCount - 1;
            }

            if (state.ResourceBindCount > 0)
            {
                for (uint32 i = 0; i < state.ResourceBindCount; i++) {
                    SAFE_RELEASE(state.Resources[i]);
                }
                state.ResourceDirtyLowerBounds = 0;
                state.ResourceDirtyUpperBounds = state.ResourceBindCount - 1;
            }

            if (state.SamplerBindCount > 0)
            {
                for (uint32 i = 0; i < state.SamplerBindCount; i++) {
                    SAFE_RELEASE(state.Samplers[i]);
                }
                state.SamplerDirtyLowerBounds = 0;
                state.SamplerDirtyUpperBounds = state.SamplerBindCount - 1;
            }

            if (state.UAVBindCount > 0)
            {
                for (uint32 i = 0; i < state.UAVBindCount; i++) {
                    SAFE_RELEASE(state.UAVs[i]);
                }
                state.UAVDirtyLowerBounds = 0;
                state.UAVDirtyUpperBounds = state.UAVBindCount - 1;
            }
        }

        SynchronizeShaderBindings();
    }

    if (clearBuffers)
    {
        if (m_currentVertexBufferBindCount > 0)
        {
            static GPUBuffer *nullVertexBuffers[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { nullptr };
            static const uint32 nullSizeOrOffset[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
            SetVertexBuffers(0, m_currentVertexBufferBindCount, nullVertexBuffers, nullSizeOrOffset, nullSizeOrOffset);
        }

        if (m_pCurrentIndexBuffer != nullptr)
            SetIndexBuffer(nullptr, GPU_INDEX_FORMAT_UINT16, 0);
    }

    if (clearStates)
    {
        SetRasterizerState(nullptr);
        SetDepthStencilState(nullptr, 0);
        SetBlendState(nullptr);
        SetDrawTopology(DRAW_TOPOLOGY_UNDEFINED);

        RENDERER_SCISSOR_RECT scissor(0, 0, 0, 0);
        SetFullViewport(nullptr);
        SetScissorRect(&scissor);
    }

    if (clearRenderTargets)
    {
        SetRenderTargets(0, nullptr, nullptr);
    }
}

GPURasterizerState *D3D11GPUContext::GetRasterizerState()
{
    return m_pCurrentRasterizerState;
}

void D3D11GPUContext::SetRasterizerState(GPURasterizerState *pRasterizerState)
{
    if (m_pCurrentRasterizerState != pRasterizerState)
    {
        if (m_pCurrentRasterizerState)
            m_pCurrentRasterizerState->Release();

        m_pCurrentRasterizerState = static_cast<D3D11GPURasterizerState *>(pRasterizerState);
        if (m_pCurrentRasterizerState)
        {
            m_pCurrentRasterizerState->AddRef();
            m_pD3DContext->RSSetState(m_pCurrentRasterizerState->GetD3DRasterizerState());
        }
        else
        {
            m_pD3DContext->RSSetState(m_pDevice->GetDefaultRasterizerState());
        }
    }
}

GPUDepthStencilState *D3D11GPUContext::GetDepthStencilState()
{
    return m_pCurrentDepthStencilState;
}

uint8 D3D11GPUContext::GetDepthStencilStateStencilRef()
{
    return m_currentDepthStencilRef;
}

void D3D11GPUContext::SetDepthStencilState(GPUDepthStencilState *pDepthStencilState, uint8 stencilRef)
{
    if (m_pCurrentDepthStencilState != pDepthStencilState || m_currentDepthStencilRef != stencilRef)
    {
        if (m_pCurrentDepthStencilState == pDepthStencilState)
        {
            // just changing ref value
            m_pD3DContext->OMSetDepthStencilState(m_pCurrentDepthStencilState->GetD3DDepthStencilState(), stencilRef);
        }
        else
        {
            if (m_pCurrentDepthStencilState)
                m_pCurrentDepthStencilState->Release();

            m_pCurrentDepthStencilState = static_cast<D3D11GPUDepthStencilState *>(pDepthStencilState);
            if (m_pCurrentDepthStencilState)
            {
                m_pCurrentDepthStencilState->AddRef();
                m_pD3DContext->OMSetDepthStencilState(m_pCurrentDepthStencilState->GetD3DDepthStencilState(), stencilRef);
            }
            else
            {
                m_pD3DContext->OMSetDepthStencilState(m_pDevice->GetDefaultDepthStencilState(), stencilRef);
            }
        }

        m_currentDepthStencilRef = stencilRef;
    }
}

GPUBlendState *D3D11GPUContext::GetBlendState()
{
    return m_pCurrentBlendState;
}

const FloatColor &D3D11GPUContext::GetBlendStateBlendFactor()
{
    return m_currentBlendStateBlendFactors;
};

void D3D11GPUContext::SetBlendState(GPUBlendState *pBlendState, const FloatColor &blendFactor /* = FloatColor::White */)
{
    if (m_pCurrentBlendState != pBlendState || blendFactor != m_currentBlendStateBlendFactors)
    {
        if (m_pCurrentBlendState)
            m_pCurrentBlendState->Release();

        m_pCurrentBlendState = static_cast<D3D11GPUBlendState *>(pBlendState);
        if (m_pCurrentBlendState)
        {
            m_pCurrentBlendState->AddRef();
            m_pD3DContext->OMSetBlendState(m_pCurrentBlendState->GetD3DBlendState(), blendFactor, 0xFFFFFFFF);
        }
        else
        {
            m_pD3DContext->OMSetBlendState(m_pDevice->GetDefaultBlendState(), blendFactor, 0xFFFFFFFF);
        }

        m_currentBlendStateBlendFactors = blendFactor;
    }
}

const RENDERER_VIEWPORT *D3D11GPUContext::GetViewport()
{
    return &m_currentViewport;
}

void D3D11GPUContext::SetViewport(const RENDERER_VIEWPORT *pNewViewport)
{
    if (Y_memcmp(&m_currentViewport, pNewViewport, sizeof(RENDERER_VIEWPORT)) == 0)
        return;

    Y_memcpy(&m_currentViewport, pNewViewport, sizeof(m_currentViewport));

    D3D11_VIEWPORT D3DViewport;
    D3DViewport.TopLeftX = (float)m_currentViewport.TopLeftX;
    D3DViewport.TopLeftY = (float)m_currentViewport.TopLeftY;
    D3DViewport.Width = (float)m_currentViewport.Width;
    D3DViewport.Height = (float)m_currentViewport.Height;
    D3DViewport.MinDepth = pNewViewport->MinDepth;
    D3DViewport.MaxDepth = pNewViewport->MaxDepth;
    m_pD3DContext->RSSetViewports(1, &D3DViewport);
}

void D3D11GPUContext::SetFullViewport(GPUTexture *pForRenderTarget /* = NULL */)
{
    RENDERER_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;

    if (pForRenderTarget == nullptr && m_nCurrentRenderTargets == 0 && m_pCurrentDepthBufferView == nullptr)
    {
        if (m_pCurrentSwapChain != nullptr)
        {
            viewport.Width = m_pCurrentSwapChain->GetWidth();
            viewport.Height = m_pCurrentSwapChain->GetHeight();
        }
        else
        {
            viewport.Width = 1;
            viewport.Height = 1;
        }        
    }
    else
    {
        DebugAssert(m_pCurrentRenderTargetViews[0] != nullptr || pForRenderTarget != nullptr);

        GPUTexture *pRT = pForRenderTarget;
        if (pRT != nullptr || m_nCurrentRenderTargets > 0)
        {
            Util::GetTextureDimensions((pRT != nullptr) ? pRT : m_pCurrentRenderTargetViews[0]->GetTargetTexture(), &viewport.Width, &viewport.Height, nullptr);
        }
        else
        {
            viewport.Width = m_pCurrentSwapChain->GetWidth();
            viewport.Height = m_pCurrentSwapChain->GetHeight();
        }
    }

    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    SetViewport(&viewport);
}

const RENDERER_SCISSOR_RECT *D3D11GPUContext::GetScissorRect()
{
    return &m_scissorRect;
}

void D3D11GPUContext::SetScissorRect(const RENDERER_SCISSOR_RECT *pScissorRect)
{
    if (Y_memcmp(&m_scissorRect, pScissorRect, sizeof(m_scissorRect)) == 0)
        return;

    Y_memcpy(&m_scissorRect, pScissorRect, sizeof(m_scissorRect));

    D3D11_RECT D3DRect;
    D3DRect.left = pScissorRect->Left;
    D3DRect.top = pScissorRect->Top;
    D3DRect.right = pScissorRect->Right;
    D3DRect.bottom = pScissorRect->Bottom;
    m_pD3DContext->RSSetScissorRects(1, &D3DRect);
}

void D3D11GPUContext::ClearTargets(bool clearColor /* = true */, bool clearDepth /* = true */, bool clearStencil /* = true */, const FloatColor &clearColorValue /* = FloatColor::Black */, float clearDepthValue /* = 1.0f */, uint8 clearStencilValue /* = 0 */)
{
    uint32 clearFlags = 0;
    if (clearDepth)
        clearFlags |= D3D11_CLEAR_DEPTH;
    if (clearStencil)
        clearFlags |= D3D11_CLEAR_STENCIL;

    if (m_nCurrentRenderTargets == 0 && m_pCurrentDepthBufferView == nullptr)
    {
        // on swapchain
        if (m_pCurrentSwapChain != nullptr)
        {
            ID3D11RenderTargetView *pRTV = m_pCurrentSwapChain->GetRenderTargetView();
            ID3D11DepthStencilView *pDSV = m_pCurrentSwapChain->GetDepthStencilView();

            if (clearColor)
                m_pD3DContext->ClearRenderTargetView(pRTV, clearColorValue);

            if (clearFlags != 0 && pDSV != nullptr)
                m_pD3DContext->ClearDepthStencilView(pDSV, clearFlags, clearDepthValue, clearStencilValue);
        }
    }
    else
    {
        if (clearColor)
        {
            for (uint32 i = 0; i < m_nCurrentRenderTargets; i++)
            {
                if (m_pCurrentRenderTargetViews[i] != nullptr)
                    m_pD3DContext->ClearRenderTargetView(m_pCurrentRenderTargetViews[i]->GetD3DRTV(), clearColorValue);
            }
        }

        if (clearFlags != 0 && m_pCurrentDepthBufferView != nullptr)
            m_pD3DContext->ClearDepthStencilView(m_pCurrentDepthBufferView->GetD3DDSV(), clearFlags, clearDepthValue, clearStencilValue);
    }
}

void D3D11GPUContext::DiscardTargets(bool discardColor /* = true */, bool discardDepth /* = true */, bool discardStencil /* = true */)
{
    // only supported on 11.1+
    if (m_pD3DContext1 == nullptr)
        return;

    if (m_nCurrentRenderTargets == 0 && m_pCurrentDepthBufferView == nullptr)
    {
        // on swapchain
        if (m_pCurrentSwapChain != nullptr)
        {
            if (discardColor)
                m_pD3DContext1->DiscardView(m_pCurrentSwapChain->GetRenderTargetView());
            if (discardDepth && discardStencil && m_pCurrentSwapChain->GetDepthStencilView() != nullptr)
                m_pD3DContext1->DiscardView(m_pCurrentSwapChain->GetDepthStencilView());
        }
    }
    else
    {
        if (discardColor)
        {
            for (uint32 i = 0; i < m_nCurrentRenderTargets; i++)
            {
                if (m_pCurrentRenderTargetViews[i] != nullptr)
                    m_pD3DContext1->DiscardView(m_pCurrentRenderTargetViews[i]->GetD3DRTV());
            }
        }

        if (discardDepth && discardStencil && m_pCurrentDepthBufferView != nullptr)
            m_pD3DContext1->DiscardView(m_pCurrentDepthBufferView->GetD3DDSV());
    }
}

GPUOutputBuffer *D3D11GPUContext::GetOutputBuffer()
{
    return m_pCurrentSwapChain;
}

void D3D11GPUContext::SetOutputBuffer(GPUOutputBuffer *pSwapChain)
{
    if (m_pCurrentSwapChain == pSwapChain)
        return;

    // copy out old swap chain
    D3D11GPUOutputBuffer *pOldSwapChain = m_pCurrentSwapChain;

    // copy in new swap chain
    if ((m_pCurrentSwapChain = static_cast<D3D11GPUOutputBuffer *>(pSwapChain)) != nullptr)
    {
//         DebugAssert(m_pCurrentSwapChain->GetOwnerContext() == nullptr);
//         if (m_pCurrentSwapChain->IsManaged())
//             static_cast<D3D11RendererOutputWindow *>(m_pCurrentSwapChain)->SetOwnerContext(this);
//         else
//             static_cast<D3D11GPUOutputBuffer *>(m_pCurrentSwapChain)->SetOwnerContext(this);

        m_pCurrentSwapChain->AddRef();
    }

    // Currently rendering to window?
    if (m_nCurrentRenderTargets == 0 && m_pCurrentDepthBufferView == nullptr)
        SynchronizeRenderTargetsAndUAVs();

    // update references
    if (pOldSwapChain != NULL)
    {
//         DebugAssert(pOldSwapChain->GetOwnerContext() == this);
//         if (pOldSwapChain->IsManaged())
//             static_cast<D3D11RendererOutputWindow *>(pOldSwapChain)->SetOwnerContext(nullptr);
//         else
//             static_cast<D3D11GPUOutputBuffer *>(pOldSwapChain)->SetOwnerContext(nullptr);

        pOldSwapChain->Release();
    }
}

bool D3D11GPUContext::GetExclusiveFullScreen()
{
    BOOL currentState;
    HRESULT hResult = m_pCurrentSwapChain->GetDXGISwapChain()->GetFullscreenState(&currentState, nullptr);
    if (FAILED(hResult))
        return false;

    return (currentState == TRUE);
}

bool D3D11GPUContext::SetExclusiveFullScreen(bool enabled, uint32 width, uint32 height, uint32 refreshRate)
{
    HRESULT hResult;

    // bound to pipeline? have to clear before switching modes
    if (m_nCurrentRenderTargets == 0 && m_pCurrentDepthBufferView == nullptr)
        m_pD3DContext->OMSetRenderTargets(0, nullptr, nullptr);

    // switch successful? ie have to resize buffers
    bool switchResult = true;
    uint32 newWidth = width;
    uint32 newHeight = height;

    // get current fullscreen state
    BOOL currentFullScreenState;
    if (FAILED(m_pCurrentSwapChain->GetDXGISwapChain()->GetFullscreenState(&currentFullScreenState, nullptr)))
        currentFullScreenState = FALSE;

    // to fullscreen?
    if (enabled)
    {
        // get output
        IDXGIOutput *pOutput;
        hResult = m_pDevice->GetDXGIAdapter()->EnumOutputs(0, &pOutput);
        if (SUCCEEDED(hResult))
        {
            // fill in mode details with requested width/height
            DXGI_MODE_DESC modeDesc;
            Y_memzero(&modeDesc, sizeof(modeDesc));
            modeDesc.Width = width;
            modeDesc.Height = height;
            modeDesc.RefreshRate.Numerator = refreshRate;
            modeDesc.RefreshRate.Denominator = 1;
            modeDesc.Format = m_pDevice->GetSwapChainBackBufferFormat();

            // find the best mode match
            DXGI_MODE_DESC closestMatch;
            hResult = pOutput->FindClosestMatchingMode(&modeDesc, &closestMatch, m_pD3DDevice);
            if (SUCCEEDED(hResult))
            {
                // update dimensions with closest match
                Log_InfoPrintf("D3D11GPUContext::SetExclusiveFullScreen: Switching to closest mode match %ux%u RefreshRate %u/%u", closestMatch.Width, closestMatch.Height, closestMatch.RefreshRate.Numerator, closestMatch.RefreshRate.Denominator);
                newWidth = closestMatch.Width;
                newHeight = closestMatch.Height;

                // switch to fullscreen state
                if (!currentFullScreenState)
                {
                    hResult = m_pCurrentSwapChain->GetDXGISwapChain()->SetFullscreenState(TRUE, pOutput);
                    if (FAILED(hResult))
                    {
                        Log_ErrorPrintf("D3D11GPUContext::SetExclusiveFullScreen: IDXGISwapChain::SetFullscreenState failed with hResult %08X.", hResult);
                        switchResult = false;
                    }
                }

                // resize the target
                if (switchResult)
                {
                    // call ResizeTarget to fix up the window size
                    hResult = m_pCurrentSwapChain->GetDXGISwapChain()->ResizeTarget(&closestMatch);
                    if (FAILED(hResult))
                    {
                        Log_ErrorPrintf("D3D11GPUContext::SetExclusiveFullScreen: IDXGISwapChain::ResizeTarget failed with hResult %08X.", hResult);
                        switchResult = false;
                    }
                }
            }
            else
            {
                Log_ErrorPrintf("D3D11GPUContext::SetExclusiveFullScreen: IDXGIOutput::FindClosestMatchingMode failed with hResult %08X.", hResult);
                switchResult = false;
            }

            pOutput->Release();
        }
        else
        {
            Log_ErrorPrintf("D3D11GPUContext::SetExclusiveFullScreen: IDXGIAdapter::EnumOutputs failed with hResult %08X.", hResult);
            switchResult = false;
        }
    }
    else
    {
        // remove if currently set
        if (currentFullScreenState)
        {
            hResult = m_pCurrentSwapChain->GetDXGISwapChain()->SetFullscreenState(FALSE, nullptr);
            if (FAILED(hResult))
            {
                Log_ErrorPrintf("D3D11GPUContext::SetExclusiveFullScreen: IDXGISwapChain::SetFullscreenState failed with hResult %08X.", hResult);
                switchResult = false;
            }
            else
            {
                // update dimensions
                RECT clientRect;
                GetClientRect(m_pCurrentSwapChain->GetHWND(), &clientRect);
                newWidth = Max(clientRect.right - clientRect.left, (LONG)1);
                newHeight = Max(clientRect.bottom - clientRect.top, (LONG)1);
            }
        }
    }

    // switch successful?
    if (switchResult)
    {
        // resize buffers
        m_pCurrentSwapChain->InternalResizeBuffers(newWidth, newHeight, m_pCurrentSwapChain->GetVSyncType());
    }

    // synchronize render targets if we were bound
    if (m_nCurrentRenderTargets == 0 && m_pCurrentDepthBufferView == nullptr)
        SynchronizeRenderTargetsAndUAVs();

    // done
    return true;
}

bool D3D11GPUContext::ResizeOutputBuffer(uint32 width /* = 0 */, uint32 height /* = 0 */)
{
    if (width == 0 || height == 0)
    {
        // get the new size of the window
        RECT clientRect;
        GetClientRect(m_pCurrentSwapChain->GetHWND(), &clientRect);

        // changed?
        width = Max(clientRect.right - clientRect.left, (LONG)1);
        height = Max(clientRect.bottom - clientRect.top, (LONG)1);
    }

    // changed?
    if (m_pCurrentSwapChain->GetWidth() == width && m_pCurrentSwapChain->GetHeight() == height)
        return true;

    // unbind if we're currently bound to the pipeline
    if (m_nCurrentRenderTargets == 0 && m_pCurrentDepthBufferView == nullptr)
        m_pD3DContext->OMSetRenderTargets(0, nullptr, nullptr);

    // invoke the resize
    m_pCurrentSwapChain->InternalResizeBuffers(width, height, m_pCurrentSwapChain->GetVSyncType());

    // synchronize render targets if we were bound
    if (m_nCurrentRenderTargets == 0 && m_pCurrentDepthBufferView == nullptr)
        SynchronizeRenderTargetsAndUAVs();

    // done
    return true;
}

void D3D11GPUContext::PresentOutputBuffer(GPU_PRESENT_BEHAVIOUR presentBehaviour)
{
    m_pCurrentSwapChain->GetDXGISwapChain()->Present((presentBehaviour == GPU_PRESENT_BEHAVIOUR_WAIT_FOR_VBLANK) ? 1 : 0, 0);
}

void D3D11GPUContext::BeginFrame()
{

}

void D3D11GPUContext::Flush()
{
    m_pD3DContext->Flush();
}

void D3D11GPUContext::Finish()
{
    // Doesn't exist in D3D11?
}

uint32 D3D11GPUContext::GetRenderTargets(uint32 nRenderTargets, GPURenderTargetView **ppRenderTargetViews, GPUDepthStencilBufferView **ppDepthBufferView)
{
    uint32 i, j;

    for (i = 0; i < m_nCurrentRenderTargets && i < nRenderTargets; i++)
        ppRenderTargetViews[i] = m_pCurrentRenderTargetViews[i];

    for (j = i; j < nRenderTargets; j++)
        ppRenderTargetViews[j] = nullptr;

    if (ppDepthBufferView != nullptr)
        *ppDepthBufferView = m_pCurrentDepthBufferView;

    return i;
}

void D3D11GPUContext::SetRenderTargets(uint32 nRenderTargets, GPURenderTargetView **ppRenderTargets, GPUDepthStencilBufferView *pDepthBufferView)
{
    // to system framebuffer?
    if ((nRenderTargets == 0 || (nRenderTargets == 1 && ppRenderTargets[0] == nullptr)) && pDepthBufferView == nullptr)
    {
        // change?
        if (m_nCurrentRenderTargets == 0 && m_pCurrentDepthBufferView == nullptr)
            return;

        // kill current targets
        for (uint32 i = 0; i < m_nCurrentRenderTargets; i++)
        {
            m_pCurrentRenderTargetViews[i]->Release();
            m_pCurrentRenderTargetViews[i] = nullptr;
        }
        if (m_pCurrentDepthBufferView != nullptr)
        {
            m_pCurrentDepthBufferView->Release();
            m_pCurrentDepthBufferView = nullptr;
        }

        m_nCurrentRenderTargets = 0;
        SynchronizeShaderBindings();
        SynchronizeRenderTargetsAndUAVs();
    }
    else
    {
        DebugAssert(nRenderTargets < countof(m_pCurrentRenderTargetViews));

        uint32 slot;
        uint32 newRenderTargetCount = 0;
        bool doUpdate = false;

        // set inclusive slots
        for (slot = 0; slot < nRenderTargets; slot++)
        {
            if (m_pCurrentRenderTargetViews[slot] != ppRenderTargets[slot])
            {
                if (m_pCurrentRenderTargetViews[slot] != nullptr)
                    m_pCurrentRenderTargetViews[slot]->Release();

                if ((m_pCurrentRenderTargetViews[slot] = static_cast<D3D11GPURenderTargetView *>(ppRenderTargets[slot])) != nullptr)
                    m_pCurrentRenderTargetViews[slot]->AddRef();

                doUpdate = true;
            }

            if (m_pCurrentRenderTargetViews[slot] != nullptr)
                newRenderTargetCount = slot + 1;
        }

        // clear extra slots
        for (; slot < m_nCurrentRenderTargets; slot++)
        {
            if (m_pCurrentRenderTargetViews[slot] != nullptr)
            {
                m_pCurrentRenderTargetViews[slot]->Release();
                m_pCurrentRenderTargetViews[slot] = nullptr;
                doUpdate = true;
            }
        }

        // update counter
        if (newRenderTargetCount != m_nCurrentRenderTargets)
            doUpdate = true;
        m_nCurrentRenderTargets = newRenderTargetCount;

        if (m_pCurrentDepthBufferView != pDepthBufferView)
        {
            if (m_pCurrentDepthBufferView != nullptr)
                m_pCurrentDepthBufferView->Release();

            if ((m_pCurrentDepthBufferView = static_cast<D3D11GPUDepthStencilBufferView *>(pDepthBufferView)) != nullptr)
                m_pCurrentDepthBufferView->AddRef();

            doUpdate = true;
        }

        if (doUpdate)
        {
            SynchronizeShaderBindings();
            SynchronizeRenderTargetsAndUAVs();
        }
    }
}

DRAW_TOPOLOGY D3D11GPUContext::GetDrawTopology()
{
    return m_currentTopology;
}

void D3D11GPUContext::SetDrawTopology(DRAW_TOPOLOGY topology)
{
    static const D3D11_PRIMITIVE_TOPOLOGY D3D11Topologies[DRAW_TOPOLOGY_COUNT] =
    {
        D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED,     // DRAW_TOPOLOGY_UNDEFINED
        D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,     // DRAW_TOPOLOGY_POINTS
        D3D11_PRIMITIVE_TOPOLOGY_LINELIST,      // DRAW_TOPOLOGY_LINE_LIST
        D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,     // DRAW_TOPOLOGY_LINE_STRIP
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,  // DRAW_TOPOLOGY_TRIANGLE_LIST
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, // DRAW_TOPOLOGY_TRIANGLE_STRIP
    };

    if (topology == m_currentTopology)
        return;

    DebugAssert(topology < DRAW_TOPOLOGY_COUNT);
    m_pD3DContext->IASetPrimitiveTopology(D3D11Topologies[topology]);
    m_currentTopology = topology;
}

uint32 D3D11GPUContext::GetVertexBuffers(uint32 firstBuffer, uint32 nBuffers, GPUBuffer **ppVertexBuffers, uint32 *pVertexBufferOffsets, uint32 *pVertexBufferStrides)
{
    DebugAssert(firstBuffer + nBuffers < countof(m_pCurrentVertexBuffers));

    uint32 saveCount;
    for (saveCount = 0; saveCount < nBuffers; saveCount++)
    {
        if ((firstBuffer + saveCount) > m_currentVertexBufferBindCount)
            break;

        ppVertexBuffers[saveCount] = m_pCurrentVertexBuffers[firstBuffer + saveCount];
        pVertexBufferOffsets[saveCount] = m_currentVertexBufferOffsets[firstBuffer + saveCount];
        pVertexBufferStrides[saveCount] = m_currentVertexBufferStrides[firstBuffer + saveCount];
    }

    return saveCount;
}

void D3D11GPUContext::SetVertexBuffers(uint32 firstBuffer, uint32 nBuffers, GPUBuffer *const *ppVertexBuffers, const uint32 *pVertexBufferOffsets, const uint32 *pVertexBufferStrides)
{
    ID3D11Buffer *pD3DBuffers[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
    UINT D3DBufferOffsets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
    UINT D3DBufferStrides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];

    for (uint32 i = 0; i < nBuffers; i++)
    {
        D3D11GPUBuffer *pD3D11VertexBuffer = static_cast<D3D11GPUBuffer *>(ppVertexBuffers[i]);
        uint32 bufferIndex = firstBuffer + i;

        if (m_pCurrentVertexBuffers[bufferIndex] != nullptr)
        {
            m_pCurrentVertexBuffers[bufferIndex]->Release();
            m_pCurrentVertexBuffers[bufferIndex] = nullptr;
        }

        if ((m_pCurrentVertexBuffers[bufferIndex] = pD3D11VertexBuffer) != nullptr)
        {
            pD3D11VertexBuffer->AddRef();
            m_currentVertexBufferOffsets[bufferIndex] = pVertexBufferOffsets[i];
            m_currentVertexBufferStrides[bufferIndex] = pVertexBufferStrides[i];

            pD3DBuffers[i] = pD3D11VertexBuffer->GetD3DBuffer();
            D3DBufferOffsets[i] = pVertexBufferOffsets[i];
            D3DBufferStrides[i] = pVertexBufferStrides[i];
        }
        else
        {
            m_currentVertexBufferOffsets[bufferIndex] = 0;
            m_currentVertexBufferStrides[bufferIndex] = 0;

            pD3DBuffers[i] = nullptr;
            D3DBufferOffsets[i] = 0;
            D3DBufferStrides[i] = 0;
        }
    }

    // todo: pending set
    m_pD3DContext->IASetVertexBuffers(firstBuffer, nBuffers, pD3DBuffers, D3DBufferStrides, D3DBufferOffsets);

    // update new bind count
    uint32 bindCount = 0;
    uint32 searchCount = Max((firstBuffer + nBuffers), m_currentVertexBufferBindCount);
    for (uint32 i = 0; i < searchCount; i++)
    {
        if (m_pCurrentVertexBuffers[i] != nullptr)
            bindCount = i + 1;
    }
    m_currentVertexBufferBindCount = bindCount;
}

void D3D11GPUContext::SetVertexBuffer(uint32 bufferIndex, GPUBuffer *pVertexBuffer, uint32 offset, uint32 stride)
{
    if (m_pCurrentVertexBuffers[bufferIndex] == pVertexBuffer &&
        m_currentVertexBufferOffsets[bufferIndex] == offset &&
        m_currentVertexBufferStrides[bufferIndex] == stride)
    {
        return;
    }

    if (m_pCurrentVertexBuffers[bufferIndex] != pVertexBuffer)
    {
        if (m_pCurrentVertexBuffers[bufferIndex] != nullptr)
            m_pCurrentVertexBuffers[bufferIndex]->Release();

        if ((m_pCurrentVertexBuffers[bufferIndex] = static_cast<D3D11GPUBuffer *>(pVertexBuffer)) != nullptr)
            m_pCurrentVertexBuffers[bufferIndex]->AddRef();
    }

    m_currentVertexBufferOffsets[bufferIndex] = offset;
    m_currentVertexBufferStrides[bufferIndex] = stride;

    // todo: pending set
    ID3D11Buffer *pD3DBuffer = (pVertexBuffer != nullptr) ? static_cast<D3D11GPUBuffer *>(pVertexBuffer)->GetD3DBuffer() : nullptr;
    UINT D3DOffset = (pVertexBuffer != nullptr) ? offset : 0;
    UINT D3DStride = (pVertexBuffer != nullptr) ? stride : 0;
    m_pD3DContext->IASetVertexBuffers(bufferIndex, 1, &pD3DBuffer, &D3DStride, &D3DOffset);

    // update new bind count
    uint32 bindCount = 0;
    uint32 searchCount = Max((bufferIndex + 1), m_currentVertexBufferBindCount);
    for (uint32 i = 0; i < searchCount; i++)
    {
        if (m_pCurrentVertexBuffers[i] != nullptr)
            bindCount = i + 1;
    }
    m_currentVertexBufferBindCount = bindCount;
}

void D3D11GPUContext::GetIndexBuffer(GPUBuffer **ppBuffer, GPU_INDEX_FORMAT *pFormat, uint32 *pOffset)
{
    *ppBuffer = m_pCurrentIndexBuffer;
    *pFormat = m_currentIndexFormat;
    *pOffset = m_currentIndexBufferOffset;
}

void D3D11GPUContext::SetIndexBuffer(GPUBuffer *pBuffer, GPU_INDEX_FORMAT format, uint32 offset)
{
    if (m_pCurrentIndexBuffer == pBuffer && m_currentIndexFormat == format && m_currentIndexBufferOffset == offset)
        return;

    if (m_pCurrentIndexBuffer != pBuffer)
    {
        if (m_pCurrentIndexBuffer != NULL)
            m_pCurrentIndexBuffer->Release();

        if ((m_pCurrentIndexBuffer = static_cast<D3D11GPUBuffer *>(pBuffer)) != NULL)
            m_pCurrentIndexBuffer->AddRef();
    }

    m_currentIndexFormat = format;
    m_currentIndexBufferOffset = offset;

    if (m_pCurrentIndexBuffer != NULL)
        m_pD3DContext->IASetIndexBuffer(m_pCurrentIndexBuffer->GetD3DBuffer(), (format == GPU_INDEX_FORMAT_UINT16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, m_currentIndexBufferOffset);
    else
        m_pD3DContext->IASetIndexBuffer(NULL, DXGI_FORMAT_R16_UINT, 0);
}

void D3D11GPUContext::SetInputLayout(GPUInputLayout* pInputLayout)
{
    if (m_pCurrentInputLayout == pInputLayout)
        return;

    if (m_pCurrentInputLayout)
        m_pCurrentInputLayout->Release();

    m_pCurrentInputLayout = static_cast<D3D11GPUInputLayout*>(pInputLayout);
    if (m_pCurrentInputLayout)
        m_pCurrentInputLayout->AddRef();

    m_pD3DContext->IASetInputLayout((m_pCurrentInputLayout) ? m_pCurrentInputLayout->GetD3DInputLayout() : nullptr);
}

void D3D11GPUContext::SetShaderProgram(GPUShaderProgram *pShaderProgram)
{
    if (m_pCurrentShaderProgram == pShaderProgram)
        return;

    D3D11GPUShaderProgram *pNewShaderProgram = static_cast<D3D11GPUShaderProgram *>(pShaderProgram);
    if (pNewShaderProgram)
    {
        pNewShaderProgram->Bind(this, m_pCurrentShaderProgram);
        pNewShaderProgram->AddRef();

        if (m_pCurrentShaderProgram)
            m_pCurrentShaderProgram->Release();

        m_pCurrentShaderProgram = pNewShaderProgram;
    }
    else
    {
        m_pCurrentShaderProgram->Unbind(this);
        m_pCurrentShaderProgram->Release();
        m_pCurrentShaderProgram = nullptr;
    }

    m_pDevice->GetCounters()->IncrementShaderChangeCounter();

    // Invalidate all bindings
    // TODO: Use only the active range rather than the whole thing
    m_shaderBindings.FirstDirtyConstantBuffer = 0;
    m_shaderBindings.LastDirtyConstantBuffer = m_shaderBindings.ConstantBuffers.GetSize() - 1;
    m_shaderBindings.FirstDirtySampler = 0;
    m_shaderBindings.LastDirtySampler = m_shaderBindings.Samplers.GetSize() - 1;
    m_shaderBindings.FirstDirtyResource = 0;
    m_shaderBindings.LastDirtyResource = m_shaderBindings.Resources.GetSize() - 1;
    m_shaderBindings.FirstDirtyRWResource = 0;
    m_shaderBindings.LastDirtyRWResource = m_shaderBindings.RWResources.GetSize() - 1;    
}

void D3D11GPUContext::SetShaderConstantBuffer(uint32 index, GPUBuffer* pBuffer)
{
    DebugAssert(index < m_shaderBindings.ConstantBuffers.GetSize());

    if (m_shaderBindings.ConstantBuffers[index] != pBuffer)
    {
        if (m_shaderBindings.FirstDirtyConstantBuffer < 0)
        {
            m_shaderBindings.FirstDirtyConstantBuffer = (int32)index;
            m_shaderBindings.LastDirtyConstantBuffer = (int32)index;
        }
        else
        {
            m_shaderBindings.FirstDirtyConstantBuffer = Min(m_shaderBindings.FirstDirtyConstantBuffer, (int32)index);
            m_shaderBindings.LastDirtyConstantBuffer = Max(m_shaderBindings.LastDirtyConstantBuffer, (int32)index);
        }

        if (m_shaderBindings.ConstantBuffers[index])
            m_shaderBindings.ConstantBuffers[index]->Release();

        m_shaderBindings.ConstantBuffers[index] = static_cast<D3D11GPUBuffer*>(pBuffer);
        if (m_shaderBindings.ConstantBuffers[index])
            m_shaderBindings.ConstantBuffers[index]->AddRef();
    }
}

void D3D11GPUContext::SetShaderSampler(uint32 index, GPUSamplerState* pSamplerState)
{
    DebugAssert(index < m_shaderBindings.Samplers.GetSize());

    if (m_shaderBindings.Samplers[index] != pSamplerState)
    {
        if (m_shaderBindings.FirstDirtySampler < 0)
        {
            m_shaderBindings.FirstDirtySampler = (int32)index;
            m_shaderBindings.LastDirtySampler = (int32)index;
        }
        else
        {
            m_shaderBindings.FirstDirtySampler = Min(m_shaderBindings.FirstDirtySampler, (int32)index);
            m_shaderBindings.LastDirtySampler = Max(m_shaderBindings.LastDirtySampler, (int32)index);
        }

        if (m_shaderBindings.Samplers[index])
            m_shaderBindings.Samplers[index]->Release();

        m_shaderBindings.Samplers[index] = static_cast<D3D11GPUSamplerState*>(pSamplerState);
        if (m_shaderBindings.Samplers[index])
            m_shaderBindings.Samplers[index]->AddRef();
    }
}

void D3D11GPUContext::SetShaderResource(uint32 index, GPUResource* pResource)
{
    DebugAssert(index < m_shaderBindings.Resources.GetSize());

    if (m_shaderBindings.Resources[index] != pResource)
    {
        if (m_shaderBindings.FirstDirtyResource < 0)
        {
            m_shaderBindings.FirstDirtyResource = (int32)index;
            m_shaderBindings.LastDirtyResource = (int32)index;
        }
        else
        {
            m_shaderBindings.FirstDirtyResource = Min(m_shaderBindings.FirstDirtyResource, (int32)index);
            m_shaderBindings.LastDirtyResource = Max(m_shaderBindings.LastDirtyResource, (int32)index);
        }

        if (m_shaderBindings.Resources[index])
            m_shaderBindings.Resources[index]->Release();

        m_shaderBindings.Resources[index] = pResource;
        if (m_shaderBindings.Resources[index])
            m_shaderBindings.Resources[index]->AddRef();
    }
}

void D3D11GPUContext::SetShaderRWResource(uint32 index, GPUResource* pResource)
{
    DebugAssert(index < m_shaderBindings.RWResources.GetSize());

    if (m_shaderBindings.RWResources[index] != pResource)
    {
        if (m_shaderBindings.FirstDirtyRWResource < 0)
        {
            m_shaderBindings.FirstDirtyRWResource = (int32)index;
            m_shaderBindings.LastDirtyRWResource = (int32)index;
        }
        else
        {
            m_shaderBindings.FirstDirtyRWResource = Min(m_shaderBindings.FirstDirtyRWResource, (int32)index);
            m_shaderBindings.LastDirtyRWResource = Max(m_shaderBindings.LastDirtyRWResource, (int32)index);
        }

        if (m_shaderBindings.RWResources[index])
            m_shaderBindings.RWResources[index]->Release();

        m_shaderBindings.RWResources[index] = pResource;
        if (m_shaderBindings.RWResources[index])
            m_shaderBindings.RWResources[index]->AddRef();
    }
}

void D3D11GPUContext::SetShaderConstantBuffers(SHADER_PROGRAM_STAGE stage, uint32 index, ID3D11Buffer *pBuffer)
{
    ShaderStageState *state = &m_shaderStates[stage];
    if (state->ConstantBuffers[index] == pBuffer)
        return;

    if (state->ConstantBuffers[index] != nullptr)
        state->ConstantBuffers[index]->Release();
    if ((state->ConstantBuffers[index] = pBuffer) != nullptr)
        pBuffer->AddRef();

    if (state->ConstantBufferDirtyLowerBounds < 0)
    {
        state->ConstantBufferDirtyLowerBounds = index;
        state->ConstantBufferDirtyUpperBounds = index;
    }
    else
    {
        state->ConstantBufferDirtyLowerBounds = Min(state->ConstantBufferDirtyLowerBounds, (int32)index);
        state->ConstantBufferDirtyUpperBounds = Max(state->ConstantBufferDirtyUpperBounds, (int32)index);
    }
}

void D3D11GPUContext::SetShaderResources(SHADER_PROGRAM_STAGE stage, uint32 index, ID3D11ShaderResourceView *pResource)
{
    ShaderStageState *state = &m_shaderStates[stage];
    if (state->Resources[index] == pResource)
        return;

    if (state->Resources[index] != nullptr)
        state->Resources[index]->Release();
    if ((state->Resources[index] = pResource) != nullptr)
        pResource->AddRef();

    if (state->ResourceDirtyLowerBounds < 0)
    {
        state->ResourceDirtyLowerBounds = index;
        state->ResourceDirtyUpperBounds = index;
    }
    else
    {
        state->ResourceDirtyLowerBounds = Min(state->ResourceDirtyLowerBounds, (int32)index);
        state->ResourceDirtyUpperBounds = Max(state->ResourceDirtyUpperBounds, (int32)index);
    }
}

void D3D11GPUContext::SetShaderSamplers(SHADER_PROGRAM_STAGE stage, uint32 index, ID3D11SamplerState *pSampler)
{
    ShaderStageState *state = &m_shaderStates[stage];
    if (state->Samplers[index] == pSampler)
        return;

    if (state->Samplers[index] != nullptr)
        state->Samplers[index]->Release();
    if ((state->Samplers[index] = pSampler) != nullptr)
        pSampler->AddRef();

    if (state->SamplerDirtyLowerBounds < 0)
    {
        state->SamplerDirtyLowerBounds = index;
        state->SamplerDirtyUpperBounds = index;
    }
    else
    {
        state->SamplerDirtyLowerBounds = Min(state->SamplerDirtyLowerBounds, (int32)index);
        state->SamplerDirtyUpperBounds = Max(state->SamplerDirtyUpperBounds, (int32)index);
    }
}

void D3D11GPUContext::SetShaderUAVs(SHADER_PROGRAM_STAGE stage, uint32 index, ID3D11UnorderedAccessView *pResource)
{
    ShaderStageState *state = &m_shaderStates[stage];
    if (state->UAVs[index] == pResource)
        return;

    if (state->UAVs[index] != nullptr)
        state->UAVs[index]->Release();
    if ((state->UAVs[index] = pResource) != nullptr)
        pResource->AddRef();

    if (state->UAVDirtyLowerBounds < 0)
    {
        state->UAVDirtyLowerBounds = index;
        state->UAVDirtyUpperBounds = index;
    }
    else
    {
        state->UAVDirtyLowerBounds = Min(state->UAVDirtyLowerBounds, (int32)index);
        state->UAVDirtyUpperBounds = Max(state->UAVDirtyUpperBounds, (int32)index);
    }
}

void D3D11GPUContext::SynchronizeRenderTargetsAndUAVs()
{
    ID3D11RenderTargetView *renderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
    ID3D11DepthStencilView *depthStencilView = nullptr;
    uint32 renderTargetCount = 0;

    // get render target views
    if (m_nCurrentRenderTargets == 0 && m_pCurrentDepthBufferView == nullptr)
    {
        // using swap chain
        if (m_pCurrentSwapChain != nullptr)
        {
            renderTargetViews[0] = m_pCurrentSwapChain->GetRenderTargetView();
            depthStencilView = m_pCurrentSwapChain->GetDepthStencilView();
            renderTargetCount = 1;
        }
    }
    else
    {
        // get render target views
        for (uint32 renderTargetIndex = 0; renderTargetIndex < m_nCurrentRenderTargets; renderTargetIndex++)
        {
            if (m_pCurrentRenderTargetViews[renderTargetIndex] != nullptr)
            {
                if ((renderTargetViews[renderTargetIndex] = m_pCurrentRenderTargetViews[renderTargetIndex]->GetD3DRTV()) != nullptr)
                    renderTargetCount = renderTargetIndex + 1;
            }
        }

        // get depth stencil view
        if (m_pCurrentDepthBufferView != nullptr)
            depthStencilView = m_pCurrentDepthBufferView->GetD3DDSV();
    }

    // get unordered access views
    ID3D11UnorderedAccessView *unorderedAccessViews[D3D11_PS_CS_UAV_REGISTER_COUNT];
    UINT unorderedAccessViewInitialCounts[D3D11_PS_CS_UAV_REGISTER_COUNT];
    uint32 unorderedAccessCount = 0;
    if (m_shaderStates[SHADER_PROGRAM_STAGE_PIXEL_SHADER].UAVBindCount > 0)
    {
        const ShaderStageState *state = &m_shaderStates[SHADER_PROGRAM_STAGE_PIXEL_SHADER];
        for (uint32 i = 0; i < state->UAVBindCount; i++)
            unorderedAccessViewInitialCounts[i] = 0;

        unorderedAccessCount = state->UAVBindCount;
    }

    // call through
    if (unorderedAccessCount > 0)
        m_pD3DContext->OMSetRenderTargetsAndUnorderedAccessViews(renderTargetCount, (renderTargetCount > 0) ? renderTargetViews : nullptr, depthStencilView, 0, unorderedAccessCount, unorderedAccessViews, unorderedAccessViewInitialCounts);
    else
        m_pD3DContext->OMSetRenderTargets(renderTargetCount, (renderTargetCount > 0) ? renderTargetViews : nullptr, depthStencilView);
}

void D3D11GPUContext::SynchronizeShaderBindings()
{
    // apply pending bindings
    if (m_pCurrentShaderProgram)
    {
        m_pCurrentShaderProgram->UpdateUniforms(this);

        if (m_shaderBindings.FirstDirtyConstantBuffer >= 0)
        {
            m_pCurrentShaderProgram->UpdateConstantBuffers(this, m_shaderBindings.FirstDirtyConstantBuffer, m_shaderBindings.LastDirtyConstantBuffer, m_shaderBindings.ConstantBuffers.GetBasePointer());
            m_shaderBindings.FirstDirtyConstantBuffer = m_shaderBindings.LastDirtyConstantBuffer = -1;
        }

        if (m_shaderBindings.FirstDirtySampler >= 0)
        {
            m_pCurrentShaderProgram->UpdateSamplers(this, m_shaderBindings.FirstDirtySampler, m_shaderBindings.LastDirtySampler, m_shaderBindings.Samplers.GetBasePointer());
            m_shaderBindings.FirstDirtySampler = m_shaderBindings.LastDirtySampler = -1;
        }

        if (m_shaderBindings.FirstDirtyResource >= 0)
        {
            m_pCurrentShaderProgram->UpdateResources(this, m_shaderBindings.FirstDirtyResource, m_shaderBindings.LastDirtyResource, m_shaderBindings.Resources.GetBasePointer(), m_shaderBindings.Samplers.GetBasePointer());
            m_shaderBindings.FirstDirtyResource = m_shaderBindings.LastDirtyResource = -1;
        }

        if (m_shaderBindings.FirstDirtyRWResource >= 0)
        {
            m_pCurrentShaderProgram->UpdateRWResources(this, m_shaderBindings.FirstDirtyRWResource, m_shaderBindings.LastDirtyRWResource, m_shaderBindings.RWResources.GetBasePointer());
            m_shaderBindings.FirstDirtyRWResource = m_shaderBindings.LastDirtyRWResource = -1;
        }
    }

    // update states
    for (uint32 stage = 0; stage < SHADER_PROGRAM_STAGE_COUNT; stage++)
    {
        ShaderStageState *state = &m_shaderStates[stage];

        // Constant buffers
        if (state->ConstantBufferDirtyLowerBounds >= 0)
        {
            int32 firstConstantBuffer = state->ConstantBufferDirtyLowerBounds;
            int32 setCount = state->ConstantBufferDirtyUpperBounds - state->ConstantBufferDirtyLowerBounds + 1;

            // find the new bind count
            uint32 searchMax = Max((uint32)state->ConstantBufferDirtyUpperBounds + 1, state->ConstantBufferBindCount);
            uint32 bindCount = 0;
            for (uint32 i = 0; i < searchMax; i++)
            {
                if (state->ConstantBuffers[i] != nullptr)
                    bindCount = i + 1;
            }
            state->ConstantBufferBindCount = bindCount;
            state->ConstantBufferDirtyLowerBounds = state->ConstantBufferDirtyUpperBounds = -1;

            // update d3d
            switch (stage)
            {
            case SHADER_PROGRAM_STAGE_VERTEX_SHADER:    m_pD3DContext->VSSetConstantBuffers(firstConstantBuffer, setCount, state->ConstantBuffers + firstConstantBuffer);   break;
            case SHADER_PROGRAM_STAGE_HULL_SHADER:      m_pD3DContext->HSSetConstantBuffers(firstConstantBuffer, setCount, state->ConstantBuffers + firstConstantBuffer);   break;
            case SHADER_PROGRAM_STAGE_DOMAIN_SHADER:    m_pD3DContext->DSSetConstantBuffers(firstConstantBuffer, setCount, state->ConstantBuffers + firstConstantBuffer);   break;
            case SHADER_PROGRAM_STAGE_GEOMETRY_SHADER:  m_pD3DContext->GSSetConstantBuffers(firstConstantBuffer, setCount, state->ConstantBuffers + firstConstantBuffer);   break;
            case SHADER_PROGRAM_STAGE_PIXEL_SHADER:     m_pD3DContext->PSSetConstantBuffers(firstConstantBuffer, setCount, state->ConstantBuffers + firstConstantBuffer);   break;
            case SHADER_PROGRAM_STAGE_COMPUTE_SHADER:   m_pD3DContext->CSSetConstantBuffers(firstConstantBuffer, setCount, state->ConstantBuffers + firstConstantBuffer);   break;
            }
        }

        // Resources
        if (state->ResourceDirtyLowerBounds >= 0)
        {
            int32 firstResource = state->ResourceDirtyLowerBounds;
            int32 setCount = state->ResourceDirtyUpperBounds - state->ResourceDirtyLowerBounds + 1;

            // find the new bind count
            uint32 searchMax = Max((uint32)state->ResourceDirtyUpperBounds + 1, state->ResourceBindCount);
            uint32 bindCount = 0;
            for (uint32 i = 0; i < searchMax; i++)
            {
                if (state->Resources[i] != nullptr)
                    bindCount = i + 1;
            }
            state->ResourceBindCount = bindCount;
            state->ResourceDirtyLowerBounds = state->ResourceDirtyUpperBounds = -1;

            // update d3d
            switch (stage)
            {
            case SHADER_PROGRAM_STAGE_VERTEX_SHADER:    m_pD3DContext->VSSetShaderResources(firstResource, setCount, state->Resources + firstResource);     break;
            case SHADER_PROGRAM_STAGE_HULL_SHADER:      m_pD3DContext->HSSetShaderResources(firstResource, setCount, state->Resources + firstResource);     break;
            case SHADER_PROGRAM_STAGE_DOMAIN_SHADER:    m_pD3DContext->DSSetShaderResources(firstResource, setCount, state->Resources + firstResource);     break;
            case SHADER_PROGRAM_STAGE_GEOMETRY_SHADER:  m_pD3DContext->GSSetShaderResources(firstResource, setCount, state->Resources + firstResource);     break;
            case SHADER_PROGRAM_STAGE_PIXEL_SHADER:     m_pD3DContext->PSSetShaderResources(firstResource, setCount, state->Resources + firstResource);     break;
            case SHADER_PROGRAM_STAGE_COMPUTE_SHADER:   m_pD3DContext->CSSetShaderResources(firstResource, setCount, state->Resources + firstResource);     break;
            }
        }

        // Samplers
        if (state->SamplerDirtyLowerBounds >= 0)
        {
            int32 firstSampler = state->SamplerDirtyLowerBounds;
            int32 setCount = state->SamplerDirtyUpperBounds - state->SamplerDirtyLowerBounds + 1;

            // find the new bind count
            uint32 searchMax = Max((uint32)state->SamplerDirtyUpperBounds + 1, state->SamplerBindCount);
            uint32 bindCount = 0;
            for (uint32 i = 0; i < searchMax; i++)
            {
                if (state->Samplers[i] != nullptr)
                    bindCount = i + 1;
            }
            state->SamplerBindCount = bindCount;
            state->SamplerDirtyLowerBounds = state->SamplerDirtyUpperBounds = -1;

            // update d3d
            switch (stage)
            {
            case SHADER_PROGRAM_STAGE_VERTEX_SHADER:    m_pD3DContext->VSSetSamplers(firstSampler, setCount, state->Samplers + firstSampler);       break;
            case SHADER_PROGRAM_STAGE_HULL_SHADER:      m_pD3DContext->HSSetSamplers(firstSampler, setCount, state->Samplers + firstSampler);       break;
            case SHADER_PROGRAM_STAGE_DOMAIN_SHADER:    m_pD3DContext->DSSetSamplers(firstSampler, setCount, state->Samplers + firstSampler);       break;
            case SHADER_PROGRAM_STAGE_GEOMETRY_SHADER:  m_pD3DContext->GSSetSamplers(firstSampler, setCount, state->Samplers + firstSampler);       break;
            case SHADER_PROGRAM_STAGE_PIXEL_SHADER:     m_pD3DContext->PSSetSamplers(firstSampler, setCount, state->Samplers + firstSampler);       break;
            case SHADER_PROGRAM_STAGE_COMPUTE_SHADER:   m_pD3DContext->CSSetSamplers(firstSampler, setCount, state->Samplers + firstSampler);       break;
            }
        }

        // UAVs
        if (state->UAVDirtyLowerBounds >= 0)
        {
            int32 firstUAV = state->UAVDirtyLowerBounds;
            int32 setCount = state->UAVDirtyUpperBounds - state->UAVDirtyLowerBounds + 1;

            // find the new bind count
            uint32 searchMax = Max((uint32)state->UAVDirtyUpperBounds + 1, state->UAVBindCount);
            uint32 bindCount = 0;
            for (uint32 i = 0; i < searchMax; i++)
            {
                if (state->UAVs[i] != nullptr)
                    bindCount = i + 1;
            }
            state->UAVBindCount = bindCount;
            state->UAVDirtyLowerBounds = state->UAVDirtyUpperBounds = -1;

            // update d3d
            switch (stage)
            {
            case SHADER_PROGRAM_STAGE_PIXEL_SHADER:     SynchronizeRenderTargetsAndUAVs();                                                              break;
            case SHADER_PROGRAM_STAGE_COMPUTE_SHADER:   m_pD3DContext->CSSetUnorderedAccessViews(firstUAV, setCount, state->UAVs + firstUAV, nullptr);  break;
            }
        }
    }
}

void D3D11GPUContext::Draw(uint32 firstVertex, uint32 nVertices)
{
    if (nVertices == 0)
        return;
    
    DebugAssert(m_pCurrentShaderProgram != nullptr);
    SynchronizeShaderBindings();

    m_pD3DContext->Draw(nVertices, firstVertex);
    m_pDevice->GetCounters()->IncrementDrawCallCounter();
}

void D3D11GPUContext::DrawInstanced(uint32 firstVertex, uint32 nVertices, uint32 nInstances)
{
    if (nVertices == 0 || nInstances == 0)
        return;

    DebugAssert(m_pCurrentShaderProgram != nullptr);
    SynchronizeShaderBindings();

    m_pD3DContext->DrawInstanced(nVertices, nInstances, firstVertex, 0);
    m_pDevice->GetCounters()->IncrementDrawCallCounter();
}

void D3D11GPUContext::DrawIndexed(uint32 startIndex, uint32 nIndices, uint32 baseVertex)
{
    if (nIndices == 0)
        return;
     
    DebugAssert(m_pCurrentShaderProgram != nullptr);
    SynchronizeShaderBindings();

    m_pD3DContext->DrawIndexed(nIndices, startIndex, baseVertex);
    m_pDevice->GetCounters()->IncrementDrawCallCounter();
}

void D3D11GPUContext::DrawIndexedInstanced(uint32 startIndex, uint32 nIndices, uint32 baseVertex, uint32 nInstances)
{
    if (nIndices == 0)
        return;

    DebugAssert(m_pCurrentShaderProgram != nullptr);
    SynchronizeShaderBindings();

    m_pD3DContext->DrawIndexedInstanced(nIndices, nInstances, startIndex, baseVertex, 0);
    m_pDevice->GetCounters()->IncrementDrawCallCounter();
}

void D3D11GPUContext::Dispatch(uint32 threadGroupCountX, uint32 threadGroupCountY, uint32 threadGroupCountZ)
{
    DebugAssert(m_pCurrentShaderProgram != nullptr);
    SynchronizeShaderBindings();

    m_pD3DContext->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
    m_pDevice->GetCounters()->IncrementDrawCallCounter();
}

void D3D11GPUContext::DrawUserPointer(const void *pVertices, uint32 vertexSize, uint32 nVertices)
{
    const byte *pVerticesPtr = reinterpret_cast<const byte *>(pVertices);
    uint32 remainingVertices = nVertices;

    // setup draw
    DebugAssert(m_pCurrentShaderProgram != nullptr);
    SynchronizeShaderBindings();

    // while we still have vertices left to draw...
    while (remainingVertices > 0)
    {
        uint32 maxVertices = Min(nVertices, (m_userVertexBufferSize / vertexSize));
        uint32 requestedSpace = maxVertices * vertexSize;
        DebugAssert(maxVertices > 0);

        HRESULT hResult;
        D3D11_MAPPED_SUBRESOURCE mappedSubResource;
        if ((m_userVertexBufferPosition + requestedSpace) < m_userVertexBufferSize)
        {
            // fit into remaining space
            hResult = m_pD3DContext->Map(m_pUserVertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedSubResource);
        }
        else
        {
            // discard all of the buffer
            m_userVertexBufferPosition = 0;
            hResult = m_pD3DContext->Map(m_pUserVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource);
        }

        // should be valid
        if (FAILED(hResult))
        {
            Log_ErrorPrintf("D3D11GPUContext::DrawUserPointer: Map failed with hResult %08X", hResult);
            return;
        }

        // fill buffer
        Y_memcpy(reinterpret_cast<byte *>(mappedSubResource.pData) + m_userVertexBufferPosition, pVerticesPtr, vertexSize * maxVertices);

        // unmap buffer
        m_pD3DContext->Unmap(m_pUserVertexBuffer, 0);

        // update vb
        m_pD3DContext->IASetVertexBuffers(0, 1, &m_pUserVertexBuffer, &vertexSize, &m_userVertexBufferPosition);

        // draw it
        m_pD3DContext->Draw(maxVertices, 0);
        m_pDevice->GetCounters()->IncrementDrawCallCounter();

        // increment positions
        m_userVertexBufferPosition += requestedSpace;
        pVerticesPtr += maxVertices * vertexSize;
        remainingVertices -= maxVertices;
    }

    // restore state
    if (m_currentVertexBufferBindCount > 0)
    {
        ID3D11Buffer *pD3DBuffer = (m_pCurrentVertexBuffers[0] != nullptr) ? m_pCurrentVertexBuffers[0]->GetD3DBuffer() : nullptr;
        UINT D3DOffset = m_currentVertexBufferOffsets[0];
        UINT D3DStride = m_currentVertexBufferStrides[0];
        m_pD3DContext->IASetVertexBuffers(0, 1, &pD3DBuffer, &D3DStride, &D3DOffset);
    }
    else
    {
        // unbind the temp one
        ID3D11Buffer *pNullD3DBuffer = NULL;
        UINT nullD3DOffsetStride = 0;
        m_pD3DContext->IASetVertexBuffers(0, 1, &pNullD3DBuffer, &nullD3DOffsetStride, &nullD3DOffsetStride);
    }
}

bool D3D11GPUContext::CopyTexture(GPUTexture2D *pSourceTexture, GPUTexture2D *pDestinationTexture)
{
    // textures have to be compatible, for now this means same texture format
    D3D11GPUTexture2D *pD3D11SourceTexture = static_cast<D3D11GPUTexture2D *>(pSourceTexture);
    D3D11GPUTexture2D *pD3D11DestinationTexture = static_cast<D3D11GPUTexture2D *>(pDestinationTexture);
    if (pD3D11SourceTexture->GetDesc()->Width != pD3D11DestinationTexture->GetDesc()->Width ||
        pD3D11SourceTexture->GetDesc()->Height != pD3D11DestinationTexture->GetDesc()->Height ||
        pD3D11SourceTexture->GetDesc()->Format != pD3D11DestinationTexture->GetDesc()->Format ||
        pD3D11SourceTexture->GetDesc()->MipLevels != pD3D11DestinationTexture->GetDesc()->MipLevels)
    {
        return false;
    }

    // copy it
    m_pD3DContext->CopyResource(pD3D11DestinationTexture->GetD3DTexture(), pD3D11SourceTexture->GetD3DTexture());
    return true;
}

bool D3D11GPUContext::CopyTextureRegion(GPUTexture2D *pSourceTexture, uint32 sourceX, uint32 sourceY, uint32 width, uint32 height, uint32 sourceMipLevel, GPUTexture2D *pDestinationTexture, uint32 destX, uint32 destY, uint32 destMipLevel)
{
    // textures have to be compatible, for now this means same texture format
    D3D11GPUTexture2D *pD3D11SourceTexture = static_cast<D3D11GPUTexture2D *>(pSourceTexture);
    D3D11GPUTexture2D *pD3D11DestinationTexture = static_cast<D3D11GPUTexture2D *>(pDestinationTexture);
    if (pD3D11SourceTexture->GetDesc()->Format != pD3D11DestinationTexture->GetDesc()->Format ||
        pD3D11SourceTexture->GetDesc()->MipLevels != pD3D11DestinationTexture->GetDesc()->MipLevels)
    {
        return false;
    }

    // create source box, copy it
    D3D11_BOX sourceBox = { sourceX, sourceY, 0, sourceX + width, sourceY + height, 1 };
    m_pD3DContext->CopySubresourceRegion(pD3D11DestinationTexture->GetD3DTexture(), D3D11CalcSubresource(destMipLevel, 0, pD3D11DestinationTexture->GetDesc()->MipLevels), destX, destY, 0,
                                         pD3D11SourceTexture->GetD3DTexture(), D3D11CalcSubresource(sourceMipLevel, 0, pD3D11SourceTexture->GetDesc()->MipLevels), &sourceBox);

    return true;
}

void D3D11GPUContext::BlitFrameBuffer(GPUTexture2D *pTexture, uint32 sourceX, uint32 sourceY, uint32 sourceWidth, uint32 sourceHeight, uint32 destX, uint32 destY, uint32 destWidth, uint32 destHeight, RENDERER_FRAMEBUFFER_BLIT_RESIZE_FILTER resizeFilter /*= RENDERER_FRAMEBUFFER_BLIT_RESIZE_FILTER_NEAREST*/)
{
    DebugAssert(m_nCurrentRenderTargets <= 1);

    // read source formats
    DebugAssert(static_cast<D3D11GPUTexture2D *>(pTexture)->GetDesc()->Flags & GPU_TEXTURE_FLAG_SHADER_BINDABLE);
    DXGI_FORMAT sourceTextureFormat = D3D11TypeConversion::PixelFormatToDXGIFormat(static_cast<D3D11GPUTexture2D *>(pTexture)->GetDesc()->Format);
    ID3D11Resource *pSourceResource = static_cast<D3D11GPUTexture2D *>(pTexture)->GetD3DTexture();
    uint32 sourceTextureWidth = static_cast<D3D11GPUTexture2D *>(pTexture)->GetDesc()->Width;
    uint32 sourceTextureHeight = static_cast<D3D11GPUTexture2D *>(pTexture)->GetDesc()->Height;

    // read destination format
    DXGI_FORMAT destinationTextureFormat = DXGI_FORMAT_UNKNOWN;
    ID3D11Resource *pDestinationResource = NULL;
    uint32 destinationTextureWidth = 0, destinationTextureHeight = 0;
    if (m_nCurrentRenderTargets == 0)
    {
        destinationTextureFormat = static_cast<D3D11GPUOutputBuffer *>(m_pCurrentSwapChain)->GetBackBufferFormat();
        pDestinationResource = static_cast<D3D11GPUOutputBuffer *>(m_pCurrentSwapChain)->GetBackBufferTexture();
        destinationTextureWidth = static_cast<D3D11GPUOutputBuffer *>(m_pCurrentSwapChain)->GetWidth();
        destinationTextureHeight = static_cast<D3D11GPUOutputBuffer *>(m_pCurrentSwapChain)->GetHeight();
    }
    else
    {
        switch (m_pCurrentRenderTargetViews[0]->GetTargetTexture()->GetTextureType())
        {
        case TEXTURE_TYPE_2D:
            destinationTextureFormat = D3D11TypeConversion::PixelFormatToDXGIFormat(static_cast<D3D11GPUTexture2D *>(m_pCurrentRenderTargetViews[0]->GetTargetTexture())->GetDesc()->Format);
            pDestinationResource = static_cast<D3D11GPUTexture2D *>(m_pCurrentRenderTargetViews[0]->GetTargetTexture())->GetD3DTexture();
            destinationTextureWidth = static_cast<D3D11GPUTexture2D *>(m_pCurrentRenderTargetViews[0]->GetTargetTexture())->GetDesc()->Width;
            destinationTextureHeight = static_cast<D3D11GPUTexture2D *>(m_pCurrentRenderTargetViews[0]->GetTargetTexture())->GetDesc()->Height;
            break;

        default:
            Panic("D3D11GPUContext::BlitFrameBuffer: Destination is an unsupported texture type");
            return;
        }
    }

    // d3d11 can do a direct copy between identical types
    if (sourceTextureFormat == destinationTextureFormat && sourceWidth == destWidth && sourceHeight == destHeight)
    {
        // whole texture?
        if (sourceX == 0 && sourceY == 0 && destX == 0 && destY == 0 && sourceWidth == sourceTextureWidth && sourceHeight == sourceTextureHeight)
        {
            // use CopyResource
            m_pD3DContext->CopyResource(pDestinationResource, pSourceResource);
            return;
        }
        else
        {
            // use CopySubResourceRegion
            D3D11_BOX sourceBox = { sourceX, sourceY, 0, sourceX + sourceWidth, sourceY + sourceHeight, 1 };
            m_pD3DContext->CopySubresourceRegion(pDestinationResource, 0, destX, destY, 0, pSourceResource, 0, &sourceBox);
            return;
        }
    }

    // TODO: use shader
    Panic("Shader framebufer blit");
}

void D3D11GPUContext::GenerateMips(GPUTexture *pTexture)
{
    ID3D11ShaderResourceView *pSRV = nullptr;
    uint32 flags = 0;
    switch (pTexture->GetTextureType())
    {
    case TEXTURE_TYPE_1D:
        pSRV = static_cast<D3D11GPUTexture1D *>(pTexture)->GetD3DSRV();
        flags = static_cast<D3D11GPUTexture1D *>(pTexture)->GetDesc()->Flags;
        break;

    case TEXTURE_TYPE_1D_ARRAY:
        pSRV = static_cast<D3D11GPUTexture1DArray *>(pTexture)->GetD3DSRV();
        flags = static_cast<D3D11GPUTexture1DArray *>(pTexture)->GetDesc()->Flags;
        break;

    case TEXTURE_TYPE_2D:
        pSRV = static_cast<D3D11GPUTexture2D *>(pTexture)->GetD3DSRV();
        flags = static_cast<D3D11GPUTexture2D *>(pTexture)->GetDesc()->Flags;
        break;

    case TEXTURE_TYPE_2D_ARRAY:
        pSRV = static_cast<D3D11GPUTexture2DArray *>(pTexture)->GetD3DSRV();
        flags = static_cast<D3D11GPUTexture2DArray *>(pTexture)->GetDesc()->Flags;
        break;

    case TEXTURE_TYPE_3D:
        pSRV = static_cast<D3D11GPUTexture3D *>(pTexture)->GetD3DSRV();
        flags = static_cast<D3D11GPUTexture3D *>(pTexture)->GetDesc()->Flags;
        break;

    case TEXTURE_TYPE_CUBE:
        pSRV = static_cast<D3D11GPUTextureCube *>(pTexture)->GetD3DSRV();
        flags = static_cast<D3D11GPUTextureCube *>(pTexture)->GetDesc()->Flags;
        break;

    case TEXTURE_TYPE_CUBE_ARRAY:
        pSRV = static_cast<D3D11GPUTextureCubeArray *>(pTexture)->GetD3DSRV();
        flags = static_cast<D3D11GPUTextureCubeArray *>(pTexture)->GetDesc()->Flags;
        break;
    }

    if (pSRV == nullptr || !(flags & GPU_TEXTURE_FLAG_GENERATE_MIPS))
    {
        Log_ErrorPrintf("D3D11GPUContext::GenerateMips: Texture not created with GPU_TEXTURE_FLAG_GENERATE_MIPS.");
        return;
    }

    m_pD3DContext->GenerateMips(pSRV);
}

GPUCommandList *D3D11GPUContext::CreateCommandList()
{
    return nullptr;
}

bool D3D11GPUContext::OpenCommandList(GPUCommandList *pCommandList)
{
    return false;
}

bool D3D11GPUContext::CloseCommandList(GPUCommandList *pCommandList)
{
    return false;
}

void D3D11GPUContext::ExecuteCommandList(GPUCommandList *pCommandList)
{
    Panic("Not available.");
}


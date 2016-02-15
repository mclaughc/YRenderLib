#include <SDL.h>
#include "YBaseLib/AutoReleasePtr.h"
#include "YBaseLib/ByteStream.h"
#include "YBaseLib/Log.h"
#include "YBaseLib/StringConverter.h"
#include "YBaseLib/Timer.h"
#include "YRenderLib/ImGui/ImGuiBridge.h"
#include "YRenderLib/Renderer.h"
#include "YRenderLib/ShaderCompiler/ShaderCompiler.h"
Log_SetChannel(ImGuiBridge);

static GPUDevice* s_pGPUDevice = nullptr;
static GPUContext* s_pGPUContext = nullptr;
static GPURasterizerState* s_pRasterizerState = nullptr;
static GPUDepthStencilState* s_pDepthStencilState = nullptr;
static GPUBlendState* s_pBlendState = nullptr;
static GPUInputLayout* s_pInputLayout = nullptr;
static GPUShaderProgram* s_pShaderProgram = nullptr;
static GPUBuffer* s_pVertexBuffer = nullptr;
static GPUBuffer* s_pIndexBuffer = nullptr;
static uint32 s_vertexBufferSize = 0;
static uint32 s_indexBufferSize = 0;
static bool s_supportsBaseVertex = false;
static Timer s_lastFrameTime;

static void RenderDrawListsCallback(ImDrawData *pDrawData)
{
    // check buffer size
    if ((uint32)pDrawData->TotalVtxCount > s_vertexBufferSize)
    {
        uint32 newVertexCount = Max((s_vertexBufferSize != 0) ? (s_vertexBufferSize * 2) : 1024, (uint32)pDrawData->TotalVtxCount);
        Log_PerfPrintf("Reallocating ImGui vertex buffer, new count = %u (%s)", newVertexCount, StringConverter::SizeToHumanReadableString(newVertexCount * sizeof(ImDrawVert)).GetCharArray());

        GPU_BUFFER_DESC bufferDesc(GPU_BUFFER_FLAG_BIND_VERTEX_BUFFER, newVertexCount * sizeof(ImDrawVert));
        if (s_pGPUDevice->GetFeatureLevel() >= RENDERER_FEATURE_LEVEL_ES3)
            bufferDesc.Flags |= GPU_BUFFER_FLAG_MAPPABLE;
        else
            bufferDesc.Flags |= GPU_BUFFER_FLAG_WRITABLE;

        GPUBuffer *pNewVertexBuffer = s_pGPUDevice->CreateBuffer(&bufferDesc, nullptr);
        if (pNewVertexBuffer == nullptr)
        {
            Log_ErrorPrint("Failed to allocate ImGui vertex buffer.");
            return;
        }

        if (s_pVertexBuffer != nullptr)
            s_pVertexBuffer->Release();

        s_pVertexBuffer = pNewVertexBuffer;
        s_vertexBufferSize = newVertexCount;
    }
    if ((uint32)pDrawData->TotalIdxCount > s_indexBufferSize)
    {
        uint32 newIndexCount = Max((s_indexBufferSize != 0) ? (s_indexBufferSize * 2) : 1024, (uint32)pDrawData->TotalIdxCount);
        Log_PerfPrintf("Reallocating ImGui index buffer, new count = %u (%s)", newIndexCount, StringConverter::SizeToHumanReadableString(newIndexCount * sizeof(ImDrawIdx)).GetCharArray());

        GPU_BUFFER_DESC bufferDesc(GPU_BUFFER_FLAG_BIND_INDEX_BUFFER, newIndexCount * sizeof(ImDrawIdx));
        if (s_pGPUDevice->GetFeatureLevel() >= RENDERER_FEATURE_LEVEL_ES3)
            bufferDesc.Flags |= GPU_BUFFER_FLAG_MAPPABLE;
        else
            bufferDesc.Flags |= GPU_BUFFER_FLAG_WRITABLE;

        GPUBuffer *pNewIndexBuffer = s_pGPUDevice->CreateBuffer(&bufferDesc, nullptr);
        if (pNewIndexBuffer == nullptr)
        {
            Log_ErrorPrint("Failed to allocate ImGui index buffer.");
            return;
        }

        if (s_pIndexBuffer != nullptr)
            s_pIndexBuffer->Release();

        s_pIndexBuffer = pNewIndexBuffer;
        s_indexBufferSize = newIndexCount;
    }

    // write to buffers
    if (s_pGPUDevice->GetFeatureLevel() >= RENDERER_FEATURE_LEVEL_ES3)
    {
        ImDrawVert *pMappedVertexBuffer;
        if (!s_pGPUContext->MapBuffer(s_pVertexBuffer, GPU_MAP_TYPE_WRITE_DISCARD, reinterpret_cast<void **>(&pMappedVertexBuffer)))
        {
            Log_ErrorPrint("Failed to map ImGui vertex buffer");
            return;
        }

        ImDrawIdx *pMappedIndexBuffer;
        if (!s_pGPUContext->MapBuffer(s_pIndexBuffer, GPU_MAP_TYPE_WRITE_DISCARD, reinterpret_cast<void **>(&pMappedIndexBuffer)))
        {
            s_pGPUContext->Unmapbuffer(s_pVertexBuffer, pMappedVertexBuffer);
            Log_ErrorPrint("Failed to map ImGui index buffer");
            return;
        }

        // copy vertices in
        ImDrawVert *pCurrentVertex = pMappedVertexBuffer;
        ImDrawIdx *pCurrentIndex = pMappedIndexBuffer;
        for (int i = 0; i < pDrawData->CmdListsCount; i++)
        {
            Y_memcpy(pCurrentVertex, pDrawData->CmdLists[i]->VtxBuffer.Data, pDrawData->CmdLists[i]->VtxBuffer.size() * sizeof(ImDrawVert));
            Y_memcpy(pCurrentIndex, pDrawData->CmdLists[i]->IdxBuffer.Data, pDrawData->CmdLists[i]->IdxBuffer.size() * sizeof(ImDrawIdx));
            pCurrentVertex += pDrawData->CmdLists[i]->VtxBuffer.size();
            pCurrentIndex += pDrawData->CmdLists[i]->IdxBuffer.size();
        }

        // unmap again
        s_pGPUContext->Unmapbuffer(s_pIndexBuffer, pMappedIndexBuffer);
        s_pGPUContext->Unmapbuffer(s_pVertexBuffer, pMappedVertexBuffer);
    }
    else
    {
        // annoyingly, ES2 doesn't have the ability to map buffers
        uint32 vertexBufferOffset = 0;
        uint32 indexBufferOffset = 0;
        for (int i = 0; i < pDrawData->CmdListsCount; i++)
        {
            s_pGPUContext->WriteBuffer(s_pVertexBuffer, pDrawData->CmdLists[i]->VtxBuffer.Data, vertexBufferOffset, pDrawData->CmdLists[i]->VtxBuffer.size() * sizeof(ImDrawVert));
            s_pGPUContext->WriteBuffer(s_pIndexBuffer, pDrawData->CmdLists[i]->IdxBuffer.Data, indexBufferOffset, pDrawData->CmdLists[i]->IdxBuffer.size() * sizeof(ImDrawIdx));
            vertexBufferOffset += pDrawData->CmdLists[i]->VtxBuffer.size() * sizeof(ImDrawVert);
            indexBufferOffset += pDrawData->CmdLists[i]->IdxBuffer.size() * sizeof(ImDrawIdx);
        }
    }

    // set up device
    s_pGPUContext->SetRasterizerState(s_pRasterizerState);
    s_pGPUContext->SetDepthStencilState(s_pDepthStencilState, 0);
    s_pGPUContext->SetBlendState(s_pBlendState);

    // load shader
    s_pGPUContext->SetInputLayout(s_pInputLayout);
    s_pGPUContext->SetShaderProgram(s_pShaderProgram);
    s_pGPUContext->SetDrawTopology(DRAW_TOPOLOGY_TRIANGLE_LIST);

    // set buffers
    s_pGPUContext->SetVertexBuffer(0, s_pVertexBuffer, 0, sizeof(ImDrawVert));
    s_pGPUContext->SetIndexBuffer(s_pIndexBuffer, GPU_INDEX_FORMAT_UINT16, 0);

    // update screen size uniform
    const RENDERER_VIEWPORT* pViewport = s_pGPUContext->GetViewport();
    float inverseViewportSize[2] = { 1.0f / (float)pViewport->Width, 1.0f / (float)pViewport->Height };
    s_pShaderProgram->SetUniform(0, SHADER_PARAMETER_TYPE_FLOAT2, inverseViewportSize);

    // draw commands
    unsigned int baseVertex = 0;
    unsigned int baseIndex = 0;
    for (int i = 0; i < pDrawData->CmdListsCount; i++)
    {
        ImDrawList *pCmdList = pDrawData->CmdLists[i];
        unsigned int firstIndex = 0;

        for (int j = 0; j < pCmdList->CmdBuffer.size(); j++)
        {
            const ImDrawCmd *pCmd = &pCmdList->CmdBuffer[j];
            if (pCmd->UserCallback != nullptr)
            {
                pCmd->UserCallback(pCmdList, pCmd);
                continue;
            }

            // set up clip rect
            RENDERER_SCISSOR_RECT scissorRect((uint32)pCmd->ClipRect.x, (uint32)pCmd->ClipRect.y, (uint32)pCmd->ClipRect.z, (uint32)pCmd->ClipRect.w);
            s_pGPUContext->SetScissorRect(&scissorRect);

            // bind texture
            s_pGPUContext->SetShaderResource(0, reinterpret_cast<GPUTexture2D *>(pCmd->TextureId));

            // adjust buffer pointer
            if (!s_supportsBaseVertex && baseVertex > 0)
            {
                s_pGPUContext->SetVertexBuffer(0, s_pVertexBuffer, sizeof(ImDrawVert) * baseVertex, sizeof(ImDrawVert));
                s_pGPUContext->SetIndexBuffer(s_pIndexBuffer, GPU_INDEX_FORMAT_UINT16, sizeof(ImDrawIdx) * baseIndex);
                s_pGPUContext->DrawIndexed(firstIndex, pCmd->ElemCount, 0);
            }
            else
            {
                s_pGPUContext->DrawIndexed(baseIndex + firstIndex, pCmd->ElemCount, baseVertex);
            }

            // update pointers
            baseIndex += pCmd->ElemCount;
        }

        baseVertex += pCmdList->VtxBuffer.size();
    }

    // clear bindings
    s_pGPUContext->ClearState(false, true, false, false);
}

static bool CreateStates()
{
    RENDERER_RASTERIZER_STATE_DESC rasterizerStateDesc(RENDERER_FILL_SOLID, RENDERER_CULL_NONE, true, true, 0, 0.0f, false);
    s_pRasterizerState = s_pGPUDevice->CreateRasterizerState(&rasterizerStateDesc);
    if (s_pRasterizerState == nullptr)
    {
        Log_ErrorPrintf("Failed to create rasterizer state");
        return false;
    }

    RENDERER_DEPTHSTENCIL_STATE_DESC depthStencilStateDesc(false, false, GPU_COMPARISON_FUNC_ALWAYS);
    s_pDepthStencilState = s_pGPUDevice->CreateDepthStencilState(&depthStencilStateDesc);
    if (s_pDepthStencilState == nullptr)
    {
        Log_ErrorPrintf("Failed to create depth state");
        return false;
    }

    RENDERER_BLEND_STATE_DESC blendStateDesc(true, RENDERER_BLEND_SRC_ALPHA, RENDERER_BLEND_OP_ADD, RENDERER_BLEND_INV_SRC_ALPHA, RENDERER_BLEND_ONE, RENDERER_BLEND_OP_ADD, RENDERER_BLEND_ZERO, true);
    s_pBlendState = s_pGPUDevice->CreateBlendState(&blendStateDesc);
    if (s_pBlendState == nullptr)
    {
        Log_ErrorPrintf("Failed to create blend state");
        return false;
    }

    return true;
}

static bool CreateShaderProgram()
{
    static const char* vertexShaderSource = R"(
        float2 inv_vp_size : register(c0);

        void main(in float2 ipos : POSITION,
                  in float2 iuv : TEXCOORD,
                  in float4 icol : COLOR,
                  out float2 ouv : TEXCOORD,
                  out float4 ocol : COLOR,
                  out float4 opos : SV_Position)
        {
            ouv = iuv;
            ocol = icol;

            // create clipspace coordinates
            float2 cs_pos = ipos * inv_vp_size;     // ipos / vp_size
            cs_pos.x = cs_pos.x * 2.0f - 1.0f;
            cs_pos.y = (1.0f - cs_pos.y) * 2.0f - 1.0f;
            opos = float4(cs_pos, 0.0f, 1.0f);
        }
    )";

    static const char* pixelShaderSource = R"(
        Texture2D fonttex : register(t0);
        SamplerState fonttex_SamplerState : register(s0);

        void main(in float2 iuv : TEXCOORD,
                  in float4 icol : COLOR,
                  out float4 ocol : SV_Target)
        {
            ocol = fonttex.Sample(fonttex_SamplerState, iuv) * icol;
        }
    )";

    AutoReleasePtr<ShaderCompiler> pShaderCompiler = ShaderCompiler::Create();
    pShaderCompiler->SetStageSourceCode(SHADER_PROGRAM_STAGE_VERTEX_SHADER, "", vertexShaderSource, "main");
    pShaderCompiler->SetStageSourceCode(SHADER_PROGRAM_STAGE_PIXEL_SHADER, "", pixelShaderSource, "main");

    AutoReleasePtr<ByteStream> pShaderBlob = ByteStream_CreateGrowableMemoryStream();
    if (!pShaderCompiler->CompileSingleTypeProgram(s_pGPUDevice->GetShaderProgramType(), 0, pShaderBlob, nullptr, nullptr))
    {
        Log_ErrorPrintf("Failed to compile ImGui program");
        return false;
    }

    pShaderBlob->SeekAbsolute(0);
    s_pShaderProgram = s_pGPUDevice->CreateGraphicsProgram(pShaderBlob);
    if (s_pShaderProgram == nullptr)
    {
        Log_ErrorPrintf("Failed to create ImGui program");
        return false;
    }

    return true;
}

static bool CreateInputLayout()
{
    static const GPU_VERTEX_ELEMENT_DESC vertexElements[] = {
        { GPU_VERTEX_ELEMENT_SEMANTIC_POSITION, 0, GPU_VERTEX_ELEMENT_TYPE_FLOAT2, 0, offsetof(ImDrawVert, pos), 0 },
        { GPU_VERTEX_ELEMENT_SEMANTIC_TEXCOORD, 0, GPU_VERTEX_ELEMENT_TYPE_FLOAT2, 0, offsetof(ImDrawVert, uv), 0 },
        { GPU_VERTEX_ELEMENT_SEMANTIC_COLOR, 0, GPU_VERTEX_ELEMENT_TYPE_UNORM4, 0, offsetof(ImDrawVert, col), 0 }
    };

    s_pInputLayout = s_pGPUDevice->CreateInputLayout(vertexElements, countof(vertexElements));
    if (s_pInputLayout == nullptr)
    {
        Log_ErrorPrintf("Failed to create input layout");
        return false;
    }

    return true;
}

static bool CreateTextures()
{
    ImGuiIO& io = ImGui::GetIO();

    // get font pixels
    const void *pFontPixels;
    int fontWidth, fontHeight;
    uint32 pitch;
    io.Fonts->GetTexDataAsRGBA32((unsigned char **)&pFontPixels, &fontWidth, &fontHeight);
    pitch = PixelFormat_CalculateRowPitch(PIXEL_FORMAT_R8G8B8A8_UNORM, fontWidth);

    // Create the font as a GPU texture
    GPU_TEXTURE2D_DESC textureDesc(fontWidth, fontHeight, PIXEL_FORMAT_R8G8B8A8_UNORM, GPU_TEXTURE_FLAG_SHADER_BINDABLE, 1);
    GPU_SAMPLER_STATE_DESC samplerStateDesc(TEXTURE_FILTER_MIN_MAG_LINEAR_MIP_POINT, TEXTURE_ADDRESS_MODE_CLAMP, TEXTURE_ADDRESS_MODE_CLAMP, TEXTURE_ADDRESS_MODE_CLAMP, FloatColor::Black, 0.0f, 0, 0, 1, GPU_COMPARISON_FUNC_NEVER);
    GPUTexture2D *pFontTexture = s_pGPUDevice->CreateTexture2D(&textureDesc, &samplerStateDesc, &pFontPixels, &pitch);
    if (pFontTexture == nullptr)
    {
        Log_ErrorPrintf("Failed to create font texture.");
        return false;
    }

    io.Fonts->TexID = pFontTexture;
    return true;
}

static void PopulateKeyMap()
{
    ImGuiIO& io = ImGui::GetIO();

    io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
    io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
    io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
    io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
    io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
    io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
    io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;
}

bool ImGuiBridge::Initialize(GPUDevice* pGPUDevice, GPUContext* pGPUContext)
{
    GPUOutputBuffer *pOutputBuffer = pGPUContext->GetOutputBuffer();

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = (float)pOutputBuffer->GetWidth();
    io.DisplaySize.y = (float)pOutputBuffer->GetHeight();
    io.DeltaTime = 0.0f;
    io.RenderDrawListsFn = RenderDrawListsCallback;
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    // set vars
    s_pGPUDevice = AddRefAndReturn(pGPUDevice);
    s_pGPUContext = AddRefAndReturn(pGPUContext);

    // get caps
    RendererCapabilities capabilities;
    pGPUDevice->GetCapabilities(&capabilities);
    s_supportsBaseVertex = capabilities.SupportsDrawBaseVertex;

    // create gpu objects
    if (!CreateStates() ||
        !CreateInputLayout() ||
        !CreateShaderProgram() ||
        !CreateTextures())
    {
        FreeResources();
        return false;
    }

    // initialize keyboard map
    PopulateKeyMap();

    // done
    return true;
}

void ImGuiBridge::Shutdown()
{
    FreeResources();

    ImGui::Shutdown();
}

void ImGuiBridge::SetDisplaySize(uint32 width, uint32 height)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = (float)width;
    io.DisplaySize.y = (float)height;
}

void ImGuiBridge::NewFrame(float deltaTime)
{
    ImGuiIO& io = ImGui::GetIO();

    // update delta time
    io.DeltaTime = deltaTime;

    // update keyboard modifiers
    SDL_Keymod currentModifiers = SDL_GetModState();
    io.KeyAlt = (currentModifiers & (KMOD_LALT | KMOD_RALT)) != 0;
    io.KeyCtrl = (currentModifiers & (KMOD_LCTRL | KMOD_RCTRL)) != 0;
    io.KeyShift = (currentModifiers & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0;

    // fall through
    ImGui::NewFrame();
}

void ImGuiBridge::NewFrame()
{
    float deltaTime = (float)s_lastFrameTime.GetTimeSeconds();
    s_lastFrameTime.Reset();

    NewFrame(deltaTime);
}

bool ImGuiBridge::HandleSDLEvent(const SDL_Event *pEvent, bool forceCapture /* = false */)
{
    ImGuiIO& io = ImGui::GetIO();
    bool captureKeyboardEvents = io.WantCaptureKeyboard || forceCapture;
    bool captureMouseEvents = io.WantCaptureMouse || forceCapture;


    // keyboard events
    switch (pEvent->type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        {
            if (pEvent->key.keysym.scancode < countof(io.KeysDown))
                io.KeysDown[pEvent->key.keysym.scancode] = (pEvent->type == SDL_KEYDOWN);

            return captureKeyboardEvents;
        }

    case SDL_TEXTINPUT:
        {
            // todo: utf-8 to utf-16
            size_t length = Y_strlen(pEvent->text.text);
            for (uint32 i = 0; i < length; i++)
                io.AddInputCharacter((ImWchar)pEvent->text.text[i]);

            return captureKeyboardEvents;
        }
    }

    // mouse events
    switch (pEvent->type)
    {
    case SDL_MOUSEMOTION:
        {
            io.MousePos.x = (float)pEvent->motion.x;
            io.MousePos.y = (float)pEvent->motion.y;
            return captureMouseEvents;
        }

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        {
            static const uint32 buttonMapping[5] = { 0, 0, 2, 1, 3 };
            if (pEvent->button.button < countof(buttonMapping))
                io.MouseDown[buttonMapping[pEvent->button.button]] = (pEvent->type == SDL_MOUSEBUTTONDOWN);

            return captureMouseEvents;
        }

    case SDL_MOUSEWHEEL:
        io.MouseWheel = (float)pEvent->wheel.y;
        return captureMouseEvents;
    }

    return false;
}

void ImGuiBridge::FreeResources()
{
    // release font
    ImGuiIO& io = ImGui::GetIO();
    if (io.Fonts->TexID != nullptr)
    {
        GPUTexture2D *pTexture = reinterpret_cast<GPUTexture2D *>(io.Fonts->TexID);
        pTexture->Release();
        io.Fonts->TexID = nullptr;
    }

    // release resources
    s_vertexBufferSize = 0;
    s_indexBufferSize = 0;
    SAFE_RELEASE(s_pRasterizerState);
    SAFE_RELEASE(s_pDepthStencilState);
    SAFE_RELEASE(s_pBlendState);
    SAFE_RELEASE(s_pInputLayout);
    SAFE_RELEASE(s_pShaderProgram);
    SAFE_RELEASE(s_pVertexBuffer);
    SAFE_RELEASE(s_pIndexBuffer);
    SAFE_RELEASE(s_pGPUContext);
    SAFE_RELEASE(s_pGPUDevice);
}

#include <SDL.h>
#include "YBaseLib/ByteStream.h"
#include "YBaseLib/Log.h"
#include "YRenderLib/Renderer.h"
#include "YRenderLib/ShaderCompiler/ShaderCompiler.h"

static const char* vs_source = R"(
    void main(in float4 ipos : POSITION, in float4 icolor : COLOR,
              out float4 ocolor : COLOR, out float4 opos : SV_Position)
    {
        ocolor = icolor;
        opos = ipos;
    }
)";

static const char* ps_source = R"(
    void main(in float4 icolor : COLOR, out float4 ocolor : SV_Target)
    {
        ocolor = icolor;
    }
)";

static const float vertex_data[][2][4] = {
    { { 0.0f, 0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
    { { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
    { { 0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
};

static const GPU_VERTEX_ELEMENT_DESC vertex_layout[] = {
    { GPU_VERTEX_ELEMENT_SEMANTIC_POSITION, 0, GPU_VERTEX_ELEMENT_TYPE_FLOAT4, 0, 0, 0 },
    { GPU_VERTEX_ELEMENT_SEMANTIC_COLOR, 0, GPU_VERTEX_ELEMENT_TYPE_FLOAT4, 0, sizeof(float) * 4, 0 },
};

int main(int argc, char* argv[])
{
    Log::GetInstance().SetConsoleOutputParams(true);

    RendererInitializationParameters params;

    GPUDevice* pDevice;
    GPUContext* pContext;
    RendererOutputWindow* pWindow;

    if (!RenderLib::CreateRenderDeviceAndWindow(&params, &pDevice, &pContext, &pWindow))
        Panic("Failed create window");

    ShaderCompiler* pShaderCompiler = ShaderCompiler::Create();
    pShaderCompiler->SetStageSourceCode(SHADER_PROGRAM_STAGE_VERTEX_SHADER, "file.hlsl", vs_source, "main");
    pShaderCompiler->SetStageSourceCode(SHADER_PROGRAM_STAGE_PIXEL_SHADER, "file.hlsl", ps_source, "main");

    ByteStream* pShaderBlob = ByteStream_CreateGrowableMemoryStream();
    if (!pShaderCompiler->CompileSingleTypeProgram(pDevice->GetShaderProgramType(), 0, pShaderBlob, nullptr, nullptr))
        Panic("Failed to compile program");

    pShaderBlob->SeekAbsolute(0);
    GPUShaderProgram* pShaderProgram = pDevice->CreateGraphicsProgram(pShaderBlob);
    if (!pShaderProgram)
        Panic("Failed to create program");

    GPU_BUFFER_DESC bufferDesc(GPU_BUFFER_FLAG_BIND_VERTEX_BUFFER, sizeof(vertex_data));
    GPUBuffer* pVertexBuffer = pDevice->CreateBuffer(&bufferDesc, vertex_data);
    if (!pVertexBuffer)
        Panic("Failed to create vertex buffer");

    GPUInputLayout* pInputLayout = pDevice->CreateInputLayout(vertex_layout, countof(vertex_layout));
    if (!pInputLayout)
        Panic("Failed to create input layout");

    while (!pWindow->IsClosed())
    {
        pContext->BeginFrame();
        pContext->SetFullViewport();
        pContext->ClearTargets(true, true, false, FloatColor(0.5f, 0.2f, 0.1f, 1.0f));
        pContext->SetInputLayout(pInputLayout);
        pContext->SetShaderProgram(pShaderProgram);
        pContext->SetVertexBuffer(0, pVertexBuffer, 0, sizeof(vertex_data[0]));
        pContext->SetDrawTopology(DRAW_TOPOLOGY_TRIANGLE_LIST);
        pContext->Draw(0, 3);
        pContext->ClearState();
        pContext->PresentOutputBuffer(GPU_PRESENT_BEHAVIOUR_IMMEDIATE);

        SDL_PumpEvents();

        pWindow->HandleMessages(pContext);
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    }

    pShaderCompiler->Release();
    pShaderBlob->Release();
    pShaderProgram->Release();
    pInputLayout->Release();
    pVertexBuffer->Release();

    pContext->Release();
    pWindow->Release();
    pDevice->Release();
    return 0;
}

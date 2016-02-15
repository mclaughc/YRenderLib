#include <SDL.h>
#include "YBaseLib/Log.h"
#include "YRenderLib/Renderer.h"

int main(int argc, char* argv[])
{
    Log::GetInstance().SetConsoleOutputParams(true);

    RendererInitializationParameters params;

    GPUDevice* pDevice;
    GPUContext* pContext;
    RendererOutputWindow* pWindow;

    if (!RenderLib::CreateRenderDeviceAndWindow(&params, &pDevice, &pContext, &pWindow))
        Panic("Failed create window");

    while (pWindow->IsVisible())
    {
        pContext->BeginFrame();
        pContext->ClearTargets(true, false, false, FloatColor(0.5f, 0.2f, 0.1f, 1.0f));
        pContext->PresentOutputBuffer(GPU_PRESENT_BEHAVIOUR_IMMEDIATE);

        SDL_PumpEvents();
    }

    pDevice->Release();
    pContext->Release();
    pWindow->Release();
    return 0;
}

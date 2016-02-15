#include <SDL.h>
#include "YBaseLib/Log.h"
#include "YRenderLib/ImGui/ImGuiBridge.h"
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

    if (!ImGuiBridge::Initialize(pDevice, pContext))
        Panic("Failed to initialize imgui bridge");
    
    while (!pWindow->IsClosed())
    {
        ImGuiBridge::NewFrame();
        pContext->BeginFrame();

        SDL_PumpEvents();

        // check for resizes
        if (pWindow->HandleMessages(pContext))
        {
            ImGuiBridge::SetDisplaySize(pWindow->GetWidth(), pWindow->GetHeight());
            pContext->SetFullViewport();
        }

        // handle events
        for (;;)
        {
            SDL_Event events[100];
            int nEvents = SDL_PeepEvents(events, countof(events), SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
            for (int i = 0; i < nEvents; i++)
                ImGuiBridge::HandleSDLEvent(&events[i], false);

            if (nEvents == 0)
                break;
        }

        pContext->ClearTargets(true, true, false, FloatColor::Black);

        ImGui::ShowTestWindow();
        ImGui::Render();

        pContext->ClearState();
        pContext->PresentOutputBuffer(GPU_PRESENT_BEHAVIOUR_IMMEDIATE);
    }

    ImGuiBridge::Shutdown();

    pContext->Release();
    pWindow->Release();
    pDevice->Release();
    return 0;
}

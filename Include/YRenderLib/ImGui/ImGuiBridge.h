#pragma once
#include "YRenderLib/Common.h"

// We don't currently have any overrides for imgui
#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#include "YRenderLib/ImGui/imgui.h"

union SDL_Event;
class GPUDevice;
class GPUContext;

namespace ImGuiBridge
{
    // Ready engine and imgui for drawing
    bool Initialize(GPUDevice* pGPUDevice, GPUContext* pGPUContext);

    // Shutdown bridge and imgui
    void Shutdown();

    // Alter the viewport dimensions
    void SetDisplaySize(uint32 width, uint32 height);

    // New frame helper, handles modifiers/delta time
    void NewFrame();

    // On new frame, pass delta-time
    void NewFrame(float deltaTime);

    // Process a SDL event
    bool HandleSDLEvent(const SDL_Event *pEvent, bool forceCapture = false);

    // Release everything
    void FreeResources();
}

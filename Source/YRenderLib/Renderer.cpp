#include <SDL.h>
#include "YBaseLib/Assert.h"
#include "YBaseLib/Log.h"
#include "YBaseLib/NumericLimits.h"
#include "YRenderLib/Common.h"
#include "YRenderLib/Renderer.h"
#include "YRenderLib/RendererTypes.h"
Log_SetChannel(Renderer);

// fix up a warning
#ifdef SDL_VIDEO_DRIVER_WINDOWS
    #undef WIN32_LEAN_AND_MEAN
#endif
#include <SDL/SDL_syswm.h>

#define WITH_RENDERER_D3D11
#define WITH_RENDERER_VULKAN

//----------------------------------------------------- RenderSystem Creation Functions -----------------------------------------------------------------------------------------------
// renderer creation functions
typedef bool(*RendererFactoryFunction)(const RendererInitializationParameters *pCreateParameters, SDL_Window *pSDLWindow, GPUDevice **ppDevice, GPUContext **ppImmediateContext, GPUOutputBuffer **ppOutputBuffer);
#if defined(WITH_RENDERER_D3D11)
    extern bool D3D11RenderBackend_Create(const RendererInitializationParameters *pCreateParameters, SDL_Window *pSDLWindow, GPUDevice **ppDevice, GPUContext **ppImmediateContext, GPUOutputBuffer **ppOutputBuffer);
#endif
#if defined(WITH_RENDERER_D3D12)
    extern bool D3D12RenderBackend_Create(const RendererInitializationParameters *pCreateParameters, SDL_Window *pSDLWindow, GPUDevice **ppDevice, GPUContext **ppImmediateContext, GPUOutputBuffer **ppOutputBuffer);
#endif
#if defined(WITH_RENDERER_OPENGL)
    extern bool OpenGLRenderBackend_Create(const RendererInitializationParameters *pCreateParameters, SDL_Window *pSDLWindow, GPUDevice **ppDevice, GPUContext **ppImmediateContext, GPUOutputBuffer **ppOutputBuffer);
#endif
#if defined(WITH_RENDERER_OPENGLES2)
    extern bool OpenGLES2RenderBackend_Create(const RendererInitializationParameters *pCreateParameters, SDL_Window *pSDLWindow, GPUDevice **ppDevice, GPUContext **ppImmediateContext, GPUOutputBuffer **ppOutputBuffer);
#endif
#if defined(WITH_RENDERER_VULKAN)
    extern bool VulkanBackend_Create(const RendererInitializationParameters *pCreateParameters, SDL_Window *pSDLWindow, GPUDevice **ppDevice, GPUContext **ppImmediateContext, GPUOutputBuffer **ppOutputBuffer);
#endif
struct RENDERER_PLATFORM_FACTORY_FUNCTION
{
    RENDERER_PLATFORM Platform;
    RendererFactoryFunction Function;
    bool RequiresImplicitSwapChain;
};

static const RENDERER_PLATFORM_FACTORY_FUNCTION s_renderSystemDeclarations[] =
{
#if defined(WITH_RENDERER_D3D11)
    { RENDERER_PLATFORM_D3D11,      D3D11RenderBackend_Create,      false   },
#endif
#if defined(WITH_RENDERER_D3D12)
    { RENDERER_PLATFORM_D3D12,      D3D12RenderBackend_Create,      false   },
#endif
#if defined(WITH_RENDERER_OPENGL)
    { RENDERER_PLATFORM_OPENGL,     OpenGLRenderBackend_Create,     true    },
#endif
#if defined(WITH_RENDERER_OPENGLES2)
    { RENDERER_PLATFORM_OPENGLES2,  OpenGLES2RenderBackend_Create,  true    },
#endif
#if defined(WITH_RENDERER_VULKAN)
    { RENDERER_PLATFORM_VULKAN,     VulkanBackend_Create,           true    },
#endif
};

//----------------------------------------------------- Output Window Class ----------------------------------------------------------------------------------------------------------

RendererOutputWindow::RendererOutputWindow(SDL_Window *pSDLWindow, GPUOutputBuffer *pBuffer, RENDERER_FULLSCREEN_STATE fullscreenState)
    : m_pSDLWindow(pSDLWindow),
      m_pOutputBuffer(pBuffer),
      m_fullscreenState(fullscreenState),
      m_hasFocus(true),
      m_visible(true),
      m_closed(false),
      m_title(SDL_GetWindowTitle(pSDLWindow)),
      m_mouseGrabbed(false),
      m_mouseRelativeMovement(false)
{
    SDL_GetWindowPosition(m_pSDLWindow, &m_positionX, &m_positionY);
    SDL_GetWindowSize(m_pSDLWindow, reinterpret_cast<int *>(&m_width), reinterpret_cast<int *>(&m_height));
}

RendererOutputWindow::~RendererOutputWindow()
{
    // Buffer should no longer be used
    if (m_pOutputBuffer != nullptr)
    {
        uint32 bufferReferenceCount = m_pOutputBuffer->Release();
        Assert(bufferReferenceCount == 0);
    }

    // Destroy window itself
    SDL_DestroyWindow(m_pSDLWindow);
}

void RendererOutputWindow::SetWindowTitle(const char *title)
{
    m_title = title;
    SDL_SetWindowTitle(m_pSDLWindow, title);
}

void RendererOutputWindow::SetWindowVisibility(bool visible)
{
    m_visible = visible;
    if (visible)
        SDL_ShowWindow(m_pSDLWindow);
    else
        SDL_HideWindow(m_pSDLWindow);
}

void RendererOutputWindow::SetWindowPosition(int32 x, int32 y)
{
    SDL_SetWindowPosition(m_pSDLWindow, x, y);
    SDL_GetWindowPosition(m_pSDLWindow, &m_positionX, &m_positionY);
}

void RendererOutputWindow::SetWindowSize(uint32 width, uint32 height)
{
    SDL_SetWindowSize(m_pSDLWindow, width, height);
    SDL_GetWindowSize(m_pSDLWindow, reinterpret_cast<int *>(&m_width), reinterpret_cast<int *>(&m_height));
}

void RendererOutputWindow::SetMouseGrab(bool enabled)
{   
    SDL_SetWindowGrab(m_pSDLWindow, (enabled) ? SDL_TRUE : SDL_FALSE);
}

void RendererOutputWindow::SetMouseRelativeMovement(bool enabled)
{
    if (enabled)
    {
        SDL_RaiseWindow(m_pSDLWindow);
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }
    else
    {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
}

bool RendererOutputWindow::HandleMessages(GPUContext* pGPUContext)
{
#if 1
    struct Args
    {
        RendererOutputWindow* window;
        GPUContext* context;
        bool resized;
    };

    Args args = { this, pGPUContext, false };

    SDL_FilterEvents([](void* userdata, SDL_Event* event) -> int
    {
        if (event->type != SDL_WINDOWEVENT)
            return 1;

        Args* args = reinterpret_cast<Args*>(userdata);
        switch (event->window.event)
        {
        case SDL_WINDOWEVENT_CLOSE:
            args->window->m_closed = true;
            break;

        case SDL_WINDOWEVENT_MINIMIZED:
            args->window->m_visible = false;
            break;

        case SDL_WINDOWEVENT_RESTORED:
            args->window->m_visible = true;
            break;

        case SDL_WINDOWEVENT_SIZE_CHANGED:
            {
                uint32 width = event->window.data1;
                uint32 height = event->window.data2;

                GPUOutputBuffer* old_buffer = args->context->GetOutputBuffer();
                if (old_buffer != args->window->m_pOutputBuffer)
                    args->context->SetOutputBuffer(args->window->m_pOutputBuffer);

                args->context->ResizeOutputBuffer(width, height);
                args->window->m_width = width;
                args->window->m_height = height;

                if (old_buffer != args->window->m_pOutputBuffer)
                    args->context->SetOutputBuffer(old_buffer);

                args->resized = true;
            }
            break;
        }

        return 1;
    }, &args);

    return args.resized;

#else
    // doesn't work for large resizes
    // shouldn't really have more than a few messages, but in case.
    SDL_Event events[16];
    int nEvents = SDL_PeepEvents(events, countof(events), SDL_PEEKEVENT, SDL_WINDOWEVENT, SDL_WINDOWEVENT);
    bool resized = false;
    for (int i = 0; i < nEvents; i++)
    {
        const SDL_Event& event = events[i];
        DebugAssert(event.type == SDL_WINDOWEVENT);

        switch (event.window.event)
        {
        case SDL_WINDOWEVENT_CLOSE:
            m_closed = true;
            break;

        case SDL_WINDOWEVENT_MINIMIZED:
            m_visible = false;
            break;

        case SDL_WINDOWEVENT_RESTORED:
            m_visible = true;
            break;

        case SDL_WINDOWEVENT_SIZE_CHANGED:
            {
                uint32 width = event.window.data1;
                uint32 height = event.window.data2;

                pGPUContext->ResizeOutputBuffer(width, height);
                m_width = width;
                m_height = height;
                resized = true;
            }
            break;
        }
    }

    return resized;
#endif
}

//----------------------------------------------------- Init/Startup/Shutdown -----------------------------------------------------------------------------------------------------
#if defined(Y_PLATFORM_WINDOWS)
    static constexpr RENDERER_PLATFORM DEFAULT_RENDERER_PLATFORM = RENDERER_PLATFORM_D3D11;
#elif defined(Y_PLATFORM_LINUX) || defined(Y_PLATFORM_OSX)
    static constexpr RENDERER_PLATFORM DEFAULT_RENDERER_PLATFORM = RENDERER_PLATFORM_OPENGL;
#elif defined(Y_PLATFORM_ANDROID) || defined(Y_PLATFORM_IOS) || defined(Y_PLATFORM_HTML5)
    static constexpr RENDERER_PLATFORM DEFAULT_RENDERER_PLATFORM = RENDERER_PLATFORM_OPENGLES2;
#else
    static constexpr RENDERER_PLATFORM DEFAULT_RENDERER_PLATFORM = RENDERER_PLATFORM_OPENGL;
#endif
#ifdef Y_BUILD_CONFIG_DEBUG
    static constexpr bool DEFAULT_USE_DEBUG_DEVICE = true;
#else
    static constexpr bool DEFAULT_USE_DEBUG_DEVICE = false;
#endif

RendererInitializationParameters::RendererInitializationParameters()
    : Platform(DEFAULT_RENDERER_PLATFORM)
    , EnableThreadedRendering(true)
    , BackBufferFormat(PIXEL_FORMAT_R8G8B8A8_UNORM)
    , DepthStencilBufferFormat(PIXEL_FORMAT_D24_UNORM_S8_UINT)
    , HideImplicitSwapChain(false)
    , ImplicitSwapChainCaption("Renderer")
    , ImplicitSwapChainWidth(800)
    , ImplicitSwapChainHeight(600)
    , ImplicitSwapChainFullScreen(RENDERER_FULLSCREEN_STATE_WINDOWED)
    , ImplicitSwapChainVSyncType(RENDERER_VSYNC_TYPE_NONE)
    , GPUFrameLatency(3)
    , CreateDebugDevice(DEFAULT_USE_DEBUG_DEVICE)
    , D3DForceWarpDevice(false)
{

}

SDL_Window* RenderLib::CreateRenderWindow(const char *windowTitle, uint32 windowWidth, uint32 windowHeight, uint32 windowFlags)
{
    // sdl init must be done before creating windows
    Log_DevPrintf(" Calling SDL_Init(SDL_INIT_VIDEO)...");
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        Log_ErrorPrintf(" Failed to initialize SDL video subsystem: %s", SDL_GetError());
        return false;
    }

    // TODO: Check for OGL
    // windowFlags |= SDL_WINDOW_OPENGL;

    return SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, windowFlags);
}

bool RenderLib::CreateRenderDevice(const RendererInitializationParameters *parameters, SDL_Window* window, GPUDevice** createdDevice, GPUContext** createdContext, GPUOutputBuffer** createdBuffer)
{
    // create start
    Log_InfoPrint("------- CreateRenderDevice -------");
    Log_DevPrintf("Platform: %s", NameTable_GetNameString(NameTables::RendererPlatformFullName, parameters->Platform));
    Log_DevPrintf("Render thread is %s", parameters->EnableThreadedRendering ? "enabled" : "disabled");
    Log_DevPrintf("Backbuffer format: %s", PixelFormat_GetPixelFormatName(parameters->BackBufferFormat));
    Log_DevPrintf("Depth-stencil format: %s", PixelFormat_GetPixelFormatName(parameters->DepthStencilBufferFormat));
    Log_DevPrintf("Implicit window is %s", parameters->HideImplicitSwapChain ? "hidden" : ((parameters->ImplicitSwapChainFullScreen == RENDERER_FULLSCREEN_STATE_FULLSCREEN) ? "fullscreen" : "windowed"));
    Log_DevPrintf("Implicit window dimensions: %ux%u", parameters->ImplicitSwapChainWidth, parameters->ImplicitSwapChainHeight);

    // Try the preferred renderer first
    bool backendInitialized = false;
    GPUDevice *pDevice = nullptr;
    GPUContext *pImmediateContext = nullptr;
    GPUOutputBuffer *pOutputBuffer = nullptr;
    Log_InfoPrintf("  Requested renderer: %s", NameTable_GetNameString(NameTables::RendererPlatform, parameters->Platform));
    for (uint32 i = 0; i < countof(s_renderSystemDeclarations); i++)
    {
        if (s_renderSystemDeclarations[i].Platform == parameters->Platform)
        {
            Log_InfoPrintf(" Creating \"%s\" render backend...", NameTable_GetNameString(NameTables::RendererPlatform, s_renderSystemDeclarations[i].Platform));
            backendInitialized = s_renderSystemDeclarations[i].Function(parameters, window, &pDevice, &pImmediateContext, &pOutputBuffer);
            break;
        }
    }

    if (!backendInitialized)
    {
        Log_ErrorPrintf(" Unknown render backend or failed to create: %s", NameTable_GetNameString(NameTables::RendererPlatform, parameters->Platform));
        return false;
    }

    // clear state ready for rendering
    pImmediateContext->ClearState(true, true, true, true);

    *createdDevice = pDevice;
    *createdContext = pImmediateContext;
    *createdBuffer = pOutputBuffer;

    return true;
}

bool RenderLib::CreateRenderDeviceAndWindow(const RendererInitializationParameters *parameters, GPUDevice** createdDevice, GPUContext** createdContext, RendererOutputWindow** createdWindow)
{
    // output window flags
    uint32 windowFlags = SDL_WINDOW_RESIZABLE;
    if (parameters->HideImplicitSwapChain)
        windowFlags |= SDL_WINDOW_HIDDEN;
    else
        windowFlags |= SDL_WINDOW_SHOWN;
    if (parameters->ImplicitSwapChainFullScreen == RENDERER_FULLSCREEN_STATE_WINDOWED_FULLSCREEN)
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    SDL_Window* pSDLWindow = CreateRenderWindow(parameters->ImplicitSwapChainCaption, parameters->ImplicitSwapChainWidth, parameters->ImplicitSwapChainHeight, windowFlags);
    if (pSDLWindow == nullptr)
    {
        Log_ErrorPrintf(" Failed to create implicit output window.");
        return false;
    }

    GPUOutputBuffer* gpuBuffer;
    if (!CreateRenderDevice(parameters, pSDLWindow, createdDevice, createdContext, &gpuBuffer))
    {
        SDL_DestroyWindow(pSDLWindow);
        return false;
    }
   
    // attempt to switch to exclusive fullscreen if possible
    RendererOutputWindow* outputWindow = new RendererOutputWindow(pSDLWindow, gpuBuffer, parameters->ImplicitSwapChainFullScreen);
    if (parameters->ImplicitSwapChainFullScreen == RENDERER_FULLSCREEN_STATE_FULLSCREEN)
    {
        Log_DevPrintf(" Attempting to switch to fullscreen...");
        if (!(*createdContext)->SetExclusiveFullScreen(true, parameters->ImplicitSwapChainWidth, parameters->ImplicitSwapChainHeight, 60))
            outputWindow->SetFullscreenState(RENDERER_FULLSCREEN_STATE_WINDOWED);
    }

    Log_InfoPrint("-----------------------------------");
    *createdWindow = outputWindow;
    return true;
}

RendererCounters::RendererCounters()
    : m_frameNumber(0)
    , m_drawCallCounter(0)
    , m_shaderChangeCounter(0)
    , m_pipelineChangeCounter(0)
    , m_framesDroppedCounter(0)
{
    Y_memzero((void *)m_resourceCPUMemoryUsage, sizeof(m_resourceCPUMemoryUsage));
    Y_memzero((void *)m_resourceGPUMemoryUsage, sizeof(m_resourceGPUMemoryUsage));
}

RendererCounters::~RendererCounters()
{

}

void RendererCounters::ResetPerFrameCounters()
{
    m_frameNumber++;
    m_drawCallCounter = 0;
    m_shaderChangeCounter = 0;
}

void RendererCounters::OnResourceCreated(const GPUResource *pResource)
{
    GPU_RESOURCE_TYPE type = pResource->GetResourceType();
    uint32 cpuMemoryUsage, gpuMemoryUsage;
    pResource->GetMemoryUsage(&cpuMemoryUsage, &gpuMemoryUsage);
    Y_AtomicAdd(m_resourceCPUMemoryUsage[type], (ptrdiff_t)cpuMemoryUsage);
    Y_AtomicAdd(m_resourceGPUMemoryUsage[type], (ptrdiff_t)gpuMemoryUsage);
}

void RendererCounters::OnResourceDeleted(const GPUResource *pResource)
{
    GPU_RESOURCE_TYPE type = pResource->GetResourceType();
    uint32 cpuMemoryUsage, gpuMemoryUsage;
    pResource->GetMemoryUsage(&cpuMemoryUsage, &gpuMemoryUsage);
    Y_AtomicAdd(m_resourceCPUMemoryUsage[type], -(ptrdiff_t)cpuMemoryUsage);
    Y_AtomicAdd(m_resourceGPUMemoryUsage[type], -(ptrdiff_t)gpuMemoryUsage);
}

void RENDERER_RASTERIZER_STATE_DESC::SetDefault()
{
    FillMode = RENDERER_FILL_SOLID;
    //CullMode = RENDERER_CULL_NONE;
    CullMode = RENDERER_CULL_BACK;
    //FrontCounterClockwise = false;
    FrontCounterClockwise = true;
    ScissorEnable = false;
    DepthBias = 0;
    SlopeScaledDepthBias = 0.0f;
    DepthClipEnable = true;
}

void RENDERER_DEPTHSTENCIL_STATE_DESC::SetDefault()
{
    DepthTestEnable = true;
    DepthWriteEnable = true;
    DepthFunc = GPU_COMPARISON_FUNC_LESS;

    StencilTestEnable = false;
    StencilReadMask = 0xFF;
    StencilWriteMask = 0xFF;
    StencilFrontFace.FailOp = RENDERER_STENCIL_OP_KEEP;
    StencilFrontFace.DepthFailOp = RENDERER_STENCIL_OP_KEEP;
    StencilFrontFace.PassOp = RENDERER_STENCIL_OP_KEEP;
    StencilFrontFace.CompareFunc = GPU_COMPARISON_FUNC_ALWAYS;
    StencilBackFace.FailOp = RENDERER_STENCIL_OP_KEEP;
    StencilBackFace.DepthFailOp = RENDERER_STENCIL_OP_KEEP;
    StencilBackFace.PassOp = RENDERER_STENCIL_OP_KEEP;
    StencilBackFace.CompareFunc = GPU_COMPARISON_FUNC_ALWAYS;
}

void RENDERER_BLEND_STATE_DESC::SetDefault()
{
    BlendEnable = false;
    SrcBlend = RENDERER_BLEND_ONE;
    DestBlend = RENDERER_BLEND_ZERO;
    BlendOp = RENDERER_BLEND_OP_ADD;
    SrcBlendAlpha = RENDERER_BLEND_ONE;
    DestBlendAlpha = RENDERER_BLEND_ZERO;
    BlendOpAlpha = RENDERER_BLEND_OP_ADD;
    ColorWriteEnable = true;
}

void GPU_SAMPLER_STATE_DESC::SetDefault()
{
    Filter = TEXTURE_FILTER_ANISOTROPIC;
    MaxAnisotropy = 16;   
    AddressU = TEXTURE_ADDRESS_MODE_WRAP;
    AddressV = TEXTURE_ADDRESS_MODE_WRAP;
    AddressW = TEXTURE_ADDRESS_MODE_WRAP;
    BorderColor = FloatColor::Black;
    LODBias = 0.0f;
    MinLOD = Y_INT32_MIN;
    MaxLOD = Y_INT32_MAX;
    ComparisonFunc = GPU_COMPARISON_FUNC_NEVER;
}


#pragma once
#include "YBaseLib/Memory.h"
#include "YBaseLib/NonCopyable.h"
#include "YBaseLib/String.h"
#include "YBaseLib/ReferenceCounted.h"
#include "YRenderLib/Common.h"
#include "YRenderLib/RendererTypes.h"

// Declare the render system window handle type
#if defined(Y_PLATFORM_WINDOWS)
    #include "YBaseLib/Windows/WindowsHeaders.h"
    typedef HWND RenderSystemWindowHandle;
#else
    typedef void *RenderSystemWindowHandle;
#endif

// Forward declarations
class ByteStream;

// Required for output window
struct SDL_Window;

// Capability mask
struct RendererCapabilities
{
    uint32 MaxTextureAnisotropy;
    uint32 MaximumVertexBuffers;
    uint32 MaximumConstantBuffers;
    uint32 MaximumTextureUnits;
    uint32 MaximumSamplers;
    uint32 MaximumRenderTargets;
    struct
    {
        bool SupportsMultithreadedResourceCreation : 1;
        bool SupportsCommandLists : 1;
        bool SupportsDrawBaseVertex : 1;
        bool SupportsDepthTextures : 1;
        bool SupportsTextureArrays : 1;
        bool SupportsCubeMapTextureArrays : 1;
        bool SupportsGeometryShaders : 1;
        bool SupportsSinglePassCubeMaps : 1;
        bool SupportsInstancing : 1;
        uint32: 1;
    };
};

// Base class of all gpu resources
class GPUResource : public ReferenceCounted
{
    DeclareNonCopyable(GPUResource);

public:
    GPUResource() {}
    virtual ~GPUResource() {}

    // Common methods for all resources
    virtual GPU_RESOURCE_TYPE GetResourceType() const = 0;
    virtual void GetMemoryUsage(uint32 *cpuMemoryUsage, uint32 *gpuMemoryUsage) const = 0;
    virtual void SetDebugName(const char *name) = 0;
};

class GPURasterizerState : public GPUResource
{
public:
    GPURasterizerState(const RENDERER_RASTERIZER_STATE_DESC *pDesc) { Y_memcpy(&m_desc, pDesc, sizeof(m_desc)); }
    virtual ~GPURasterizerState() {}

    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_RASTERIZER_STATE; }

    const RENDERER_RASTERIZER_STATE_DESC *GetDesc() const { return &m_desc; }

protected:
    RENDERER_RASTERIZER_STATE_DESC m_desc;

private:
};

class GPUDepthStencilState : public GPUResource
{
public:
    GPUDepthStencilState(const RENDERER_DEPTHSTENCIL_STATE_DESC *pDesc) { Y_memcpy(&m_desc, pDesc, sizeof(m_desc)); }
    virtual ~GPUDepthStencilState() {}

    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_DEPTH_STENCIL_STATE; }

    const RENDERER_DEPTHSTENCIL_STATE_DESC *GetDesc() const { return &m_desc; }

protected:
    RENDERER_DEPTHSTENCIL_STATE_DESC m_desc;

private:
};

class GPUBlendState : public GPUResource
{
public:
    GPUBlendState(const RENDERER_BLEND_STATE_DESC *pDesc) { Y_memcpy(&m_desc, pDesc, sizeof(m_desc)); }
    virtual ~GPUBlendState() {}

    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_BLEND_STATE; }

    const RENDERER_BLEND_STATE_DESC *GetDesc() const { return &m_desc; }

protected:
    RENDERER_BLEND_STATE_DESC m_desc;

private:
};

class GPUSamplerState : public GPUResource
{
public:
    GPUSamplerState(const GPU_SAMPLER_STATE_DESC *pDesc) { Y_memcpy(&m_desc, pDesc, sizeof(m_desc)); }
    virtual ~GPUSamplerState() {}

    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_SAMPLER_STATE; }

    const GPU_SAMPLER_STATE_DESC *GetDesc() const { return &m_desc; }

protected:
    GPU_SAMPLER_STATE_DESC m_desc;
};

class GPUBuffer : public GPUResource
{
public:
    GPUBuffer(const GPU_BUFFER_DESC *pDesc) { Y_memcpy(&m_desc, pDesc, sizeof(m_desc)); }
    virtual ~GPUBuffer() {}

    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_BUFFER; }

    const GPU_BUFFER_DESC *GetDesc() const { return &m_desc; }

protected:
    GPU_BUFFER_DESC m_desc;

private:
};

class GPUTexture : public GPUResource
{
public:
    virtual ~GPUTexture() {}

    virtual TEXTURE_TYPE GetTextureType() const = 0;
};

class GPUTexture1D : public GPUTexture
{
public:
    GPUTexture1D(const GPU_TEXTURE1D_DESC *pDesc) { Y_memcpy(&m_desc, pDesc, sizeof(m_desc)); }
    virtual ~GPUTexture1D() {}

    virtual TEXTURE_TYPE GetTextureType() const override { return TEXTURE_TYPE_1D; }
    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_TEXTURE1D; }

    const GPU_TEXTURE1D_DESC *GetDesc() const { return &m_desc; }

protected:
    GPU_TEXTURE1D_DESC m_desc;
};

class GPUTexture1DArray : public GPUTexture
{
public:
    GPUTexture1DArray(const GPU_TEXTURE1DARRAY_DESC *pDesc) { Y_memcpy(&m_desc, pDesc, sizeof(m_desc)); }
    virtual ~GPUTexture1DArray() {}

    virtual TEXTURE_TYPE GetTextureType() const override { return TEXTURE_TYPE_1D_ARRAY; }
    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_TEXTURE1DARRAY; }

    const GPU_TEXTURE1DARRAY_DESC *GetDesc() const { return &m_desc; }

protected:
    GPU_TEXTURE1DARRAY_DESC m_desc;
};

class GPUTexture2D : public GPUTexture
{
public:
    GPUTexture2D(const GPU_TEXTURE2D_DESC *pDesc) { Y_memcpy(&m_desc, pDesc, sizeof(m_desc)); }
    virtual ~GPUTexture2D() {}

    virtual TEXTURE_TYPE GetTextureType() const override { return TEXTURE_TYPE_2D; }
    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_TEXTURE2D; }

    const GPU_TEXTURE2D_DESC *GetDesc() const { return &m_desc; }

protected:
    GPU_TEXTURE2D_DESC m_desc;
};

class GPUTexture2DArray : public GPUTexture
{
public:
    GPUTexture2DArray(const GPU_TEXTURE2DARRAY_DESC *pDesc) { Y_memcpy(&m_desc, pDesc, sizeof(m_desc)); }
    virtual ~GPUTexture2DArray() {}

    virtual TEXTURE_TYPE GetTextureType() const override { return TEXTURE_TYPE_2D_ARRAY; }
    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_TEXTURE2DARRAY; }

    const GPU_TEXTURE2DARRAY_DESC *GetDesc() const { return &m_desc; }

protected:
    GPU_TEXTURE2DARRAY_DESC m_desc;
};

class GPUTexture3D : public GPUTexture
{
public:
    GPUTexture3D(const GPU_TEXTURE3D_DESC *pDesc) { Y_memcpy(&m_desc, pDesc, sizeof(m_desc)); }
    virtual ~GPUTexture3D() {}

    virtual TEXTURE_TYPE GetTextureType() const override { return TEXTURE_TYPE_3D; }
    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_TEXTURE3D; }

    const GPU_TEXTURE3D_DESC *GetDesc() const { return &m_desc; }

protected:
    GPU_TEXTURE3D_DESC m_desc;
};

class GPUTextureCube : public GPUTexture
{
public:
    GPUTextureCube(const GPU_TEXTURECUBE_DESC *pDesc) { Y_memcpy(&m_desc, pDesc, sizeof(m_desc)); }
    virtual ~GPUTextureCube() {}

    virtual TEXTURE_TYPE GetTextureType() const override { return TEXTURE_TYPE_CUBE; }
    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_TEXTURECUBE; }

    const GPU_TEXTURECUBE_DESC *GetDesc() const { return &m_desc; }

protected:
    GPU_TEXTURECUBE_DESC m_desc;
};

class GPUTextureCubeArray : public GPUTexture
{
public:
    GPUTextureCubeArray(const GPU_TEXTURECUBEARRAY_DESC *pDesc) { Y_memcpy(&m_desc, pDesc, sizeof(m_desc)); }
    virtual ~GPUTextureCubeArray() {}

    virtual TEXTURE_TYPE GetTextureType() const override { return TEXTURE_TYPE_CUBE_ARRAY; }
    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_TEXTURECUBEARRAY; }

    const GPU_TEXTURECUBEARRAY_DESC *GetDesc() const { return &m_desc; }

protected:
    GPU_TEXTURECUBEARRAY_DESC m_desc;
};

class GPUDepthTexture : public GPUTexture
{
public:
    GPUDepthTexture(const GPU_DEPTH_TEXTURE_DESC *pDesc) { Y_memcpy(&m_desc, pDesc, sizeof(m_desc)); }
    virtual ~GPUDepthTexture() {}

    virtual TEXTURE_TYPE GetTextureType() const override { return TEXTURE_TYPE_DEPTH; }
    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_DEPTH_TEXTURE; }

    const GPU_DEPTH_TEXTURE_DESC *GetDesc() const { return &m_desc; }

protected:
    GPU_DEPTH_TEXTURE_DESC m_desc;
};

class GPURenderTargetView : public GPUResource
{
public:
    GPURenderTargetView(GPUTexture *pTexture, const GPU_RENDER_TARGET_VIEW_DESC *pDesc) { Y_memcpy(&m_desc, pDesc, sizeof(m_desc)); m_pTexture = pTexture; m_pTexture->AddRef(); }
    virtual ~GPURenderTargetView() { m_pTexture->Release(); }

    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_RENDER_TARGET_VIEW; }
  
    const GPU_RENDER_TARGET_VIEW_DESC *GetDesc() const { return &m_desc; }
    GPUTexture *GetTargetTexture() const { return m_pTexture; }

protected:
    GPUTexture *m_pTexture;
    GPU_RENDER_TARGET_VIEW_DESC m_desc;
};

class GPUDepthStencilBufferView : public GPUResource
{
public:
    GPUDepthStencilBufferView(GPUTexture *pTexture, const GPU_DEPTH_STENCIL_BUFFER_VIEW_DESC *pDesc) { Y_memcpy(&m_desc, pDesc, sizeof(m_desc)); m_pTexture = pTexture; m_pTexture->AddRef(); }
    virtual ~GPUDepthStencilBufferView() { m_pTexture->Release(); }

    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_DEPTH_BUFFER_VIEW; }

    const GPU_DEPTH_STENCIL_BUFFER_VIEW_DESC *GetDesc() const { return &m_desc; }
    GPUTexture *GetTargetTexture() const { return m_pTexture; }

protected:
    GPUTexture *m_pTexture;
    GPU_DEPTH_STENCIL_BUFFER_VIEW_DESC m_desc;
};

class GPUQuery : public GPUResource
{
public:
    virtual ~GPUQuery() {}

    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_QUERY; }
    virtual GPU_QUERY_TYPE GetQueryType() const = 0;
};

class GPUInputLayout : public GPUResource
{
public:
    GPUInputLayout(GPU_VERTEX_ELEMENT_DESC* pElements, uint32 nElements) : m_pElements(pElements), m_nElements(nElements) {}
    virtual ~GPUInputLayout() { delete[] m_pElements; }

    virtual GPU_RESOURCE_TYPE GetResourceType() const override final { return GPU_RESOURCE_TYPE_INPUT_LAYOUT; }

    const GPU_VERTEX_ELEMENT_DESC* GetElement(uint32 index) const { return &m_pElements[index]; }
    uint32 GetElementCount() const { return m_nElements; }

protected:
    GPU_VERTEX_ELEMENT_DESC* m_pElements;
    uint32 m_nElements;
};

class GPUShaderProgram : public GPUResource
{
public:
    struct Uniform
    {
        String Name;
        uint32 Index;
        SHADER_PARAMETER_TYPE Type;
        uint32 ArraySize;
        uint32 ArrayStride;
    };

    struct ConstantBuffer
    {
        String Name;
        uint32 Slot;
        uint32 Size;
    };

    struct Sampler
    {
        String Name;
        uint32 Slot;
    };

    struct Resource
    {
        String Name;
        uint32 Slot;
        GPU_RESOURCE_TYPE Type;
        int32 LinkedSamplerIndex;
    };

    struct RWResource
    {
        String Name;
        uint32 Slot;
        GPU_RESOURCE_TYPE Type;
    };

public:
    GPUShaderProgram() {}
    virtual ~GPUShaderProgram() {}

    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_SHADER_PROGRAM; }

    // Program uniforms
    virtual uint32 GetUniformCount() const = 0;
    virtual const Uniform* GetUniformByIndex(uint32 index) const = 0;
    virtual const Uniform* GetUniformByName(const char* name) const = 0;
    virtual void SetUniform(uint32 index, SHADER_PARAMETER_TYPE valueType, const void *pValue) = 0;
    virtual void SetUniformArray(uint32 index, SHADER_PARAMETER_TYPE valueType, const void *pValue, uint32 firstElement, uint32 numElements) = 0;

    // Constant buffers
    virtual uint32 GetConstantBufferCount() const = 0;
    virtual const ConstantBuffer* GetConstantBufferByIndex(uint32 index) const = 0;
    virtual const ConstantBuffer* GetConstantBufferByName(const char* name) const = 0;

    // Samplers (unused by GL backend)
    virtual uint32 GetSamplerCount() const = 0;
    virtual const Sampler* GetSamplerByIndex(uint32 index) const = 0;
    virtual const Sampler* GetSamplerByName(const char* name) const = 0;

    // Textures (also read-only structured buffers)
    virtual uint32 GetResourceCount() const = 0;
    virtual const Resource* GetResourceByIndex(uint32 index) const = 0;
    virtual const Resource* GetResourceByName(const char* name) const = 0;

    // RW resources (unordered access)
    virtual uint32 GetRWResourceCount() const = 0;
    virtual const RWResource* GetRWResourceByIndex(uint32 index) const = 0;
    virtual const RWResource* GetRWResourceByName(const char* name) const = 0;
};

class GPUShaderPipeline : public GPUShaderProgram
{
public:
    GPUShaderPipeline() {}
    virtual ~GPUShaderPipeline() {}

    virtual GPU_RESOURCE_TYPE GetResourceType() const override { return GPU_RESOURCE_TYPE_SHADER_PIPELINE; }

    // TODO: Accessors for states....
};

class GPUOutputBuffer : public ReferenceCounted
{
    DeclareNonCopyable(GPUOutputBuffer);

public:
    GPUOutputBuffer(RENDERER_VSYNC_TYPE vsyncType) : m_vsyncType(vsyncType) {}
    virtual ~GPUOutputBuffer() {}

    // Get the dimensions of this swap chain.
    virtual uint32 GetWidth() const = 0;
    virtual uint32 GetHeight() const = 0;

    // Change the vsync type associated with this swap chain.
    RENDERER_VSYNC_TYPE GetVSyncType() const { return m_vsyncType; }
    virtual void SetVSyncType(RENDERER_VSYNC_TYPE vsyncType) = 0;

protected:
    RENDERER_VSYNC_TYPE m_vsyncType;
};

class RendererOutputWindow : public ReferenceCounted
{
    DeclareNonCopyable(RendererOutputWindow);

public:
    RendererOutputWindow(SDL_Window *pSDLWindow, GPUOutputBuffer *pBuffer, RENDERER_FULLSCREEN_STATE fullscreenState);
    virtual ~RendererOutputWindow();

    // Create a new window
    static RendererOutputWindow *Create(const char *windowTitle, uint32 windowWidth, uint32 windowHeight, RENDERER_VSYNC_TYPE vsyncType);

    // window accessors
    SDL_Window *GetSDLWindow() const { return m_pSDLWindow; }
    GPUOutputBuffer *GetOutputBuffer() const { return m_pOutputBuffer; }
    RENDERER_FULLSCREEN_STATE GetFullscreenState() const { return m_fullscreenState; }
    void SetFullscreenState(RENDERER_FULLSCREEN_STATE state) { m_fullscreenState = state; }
    void SetDimensions(uint32 width, uint32 height) { m_width = width; m_height = height; }
    void SetOutputBuffer(GPUOutputBuffer *pBuffer) { m_pOutputBuffer = pBuffer; }
    int32 GetPositionX() const { return m_positionX; }
    int32 GetPositionY() const { return m_positionY; }
    uint32 GetWidth() const { return m_width; }
    uint32 GetHeight() const { return m_height; }
    bool HasFocus() const { return m_hasFocus; }
    bool IsVisible() const { return m_visible; }
    bool IsClosed() const { return m_closed; }
    const String &GetTitle() const { return m_title; }
    bool IsMouseGrabbed() const { return m_mouseGrabbed; }
    bool IsMouseRelativeMovementEnabled() const { return m_mouseRelativeMovement; }

    // Change the caption of the window
    void SetWindowTitle(const char *title);

    // Show/hide the window
    void SetWindowVisibility(bool visible);

    // Change the position of the window
    void SetWindowPosition(int32 x, int32 y);

    // Changes the window size
    void SetWindowSize(uint32 width, uint32 height);

    // Capture the mouse, if it leaves the window it will still receive movement events
    void SetMouseGrab(bool enabled);

    // Enable relative mouse movement, when enabled the mouse cursor will be locked to the window and the cursor will be hidden
    void SetMouseRelativeMovement(bool enabled);

    // Handle any resize messages from SDL. Returns true if the window dimensions have changed.
    bool HandleMessages(GPUContext* pGPUContext);

protected:
    SDL_Window *m_pSDLWindow;
    GPUOutputBuffer *m_pOutputBuffer;
    RENDERER_FULLSCREEN_STATE m_fullscreenState;
    int32 m_positionX;
    int32 m_positionY;
    uint32 m_width;
    uint32 m_height;
    bool m_hasFocus;
    bool m_visible;
    bool m_closed;
    String m_title;
    bool m_mouseGrabbed;
    bool m_mouseRelativeMovement;
};

// Renderer stats
class RendererCounters
{
public:
    RendererCounters();
    ~RendererCounters();

    // Counters
    uint32 GetFrameNumber() const { return m_frameNumber; }
    uint32 GetDrawCallCounter() const { return m_drawCallCounter; }
    uint32 GetShaderChangeCounter() const { return m_shaderChangeCounter; }
    uint32 GetPipelineChangeCounter() const { return m_pipelineChangeCounter; }
    uint32 GetFramesDroppedCounter() const { return m_framesDroppedCounter; }

    // Counter updating
    void IncrementDrawCallCounter() { Y_AtomicIncrement(m_drawCallCounter); }
    void IncrementShaderChangeCounter() { Y_AtomicIncrement(m_shaderChangeCounter); }
    void IncrementPipelineChangeCounter() { Y_AtomicIncrement(m_pipelineChangeCounter); }
    void IncrementFramesDroppedCounter() { Y_AtomicIncrement(m_framesDroppedCounter); }
    void ResetPerFrameCounters();

    // Resource memory management
    void OnResourceCreated(const GPUResource *pResource);
    void OnResourceDeleted(const GPUResource *pResource);

private:
    uint32 m_frameNumber;

    uint32 m_drawCallCounter;
    uint32 m_shaderChangeCounter;
    uint32 m_pipelineChangeCounter;
    uint32 m_framesDroppedCounter;

    Y_ATOMIC_DECL ptrdiff_t m_resourceCPUMemoryUsage[GPU_RESOURCE_TYPE_COUNT];
    Y_ATOMIC_DECL ptrdiff_t m_resourceGPUMemoryUsage[GPU_RESOURCE_TYPE_COUNT];
};

class GPUDevice : public ReferenceCounted
{
    DeclareNonCopyable(GPUDevice);

public:
    GPUDevice() {}
    virtual ~GPUDevice() {}

    // Device queries.
    virtual RENDERER_PLATFORM GetPlatform() const = 0;
    virtual RENDERER_FEATURE_LEVEL GetFeatureLevel() const = 0;
    virtual TEXTURE_PLATFORM GetTexturePlatform() const = 0;
    virtual SHADER_PROGRAM_BYTECODE_TYPE GetShaderProgramType() const = 0;
    virtual void GetCapabilities(RendererCapabilities *pCapabilities) const = 0;
    virtual void GetCounters(RendererCounters *pCounters) const = 0;
    virtual bool CheckTexturePixelFormatCompatibility(PIXEL_FORMAT PixelFormat, PIXEL_FORMAT *CompatibleFormat = nullptr) const = 0;
    virtual void CorrectProjectionMatrix(float *projectionMatrix) const = 0;
    virtual float GetTexelOffset() const = 0;

    // Creates a swap chain on an existing window.
    virtual GPUOutputBuffer *CreateOutputBuffer(RenderSystemWindowHandle hWnd, RENDERER_VSYNC_TYPE vsyncType) = 0;
    virtual GPUOutputBuffer *CreateOutputBuffer(SDL_Window *pSDLWindow, RENDERER_VSYNC_TYPE vsyncType) = 0;

    // Resource creation
    virtual GPUDepthStencilState *CreateDepthStencilState(const RENDERER_DEPTHSTENCIL_STATE_DESC *pDepthStencilStateDesc) = 0;
    virtual GPURasterizerState *CreateRasterizerState(const RENDERER_RASTERIZER_STATE_DESC *pRasterizerStateDesc) = 0;
    virtual GPUBlendState *CreateBlendState(const RENDERER_BLEND_STATE_DESC *pBlendStateDesc) = 0;
    virtual GPUQuery *CreateQuery(GPU_QUERY_TYPE type) = 0;
    virtual GPUBuffer *CreateBuffer(const GPU_BUFFER_DESC *pDesc, const void *pInitialData = NULL) = 0;
    virtual GPUTexture1D *CreateTexture1D(const GPU_TEXTURE1D_DESC *pTextureDesc, const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc, const void **ppInitialData = NULL, const uint32 *pInitialDataPitch = NULL) = 0;
    virtual GPUTexture1DArray *CreateTexture1DArray(const GPU_TEXTURE1DARRAY_DESC *pTextureDesc, const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc, const void **ppInitialData = NULL, const uint32 *pInitialDataPitch = NULL) = 0;
    virtual GPUTexture2D *CreateTexture2D(const GPU_TEXTURE2D_DESC *pTextureDesc, const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc, const void **ppInitialData = NULL, const uint32 *pInitialDataPitch = NULL) = 0;
    virtual GPUTexture2DArray *CreateTexture2DArray(const GPU_TEXTURE2DARRAY_DESC *pTextureDesc, const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc, const void **ppInitialData = NULL, const uint32 *pInitialDataPitch = NULL) = 0;
    virtual GPUTexture3D *CreateTexture3D(const GPU_TEXTURE3D_DESC *pTextureDesc, const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc, const void **ppInitialData = NULL, const uint32 *pInitialDataPitch = NULL, const uint32 *pInitialDataSlicePitch = NULL) = 0;
    virtual GPUTextureCube *CreateTextureCube(const GPU_TEXTURECUBE_DESC *pTextureDesc, const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc, const void **ppInitialData = NULL, const uint32 *pInitialDataPitch = NULL) = 0;
    virtual GPUTextureCubeArray *CreateTextureCubeArray(const GPU_TEXTURECUBEARRAY_DESC *pTextureDesc, const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc, const void **ppInitialData = NULL, const uint32 *pInitialDataPitch = NULL) = 0;
    virtual GPUDepthTexture *CreateDepthTexture(const GPU_DEPTH_TEXTURE_DESC *pTextureDesc) = 0;
    virtual GPUSamplerState *CreateSamplerState(const GPU_SAMPLER_STATE_DESC *pSamplerStateDesc) = 0;
    virtual GPURenderTargetView *CreateRenderTargetView(GPUTexture *pTexture, const GPU_RENDER_TARGET_VIEW_DESC *pDesc) = 0;
    virtual GPUDepthStencilBufferView *CreateDepthStencilBufferView(GPUTexture *pTexture, const GPU_DEPTH_STENCIL_BUFFER_VIEW_DESC *pDesc) = 0;
    virtual GPUInputLayout* CreateInputLayout(const GPU_VERTEX_ELEMENT_DESC* pElements, uint32 nElements) = 0;
    virtual GPUShaderProgram *CreateGraphicsProgram(ByteStream *pByteCodeStream) = 0;
    virtual GPUShaderProgram *CreateComputeProgram(ByteStream *pByteCodeStream) = 0;
    // CreateGraphicsPipeline
    // CreateComputePipeline

    // When creating resources off-thread, there is an implicit flush/wait after each resource creation. This forces a group to be batched together.
    virtual void BeginResourceBatchUpload() = 0;
    virtual void EndResourceBatchUpload() = 0;
};

class GPUCommandList : public ReferenceCounted
{
    DeclareNonCopyable(GPUCommandList);

public:
    GPUCommandList() {}
    virtual ~GPUCommandList() {}

    // State clearing
    virtual void ClearState(bool clearShaders = true, bool clearBuffers = true, bool clearStates = true, bool clearRenderTargets = true) = 0;

    // State Management    
    virtual GPURasterizerState *GetRasterizerState() = 0;
    virtual void SetRasterizerState(GPURasterizerState *pRasterizerState) = 0;
    virtual GPUDepthStencilState *GetDepthStencilState() = 0;
    virtual uint8 GetDepthStencilStateStencilRef() = 0;
    virtual void SetDepthStencilState(GPUDepthStencilState *pDepthStencilState, uint8 stencilRef) = 0;
    virtual GPUBlendState *GetBlendState() = 0;
    virtual const FloatColor &GetBlendStateBlendFactor() = 0;
    virtual void SetBlendState(GPUBlendState *pBlendState, const FloatColor &blendFactor = FloatColor::White) = 0;

    // Viewport Management
    virtual const RENDERER_VIEWPORT *GetViewport() = 0;
    virtual void SetViewport(const RENDERER_VIEWPORT *pNewViewport) = 0;
    virtual void SetFullViewport(GPUTexture *pForRenderTarget = nullptr) = 0;

    // Scissor Rect Management
    virtual const RENDERER_SCISSOR_RECT *GetScissorRect() = 0;
    virtual void SetScissorRect(const RENDERER_SCISSOR_RECT *pScissorRect) = 0;

    // Texture copying
    virtual bool CopyTexture(GPUTexture2D *pSourceTexture, GPUTexture2D *pDestinationTexture) = 0;
    virtual bool CopyTextureRegion(GPUTexture2D *pSourceTexture, uint32 sourceX, uint32 sourceY, uint32 width, uint32 height, uint32 sourceMipLevel, GPUTexture2D *pDestinationTexture, uint32 destX, uint32 destY, uint32 destMipLevel) = 0;

    // Blit (copy) a texture to the currently bound framebuffer. If this texture is a different size, it'll be resized. This may destroy the shader and target state, so use carefully.
    virtual void BlitFrameBuffer(GPUTexture2D *pTexture, uint32 sourceX, uint32 sourceY, uint32 sourceWidth, uint32 sourceHeight, uint32 destX, uint32 destY, uint32 destWidth, uint32 destHeight, RENDERER_FRAMEBUFFER_BLIT_RESIZE_FILTER resizeFilter = RENDERER_FRAMEBUFFER_BLIT_RESIZE_FILTER_NEAREST) = 0;

    // Mipmap generation, texture must be created with GPU_TEXTURE_FLAG_GENERATE_MIPS
    virtual void GenerateMips(GPUTexture *pTexture) = 0;

    // Query accessing
    virtual bool BeginQuery(GPUQuery *pQuery) = 0;
    virtual bool EndQuery(GPUQuery *pQuery) = 0;

    // Predicated drawing
    virtual void SetPredication(GPUQuery *pQuery) = 0;

    // RT Clearing
    virtual void ClearTargets(bool clearColor = true, bool clearDepth = true, bool clearStencil = true, const FloatColor &clearColorValue = FloatColor::Black, float clearDepthValue = 1.0f, uint8 clearStencilValue = 0) = 0;
    virtual void DiscardTargets(bool discardColor = true, bool discardDepth = true, bool discardStencil = true) = 0;

    // Swap chain changing
    virtual GPUOutputBuffer *GetOutputBuffer() = 0;
    virtual void SetOutputBuffer(GPUOutputBuffer *pOutputBuffer) = 0;

    // Render target changing
    virtual uint32 GetRenderTargets(uint32 nRenderTargets, GPURenderTargetView **ppRenderTargetViews, GPUDepthStencilBufferView **ppDepthBufferView) = 0;
    virtual void SetRenderTargets(uint32 nRenderTargets, GPURenderTargetView **ppRenderTargets, GPUDepthStencilBufferView *pDepthBufferView) = 0;

    // Drawing Setup
    virtual DRAW_TOPOLOGY GetDrawTopology() = 0;
    virtual void SetDrawTopology(DRAW_TOPOLOGY Topology) = 0;
    virtual uint32 GetVertexBuffers(uint32 firstBuffer, uint32 nBuffers, GPUBuffer **ppVertexBuffers, uint32 *pVertexBufferOffsets, uint32 *pVertexBufferStrides) = 0;
    virtual void SetVertexBuffers(uint32 firstBuffer, uint32 nBuffers, GPUBuffer *const *ppVertexBuffers, const uint32 *pVertexBufferOffsets, const uint32 *pVertexBufferStrides) = 0;
    virtual void SetVertexBuffer(uint32 bufferIndex, GPUBuffer *pVertexBuffer, uint32 offset, uint32 stride) = 0;
    virtual void GetIndexBuffer(GPUBuffer **ppBuffer, GPU_INDEX_FORMAT *pFormat, uint32 *pOffset) = 0;
    virtual void SetIndexBuffer(GPUBuffer *pBuffer, GPU_INDEX_FORMAT format, uint32 offset) = 0;

    // Shader Setup
    virtual void SetInputLayout(GPUInputLayout* pInputLayout) = 0;
    virtual void SetShaderProgram(GPUShaderProgram *pShaderProgram) = 0;
    virtual void SetShaderConstantBuffer(uint32 index, GPUBuffer* pBuffer) = 0;
    virtual void SetShaderSampler(uint32 index, GPUSamplerState* pSamplerState) = 0;
    virtual void SetShaderResource(uint32 index, GPUResource *pResource) = 0;
    virtual void SetShaderRWResource(uint32 index, GPUResource *pResource) = 0;

    // Draw calls
    virtual void Draw(uint32 firstVertex, uint32 nVertices) = 0;
    virtual void DrawInstanced(uint32 firstVertex, uint32 nVertices, uint32 nInstances) = 0;
    virtual void DrawIndexed(uint32 startIndex, uint32 nIndices, uint32 baseVertex) = 0;
    virtual void DrawIndexedInstanced(uint32 startIndex, uint32 nIndices, uint32 baseVertex, uint32 nInstances) = 0;

    // Draw calls with user-space buffer
    virtual void DrawUserPointer(const void *pVertices, uint32 vertexSize, uint32 nVertices) = 0;

    // Compute shaders
    virtual void Dispatch(uint32 threadGroupCountX, uint32 threadGroupCountY, uint32 threadGroupCountZ) = 0;
};

class GPUContext : public GPUCommandList
{
    DeclareNonCopyable(GPUContext);

public:
    GPUContext() {}
    virtual ~GPUContext() {}

    // Start of frame
    virtual void BeginFrame() = 0;

    // Ensure all queued commands are sent to the GPU.
    virtual void Flush() = 0;

    // Ensure all commands have been completed by the GPU.
    virtual void Finish() = 0;

    // Swap chain manipulation
    virtual bool GetExclusiveFullScreen() = 0;
    virtual bool SetExclusiveFullScreen(bool enabled, uint32 width, uint32 height, uint32 refreshRate) = 0;
    virtual bool ResizeOutputBuffer(uint32 width = 0, uint32 height = 0) = 0;
    virtual void PresentOutputBuffer(GPU_PRESENT_BEHAVIOUR presentBehaviour) = 0;

    // Command list execution
    virtual GPUCommandList *CreateCommandList() = 0;
    virtual bool OpenCommandList(GPUCommandList *pCommandList) = 0;
    virtual bool CloseCommandList(GPUCommandList *pCommandList) = 0;
    virtual void ExecuteCommandList(GPUCommandList *pCommandList) = 0;

    // Buffer mapping/reading/writing
    virtual bool ReadBuffer(GPUBuffer *pBuffer, void *pDestination, uint32 start, uint32 count) = 0;
    virtual bool WriteBuffer(GPUBuffer *pBuffer, const void *pSource, uint32 start, uint32 count) = 0;
    virtual bool MapBuffer(GPUBuffer *pBuffer, GPU_MAP_TYPE mapType, void **ppPointer) = 0;
    virtual void Unmapbuffer(GPUBuffer *pBuffer, void *pPointer) = 0;

    // Texture reading/writing
    virtual bool ReadTexture(GPUTexture1D *pTexture, void *pDestination, uint32 cbDestination, uint32 mipIndex, uint32 start, uint32 count) = 0;
    virtual bool ReadTexture(GPUTexture1DArray *pTexture, void *pDestination, uint32 cbDestination, uint32 arrayIndex, uint32 mipIndex, uint32 start, uint32 count) = 0;
    virtual bool ReadTexture(GPUTexture2D *pTexture, void *pDestination, uint32 destinationRowPitch, uint32 cbDestination, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY) = 0;
    virtual bool ReadTexture(GPUTexture2DArray *pTexture, void *pDestination, uint32 destinationRowPitch, uint32 cbDestination, uint32 arrayIndex, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY) = 0;
    virtual bool ReadTexture(GPUTexture3D *pTexture, void *pDestination, uint32 destinationRowPitch, uint32 destinationSlicePitch, uint32 cbDestination, uint32 mipIndex, uint32 startX, uint32 startY, uint32 startZ, uint32 countX, uint32 countY, uint32 countZ) = 0;
    virtual bool ReadTexture(GPUTextureCube *pTexture, void *pDestination, uint32 destinationRowPitch, uint32 cbDestination, CUBEMAP_FACE face, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY) = 0;
    virtual bool ReadTexture(GPUTextureCubeArray *pTexture, void *pDestination, uint32 destinationRowPitch, uint32 cbDestination, uint32 arrayIndex, CUBEMAP_FACE face, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY) = 0;
    virtual bool ReadTexture(GPUDepthTexture *pTexture, void *pDestination, uint32 destinationRowPitch, uint32 cbDestination, uint32 startX, uint32 startY, uint32 countX, uint32 countY) = 0;
    virtual bool WriteTexture(GPUTexture1D *pTexture, const void *pSource, uint32 cbSource, uint32 mipIndex, uint32 start, uint32 count) = 0;
    virtual bool WriteTexture(GPUTexture1DArray *pTexture, const void *pSource, uint32 cbSource, uint32 arrayIndex, uint32 mipIndex, uint32 start, uint32 count) = 0;
    virtual bool WriteTexture(GPUTexture2D *pTexture, const void *pSource, uint32 sourceRowPitch, uint32 cbSource, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY) = 0;
    virtual bool WriteTexture(GPUTexture2DArray *pTexture, const void *pSource, uint32 sourceRowPitch, uint32 cbSource, uint32 arrayIndex, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY) = 0;
    virtual bool WriteTexture(GPUTexture3D *pTexture, const void *pSource, uint32 sourceRowPitch, uint32 sourceSlicePitch, uint32 cbSource, uint32 mipIndex, uint32 startX, uint32 startY, uint32 startZ, uint32 countX, uint32 countY, uint32 countZ) = 0;
    virtual bool WriteTexture(GPUTextureCube *pTexture, const void *pSource, uint32 sourceRowPitch, uint32 cbSource, CUBEMAP_FACE face, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY) = 0;
    virtual bool WriteTexture(GPUTextureCubeArray *pTexture, const void *pSource, uint32 sourceRowPitch, uint32 cbSource, uint32 arrayIndex, CUBEMAP_FACE face, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY) = 0;
    virtual bool WriteTexture(GPUDepthTexture *pTexture, const void *pSource, uint32 sourceRowPitch, uint32 cbSource, uint32 startX, uint32 startY, uint32 countX, uint32 countY) = 0;

    // Query readback
    virtual GPU_QUERY_GETDATA_RESULT GetQueryData(GPUQuery *pQuery, void *pData, uint32 cbData, uint32 flags) = 0;
};

struct RendererInitializationParameters
{
    RendererInitializationParameters();

    // Platform to create.
    RENDERER_PLATFORM Platform;

    // Override default behaviour of creating a render thread should the cvar be set.
    bool EnableThreadedRendering;

    // Pixel format of all on-screen windows.
    PIXEL_FORMAT BackBufferFormat;
    PIXEL_FORMAT DepthStencilBufferFormat;

    // OpenGL at least, requires at least one swap chain be created with the device (implicit swap chain), even if it is not used.
    // If this is set, the swap chain will be created at a minimum size (to save memory) and the render window will be hidden.
    // On other APIs that do not require an implicit swap chain, none will be created.
    bool HideImplicitSwapChain;

    // If created, this specifies the caption of the implicit swap chain window.
    const char *ImplicitSwapChainCaption;

    // If created, this specifies the dimensions of the implicit swap chain window.
    uint32 ImplicitSwapChainWidth;
    uint32 ImplicitSwapChainHeight;

    // If created, whether the implicit swap chain will be created full-screen.
    RENDERER_FULLSCREEN_STATE ImplicitSwapChainFullScreen;

    // Implicit swap chain vsync behaviour
    RENDERER_VSYNC_TYPE ImplicitSwapChainVSyncType;

    // Frame latency
    uint32 GPUFrameLatency;

    // Debug device, backend-specific behavior
    bool CreateDebugDevice;
    
    // Backend-specific options
    bool D3DForceWarpDevice;
};

namespace RenderLib {

SDL_Window* CreateRenderWindow(const char *windowTitle, uint32 windowWidth, uint32 windowHeight, uint32 windowFlags);

bool CreateRenderDevice(const RendererInitializationParameters *parameters, SDL_Window* window, GPUDevice** createdDevice, GPUContext** createdContext, GPUOutputBuffer** createdBuffer);
bool CreateRenderDeviceAndWindow(const RendererInitializationParameters *parameters, GPUDevice** createdDevice, GPUContext** createdContext, RendererOutputWindow** createdWindow);

}

#pragma once
#include "YRenderLib/Vulkan/VulkanCommon.h"
#include "YRenderLib/Vulkan/VulkanDefines.h"
#include "YRenderLib/Renderer.h"

class VulkanGPUDevice;

class VulkanGPUContext : public GPUContext
{
public:
    VulkanGPUContext(VulkanGPUDevice* pDevice);
    ~VulkanGPUContext();

    virtual void BeginFrame() override;
    virtual void Flush() override;
    virtual void Finish() override;

    virtual bool GetExclusiveFullScreen() override;
    virtual bool SetExclusiveFullScreen(bool enabled, uint32 width, uint32 height, uint32 refreshRate) override;
    virtual bool ResizeOutputBuffer(uint32 width = 0, uint32 height = 0) override;
    virtual void PresentOutputBuffer(GPU_PRESENT_BEHAVIOUR presentBehaviour) override;

    virtual GPUCommandList* CreateCommandList() override;
    virtual bool OpenCommandList(GPUCommandList* pCommandList) override;
    virtual bool CloseCommandList(GPUCommandList* pCommandList) override;
    virtual void ExecuteCommandList(GPUCommandList* pCommandList) override;

    virtual bool ReadBuffer(GPUBuffer* pBuffer, void* pDestination, uint32 start, uint32 count) override;
    virtual bool WriteBuffer(GPUBuffer* pBuffer, const void* pSource, uint32 start, uint32 count) override;
    virtual bool MapBuffer(GPUBuffer* pBuffer, GPU_MAP_TYPE mapType, void* *ppPointer) override;
    virtual void Unmapbuffer(GPUBuffer* pBuffer, void* pPointer) override;

    virtual bool ReadTexture(GPUTexture1D* pTexture, void* pDestination, uint32 cbDestination, uint32 mipIndex, uint32 start, uint32 count) override;
    virtual bool ReadTexture(GPUTexture1DArray* pTexture, void* pDestination, uint32 cbDestination, uint32 arrayIndex, uint32 mipIndex, uint32 start, uint32 count) override;
    virtual bool ReadTexture(GPUTexture2D* pTexture, void* pDestination, uint32 destinationRowPitch, uint32 cbDestination, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY) override;
    virtual bool ReadTexture(GPUTexture2DArray* pTexture, void* pDestination, uint32 destinationRowPitch, uint32 cbDestination, uint32 arrayIndex, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY) override;
    virtual bool ReadTexture(GPUTexture3D* pTexture, void* pDestination, uint32 destinationRowPitch, uint32 destinationSlicePitch, uint32 cbDestination, uint32 mipIndex, uint32 startX, uint32 startY, uint32 startZ, uint32 countX, uint32 countY, uint32 countZ) override;
    virtual bool ReadTexture(GPUTextureCube* pTexture, void* pDestination, uint32 destinationRowPitch, uint32 cbDestination, CUBEMAP_FACE face, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY) override;
    virtual bool ReadTexture(GPUTextureCubeArray* pTexture, void* pDestination, uint32 destinationRowPitch, uint32 cbDestination, uint32 arrayIndex, CUBEMAP_FACE face, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY) override;
    virtual bool ReadTexture(GPUDepthTexture* pTexture, void* pDestination, uint32 destinationRowPitch, uint32 cbDestination, uint32 startX, uint32 startY, uint32 countX, uint32 countY) override;
    virtual bool WriteTexture(GPUTexture1D* pTexture, const void* pSource, uint32 cbSource, uint32 mipIndex, uint32 start, uint32 count) override;
    virtual bool WriteTexture(GPUTexture1DArray* pTexture, const void* pSource, uint32 cbSource, uint32 arrayIndex, uint32 mipIndex, uint32 start, uint32 count) override;
    virtual bool WriteTexture(GPUTexture2D* pTexture, const void* pSource, uint32 sourceRowPitch, uint32 cbSource, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY) override;
    virtual bool WriteTexture(GPUTexture2DArray* pTexture, const void* pSource, uint32 sourceRowPitch, uint32 cbSource, uint32 arrayIndex, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY) override;
    virtual bool WriteTexture(GPUTexture3D* pTexture, const void* pSource, uint32 sourceRowPitch, uint32 sourceSlicePitch, uint32 cbSource, uint32 mipIndex, uint32 startX, uint32 startY, uint32 startZ, uint32 countX, uint32 countY, uint32 countZ) override;
    virtual bool WriteTexture(GPUTextureCube* pTexture, const void* pSource, uint32 sourceRowPitch, uint32 cbSource, CUBEMAP_FACE face, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY) override;
    virtual bool WriteTexture(GPUTextureCubeArray* pTexture, const void* pSource, uint32 sourceRowPitch, uint32 cbSource, uint32 arrayIndex, CUBEMAP_FACE face, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY) override;
    virtual bool WriteTexture(GPUDepthTexture* pTexture, const void* pSource, uint32 sourceRowPitch, uint32 cbSource, uint32 startX, uint32 startY, uint32 countX, uint32 countY) override;

    virtual void ClearState(bool clearShaders = true, bool clearBuffers = true, bool clearStates = true, bool clearRenderTargets = true) override;

    virtual GPURasterizerState* GetRasterizerState() override;
    virtual void SetRasterizerState(GPURasterizerState* pRasterizerState) override;

    virtual GPUDepthStencilState* GetDepthStencilState() override;
    virtual uint8 GetDepthStencilStateStencilRef() override;
    virtual void SetDepthStencilState(GPUDepthStencilState* pDepthStencilState, uint8 stencilRef) override;

    virtual GPUBlendState* GetBlendState() override;
    virtual const FloatColor &GetBlendStateBlendFactor() override;
    virtual void SetBlendState(GPUBlendState* pBlendState, const FloatColor &blendFactor = FloatColor::White) override;

    virtual const RENDERER_VIEWPORT* GetViewport() override;
    virtual void SetViewport(const RENDERER_VIEWPORT* pNewViewport) override;
    virtual void SetFullViewport(GPUTexture* pForRenderTarget = nullptr) override;

    virtual const RENDERER_SCISSOR_RECT* GetScissorRect() override;
    virtual void SetScissorRect(const RENDERER_SCISSOR_RECT* pScissorRect) override;

    virtual bool CopyTexture(GPUTexture2D* pSourceTexture, GPUTexture2D* pDestinationTexture) override;
    virtual bool CopyTextureRegion(GPUTexture2D* pSourceTexture, uint32 sourceX, uint32 sourceY, uint32 width, uint32 height, uint32 sourceMipLevel, GPUTexture2D* pDestinationTexture, uint32 destX, uint32 destY, uint32 destMipLevel) override;

    virtual void BlitFrameBuffer(GPUTexture2D* pTexture, uint32 sourceX, uint32 sourceY, uint32 sourceWidth, uint32 sourceHeight, uint32 destX, uint32 destY, uint32 destWidth, uint32 destHeight, RENDERER_FRAMEBUFFER_BLIT_RESIZE_FILTER resizeFilter = RENDERER_FRAMEBUFFER_BLIT_RESIZE_FILTER_NEAREST) override;

    virtual void GenerateMips(GPUTexture* pTexture) override;

    virtual bool BeginQuery(GPUQuery* pQuery) override;
    virtual bool EndQuery(GPUQuery* pQuery) override;
    virtual void SetPredication(GPUQuery* pQuery) override;
    virtual GPU_QUERY_GETDATA_RESULT GetQueryData(GPUQuery* pQuery, void* pData, uint32 cbData, uint32 flags) override;

    virtual void ClearTargets(bool clearColor = true, bool clearDepth = true, bool clearStencil = true, const FloatColor &clearColorValue = FloatColor::Black, float clearDepthValue = 1.0f, uint8 clearStencilValue = 0) override;
    virtual void DiscardTargets(bool discardColor = true, bool discardDepth = true, bool discardStencil = true) override;

    virtual GPUOutputBuffer* GetOutputBuffer() override;
    virtual void SetOutputBuffer(GPUOutputBuffer* pOutputBuffer) override;

    virtual uint32 GetRenderTargets(uint32 nRenderTargets, GPURenderTargetView* *ppRenderTargetViews, GPUDepthStencilBufferView* *ppDepthBufferView) override;
    virtual void SetRenderTargets(uint32 nRenderTargets, GPURenderTargetView* *ppRenderTargets, GPUDepthStencilBufferView* pDepthBufferView) override;

    virtual DRAW_TOPOLOGY GetDrawTopology() override;
    virtual void SetDrawTopology(DRAW_TOPOLOGY Topology) override;

    virtual uint32 GetVertexBuffers(uint32 firstBuffer, uint32 nBuffers, GPUBuffer* *ppVertexBuffers, uint32* pVertexBufferOffsets, uint32* pVertexBufferStrides) override;
    virtual void SetVertexBuffers(uint32 firstBuffer, uint32 nBuffers, GPUBuffer* const* ppVertexBuffers, const uint32* pVertexBufferOffsets, const uint32* pVertexBufferStrides) override;
    virtual void SetVertexBuffer(uint32 bufferIndex, GPUBuffer* pVertexBuffer, uint32 offset, uint32 stride) override;
    virtual void GetIndexBuffer(GPUBuffer* *ppBuffer, GPU_INDEX_FORMAT* pFormat, uint32* pOffset) override;
    virtual void SetIndexBuffer(GPUBuffer* pBuffer, GPU_INDEX_FORMAT format, uint32 offset) override;
    virtual void SetInputLayout(GPUInputLayout*pInputLayout) override;

    virtual void SetShaderProgram(GPUShaderProgram* pShaderProgram) override;
    virtual void SetShaderConstantBuffer(uint32 index, GPUBuffer* pBuffer) override;
    virtual void SetShaderSampler(uint32 index, GPUSamplerState* pSamplerState) override;
    virtual void SetShaderResource(uint32 index, GPUResource* pResource) override;
    virtual void SetShaderRWResource(uint32 index, GPUResource* pResource) override;

    virtual void Draw(uint32 firstVertex, uint32 nVertices) override;
    virtual void DrawInstanced(uint32 firstVertex, uint32 nVertices, uint32 nInstances) override;
    virtual void DrawIndexed(uint32 startIndex, uint32 nIndices, uint32 baseVertex) override;
    virtual void DrawIndexedInstanced(uint32 startIndex, uint32 nIndices, uint32 baseVertex, uint32 nInstances) override;
    virtual void DrawUserPointer(const void* pVertices, uint32 vertexSize, uint32 nVertices) override;

    virtual void Dispatch(uint32 threadGroupCountX, uint32 threadGroupCountY, uint32 threadGroupCountZ) override;

private:
    VulkanGPUDevice*m_pDevice;

    VkInstance m_vkInstance;
    VkDevice m_vkDevice;
};
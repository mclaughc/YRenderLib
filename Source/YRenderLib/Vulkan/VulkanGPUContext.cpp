#include "YBaseLib/Assert.h"
#include "YRenderLib/Vulkan/VulkanGPUContext.h"
#include "YRenderLib/Vulkan/VulkanGPUDevice.h"

VulkanGPUContext::VulkanGPUContext(VulkanGPUDevice* pDevice)
    : m_pDevice(pDevice)
    , m_vkInstance(pDevice->GetVkInstance())
    , m_vkDevice(pDevice->GetVkDevice())
{
    m_pDevice->AddRef();
}

VulkanGPUContext::~VulkanGPUContext()
{
    m_pDevice->Release();
}

void VulkanGPUContext::BeginFrame()
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::Flush()
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::Finish()
{
    Panic("The method or operation is not implemented.");
}

bool VulkanGPUContext::GetExclusiveFullScreen()
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::SetExclusiveFullScreen(bool enabled, uint32 width, uint32 height, uint32 refreshRate)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::ResizeOutputBuffer(uint32 width /*= 0*/, uint32 height /*= 0*/)
{
    Panic("The method or operation is not implemented.");
    return false;
}

void VulkanGPUContext::PresentOutputBuffer(GPU_PRESENT_BEHAVIOUR presentBehaviour)
{
    Panic("The method or operation is not implemented.");
}

GPUCommandList* VulkanGPUContext::CreateCommandList()
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

bool VulkanGPUContext::OpenCommandList(GPUCommandList* pCommandList)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::CloseCommandList(GPUCommandList* pCommandList)
{
    Panic("The method or operation is not implemented.");
    return false;
}

void VulkanGPUContext::ExecuteCommandList(GPUCommandList* pCommandList)
{
    Panic("The method or operation is not implemented.");
}

bool VulkanGPUContext::ReadBuffer(GPUBuffer* pBuffer, void* pDestination, uint32 start, uint32 count)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::WriteBuffer(GPUBuffer* pBuffer, const void* pSource, uint32 start, uint32 count)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::MapBuffer(GPUBuffer* pBuffer, GPU_MAP_TYPE mapType, void* *ppPointer)
{
    Panic("The method or operation is not implemented.");
    return false;
}

void VulkanGPUContext::Unmapbuffer(GPUBuffer* pBuffer, void* pPointer)
{
    Panic("The method or operation is not implemented.");
}

bool VulkanGPUContext::ReadTexture(GPUTexture1D* pTexture, void* pDestination, uint32 cbDestination, uint32 mipIndex, uint32 start, uint32 count)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::ReadTexture(GPUTexture1DArray* pTexture, void* pDestination, uint32 cbDestination, uint32 arrayIndex, uint32 mipIndex, uint32 start, uint32 count)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::ReadTexture(GPUTexture2D* pTexture, void* pDestination, uint32 destinationRowPitch, uint32 cbDestination, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::ReadTexture(GPUTexture2DArray* pTexture, void* pDestination, uint32 destinationRowPitch, uint32 cbDestination, uint32 arrayIndex, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::ReadTexture(GPUTexture3D* pTexture, void* pDestination, uint32 destinationRowPitch, uint32 destinationSlicePitch, uint32 cbDestination, uint32 mipIndex, uint32 startX, uint32 startY, uint32 startZ, uint32 countX, uint32 countY, uint32 countZ)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::ReadTexture(GPUTextureCube* pTexture, void* pDestination, uint32 destinationRowPitch, uint32 cbDestination, CUBEMAP_FACE face, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::ReadTexture(GPUTextureCubeArray* pTexture, void* pDestination, uint32 destinationRowPitch, uint32 cbDestination, uint32 arrayIndex, CUBEMAP_FACE face, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::ReadTexture(GPUDepthTexture* pTexture, void* pDestination, uint32 destinationRowPitch, uint32 cbDestination, uint32 startX, uint32 startY, uint32 countX, uint32 countY)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::WriteTexture(GPUTexture1D* pTexture, const void* pSource, uint32 cbSource, uint32 mipIndex, uint32 start, uint32 count)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::WriteTexture(GPUTexture1DArray* pTexture, const void* pSource, uint32 cbSource, uint32 arrayIndex, uint32 mipIndex, uint32 start, uint32 count)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::WriteTexture(GPUTexture2D* pTexture, const void* pSource, uint32 sourceRowPitch, uint32 cbSource, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::WriteTexture(GPUTexture2DArray* pTexture, const void* pSource, uint32 sourceRowPitch, uint32 cbSource, uint32 arrayIndex, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::WriteTexture(GPUTexture3D* pTexture, const void* pSource, uint32 sourceRowPitch, uint32 sourceSlicePitch, uint32 cbSource, uint32 mipIndex, uint32 startX, uint32 startY, uint32 startZ, uint32 countX, uint32 countY, uint32 countZ)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::WriteTexture(GPUTextureCube* pTexture, const void* pSource, uint32 sourceRowPitch, uint32 cbSource, CUBEMAP_FACE face, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::WriteTexture(GPUTextureCubeArray* pTexture, const void* pSource, uint32 sourceRowPitch, uint32 cbSource, uint32 arrayIndex, CUBEMAP_FACE face, uint32 mipIndex, uint32 startX, uint32 startY, uint32 countX, uint32 countY)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::WriteTexture(GPUDepthTexture* pTexture, const void* pSource, uint32 sourceRowPitch, uint32 cbSource, uint32 startX, uint32 startY, uint32 countX, uint32 countY)
{
    Panic("The method or operation is not implemented.");
    return false;
}

GPU_QUERY_GETDATA_RESULT VulkanGPUContext::GetQueryData(GPUQuery* pQuery, void* pData, uint32 cbData, uint32 flags)
{
    Panic("The method or operation is not implemented.");
    return GPU_QUERY_GETDATA_RESULT_ERROR;
}

void VulkanGPUContext::ClearState(bool clearShaders /*= true*/, bool clearBuffers /*= true*/, bool clearStates /*= true*/, bool clearRenderTargets /*= true*/)
{
    Panic("The method or operation is not implemented.");
}

GPURasterizerState* VulkanGPUContext::GetRasterizerState()
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

void VulkanGPUContext::SetRasterizerState(GPURasterizerState* pRasterizerState)
{
    Panic("The method or operation is not implemented.");
}

GPUDepthStencilState* VulkanGPUContext::GetDepthStencilState()
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

uint8 VulkanGPUContext::GetDepthStencilStateStencilRef()
{
    Panic("The method or operation is not implemented.");
    return 0;
}

void VulkanGPUContext::SetDepthStencilState(GPUDepthStencilState* pDepthStencilState, uint8 stencilRef)
{
    Panic("The method or operation is not implemented.");
}

GPUBlendState* VulkanGPUContext::GetBlendState()
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

const FloatColor &VulkanGPUContext::GetBlendStateBlendFactor()
{
    Panic("The method or operation is not implemented.");
    return FloatColor::Black;
}

void VulkanGPUContext::SetBlendState(GPUBlendState* pBlendState, const FloatColor &blendFactor /*= FloatColor::White*/)
{
    Panic("The method or operation is not implemented.");
}

const RENDERER_VIEWPORT* VulkanGPUContext::GetViewport()
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

void VulkanGPUContext::SetViewport(const RENDERER_VIEWPORT* pNewViewport)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::SetFullViewport(GPUTexture* pForRenderTarget /*= nullptr*/)
{
    Panic("The method or operation is not implemented.");
}

const RENDERER_SCISSOR_RECT* VulkanGPUContext::GetScissorRect()
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

void VulkanGPUContext::SetScissorRect(const RENDERER_SCISSOR_RECT* pScissorRect)
{
    Panic("The method or operation is not implemented.");
}

bool VulkanGPUContext::CopyTexture(GPUTexture2D* pSourceTexture, GPUTexture2D* pDestinationTexture)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::CopyTextureRegion(GPUTexture2D* pSourceTexture, uint32 sourceX, uint32 sourceY, uint32 width, uint32 height, uint32 sourceMipLevel, GPUTexture2D* pDestinationTexture, uint32 destX, uint32 destY, uint32 destMipLevel)
{
    Panic("The method or operation is not implemented.");
    return false;
}

void VulkanGPUContext::BlitFrameBuffer(GPUTexture2D* pTexture, uint32 sourceX, uint32 sourceY, uint32 sourceWidth, uint32 sourceHeight, uint32 destX, uint32 destY, uint32 destWidth, uint32 destHeight, RENDERER_FRAMEBUFFER_BLIT_RESIZE_FILTER resizeFilter /*= RENDERER_FRAMEBUFFER_BLIT_RESIZE_FILTER_NEAREST*/)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::GenerateMips(GPUTexture* pTexture)
{
    Panic("The method or operation is not implemented.");
}

bool VulkanGPUContext::BeginQuery(GPUQuery* pQuery)
{
    Panic("The method or operation is not implemented.");
    return false;
}

bool VulkanGPUContext::EndQuery(GPUQuery* pQuery)
{
    Panic("The method or operation is not implemented.");
    return false;
}

void VulkanGPUContext::SetPredication(GPUQuery* pQuery)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::ClearTargets(bool clearColor /*= true*/, bool clearDepth /*= true*/, bool clearStencil /*= true*/, const FloatColor &clearColorValue /*= FloatColor::Black*/, float clearDepthValue /*= 1.0f*/, uint8 clearStencilValue /*= 0*/)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::DiscardTargets(bool discardColor /*= true*/, bool discardDepth /*= true*/, bool discardStencil /*= true*/)
{
    Panic("The method or operation is not implemented.");
}

GPUOutputBuffer* VulkanGPUContext::GetOutputBuffer()
{
    Panic("The method or operation is not implemented.");
    return nullptr;
}

void VulkanGPUContext::SetOutputBuffer(GPUOutputBuffer* pOutputBuffer)
{
    Panic("The method or operation is not implemented.");
}

uint32 VulkanGPUContext::GetRenderTargets(uint32 nRenderTargets, GPURenderTargetView* *ppRenderTargetViews, GPUDepthStencilBufferView* *ppDepthBufferView)
{
    Panic("The method or operation is not implemented.");
    return 0;
}

void VulkanGPUContext::SetRenderTargets(uint32 nRenderTargets, GPURenderTargetView* *ppRenderTargets, GPUDepthStencilBufferView* pDepthBufferView)
{
    Panic("The method or operation is not implemented.");
}

DRAW_TOPOLOGY VulkanGPUContext::GetDrawTopology()
{
    Panic("The method or operation is not implemented.");
    return DRAW_TOPOLOGY_TRIANGLE_LIST;
}

void VulkanGPUContext::SetDrawTopology(DRAW_TOPOLOGY Topology)
{
    Panic("The method or operation is not implemented.");
}

uint32 VulkanGPUContext::GetVertexBuffers(uint32 firstBuffer, uint32 nBuffers, GPUBuffer* *ppVertexBuffers, uint32* pVertexBufferOffsets, uint32* pVertexBufferStrides)
{
    Panic("The method or operation is not implemented.");
    return 0;
}

void VulkanGPUContext::SetVertexBuffers(uint32 firstBuffer, uint32 nBuffers, GPUBuffer* const* ppVertexBuffers, const uint32* pVertexBufferOffsets, const uint32* pVertexBufferStrides)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::SetVertexBuffer(uint32 bufferIndex, GPUBuffer* pVertexBuffer, uint32 offset, uint32 stride)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::GetIndexBuffer(GPUBuffer* *ppBuffer, GPU_INDEX_FORMAT* pFormat, uint32* pOffset)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::SetIndexBuffer(GPUBuffer* pBuffer, GPU_INDEX_FORMAT format, uint32 offset)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::SetInputLayout(GPUInputLayout* pInputLayout)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::SetShaderProgram(GPUShaderProgram* pShaderProgram)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::SetShaderConstantBuffer(uint32 index, GPUBuffer* pBuffer)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::SetShaderSampler(uint32 index, GPUSamplerState* pSamplerState)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::SetShaderResource(uint32 index, GPUResource* pResource)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::SetShaderRWResource(uint32 index, GPUResource* pResource)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::Draw(uint32 firstVertex, uint32 nVertices)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::DrawInstanced(uint32 firstVertex, uint32 nVertices, uint32 nInstances)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::DrawIndexed(uint32 startIndex, uint32 nIndices, uint32 baseVertex)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::DrawIndexedInstanced(uint32 startIndex, uint32 nIndices, uint32 baseVertex, uint32 nInstances)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::DrawUserPointer(const void* pVertices, uint32 vertexSize, uint32 nVertices)
{
    Panic("The method or operation is not implemented.");
}

void VulkanGPUContext::Dispatch(uint32 threadGroupCountX, uint32 threadGroupCountY, uint32 threadGroupCountZ)
{
    Panic("The method or operation is not implemented.");
}


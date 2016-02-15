#include "YBaseLib/BinaryReader.h"
#include "YBaseLib/Log.h"
#include "YRenderLib/D3D11/D3D11Defines.h"
#include "YRenderLib/D3D11/D3D11GPUBuffer.h"
#include "YRenderLib/D3D11/D3D11GPUContext.h"
#include "YRenderLib/D3D11/D3D11GPUDevice.h"
#include "YRenderLib/D3D11/D3D11GPUShaderProgram.h"
#include "YRenderLib/D3D11/D3D11GPUTexture.h"
Log_SetChannel(Renderer);

D3D11GPUShaderProgram::D3D11GPUShaderProgram()
    : m_pD3DInputLayout(nullptr),
      m_pD3DVertexShader(nullptr),
      m_pD3DHullShader(nullptr),
      m_pD3DDomainShader(nullptr),
      m_pD3DGeometryShader(nullptr),
      m_pD3DPixelShader(nullptr),
      m_pD3DComputeShader(nullptr),
      m_uniform_buffer(nullptr),
      m_uniform_buffer_values(nullptr),
      m_uniform_buffer_size(0),
      m_uniforms_dirty(false)
{
    for (uint32 i = 0; i < countof(m_uniform_buffer_bind_point); i++)
        m_uniform_buffer_bind_point[i] = -1;
}

D3D11GPUShaderProgram::~D3D11GPUShaderProgram()
{
    delete[] m_uniform_buffer_values;
    SAFE_RELEASE(m_uniform_buffer);
    SAFE_RELEASE(m_pD3DComputeShader);
    SAFE_RELEASE(m_pD3DPixelShader);
    SAFE_RELEASE(m_pD3DGeometryShader);
    SAFE_RELEASE(m_pD3DDomainShader);
    SAFE_RELEASE(m_pD3DHullShader);
    SAFE_RELEASE(m_pD3DVertexShader);
    SAFE_RELEASE(m_pD3DInputLayout);
}

bool D3D11GPUShaderProgram::Create(D3D11GPUDevice *pDevice, ByteStream *pByteCodeStream)
{
    HRESULT hResult;
    ID3D11Device *pD3DDevice = pDevice->GetD3DDevice();
    BinaryReader binaryReader(pByteCodeStream);

    // get the header of the shader cache entry
    D3DShaderCacheEntryHeader header;
    if (!binaryReader.SafeReadBytes(&header, sizeof(header)) ||
        header.Signature != D3D_SHADER_CACHE_ENTRY_HEADER)
    {
        Log_ErrorPrintf("D3D11GPUShaderProgram::Create: Shader cache entry header corrupted.");
        return false;
    }

    // create d3d shader objects, and arrays for them
    for (uint32 stageIndex = 0; stageIndex < SHADER_PROGRAM_STAGE_COUNT; stageIndex++)
    {
        if (header.StageSize[stageIndex] == 0)
            continue;

        // read in stage bytecode
        uint32 stageByteCodeSize = header.StageSize[stageIndex];
        byte *pStageByteCode = new byte[stageByteCodeSize];
        if (!binaryReader.SafeReadBytes(pStageByteCode, stageByteCodeSize))
            return false;

        // create d3d objects
        switch (stageIndex)
        {
        case SHADER_PROGRAM_STAGE_VERTEX_SHADER:
            {
                if ((hResult = pD3DDevice->CreateVertexShader(pStageByteCode, stageByteCodeSize, nullptr, &m_pD3DVertexShader)) != S_OK)
                {
                    Log_ErrorPrintf("D3D11ShaderProgram::Create: CreateVertexShader failed with hResult %08X.", hResult);
                    delete[] pStageByteCode;
                    return false;
                }
            }
            break;

        case SHADER_PROGRAM_STAGE_HULL_SHADER:
            {
                if ((hResult = pD3DDevice->CreateHullShader(pStageByteCode, stageByteCodeSize, nullptr, &m_pD3DHullShader)) != S_OK)
                {
                    Log_ErrorPrintf("D3D11ShaderProgram::Create: CreateHullShader failed with hResult %08X.", hResult);
                    delete[] pStageByteCode;
                    return false;
                }

                delete[] pStageByteCode;
            }
            break;

        case SHADER_PROGRAM_STAGE_DOMAIN_SHADER:
            {
                if ((hResult = pD3DDevice->CreateDomainShader(pStageByteCode, stageByteCodeSize, nullptr, &m_pD3DDomainShader)) != S_OK)
                {
                    Log_ErrorPrintf("D3D11ShaderProgram::Create: CreateDomainShader failed with hResult %08X.", hResult);
                    delete[] pStageByteCode;
                    return false;
                }

                delete[] pStageByteCode;
            }
            break;

        case SHADER_PROGRAM_STAGE_GEOMETRY_SHADER:
            {
                if ((hResult = pD3DDevice->CreateGeometryShader(pStageByteCode, stageByteCodeSize, nullptr, &m_pD3DGeometryShader)) != S_OK)
                {
                    Log_ErrorPrintf("D3D11ShaderProgram::Create: CreateGeometryShader failed with hResult %08X.", hResult);
                    delete[] pStageByteCode;
                    return false;
                }

                delete[] pStageByteCode;
            }
            break;
        
        case SHADER_PROGRAM_STAGE_PIXEL_SHADER:
            {
                if ((hResult = pD3DDevice->CreatePixelShader(pStageByteCode, stageByteCodeSize, nullptr, &m_pD3DPixelShader)) != S_OK)
                {
                    Log_ErrorPrintf("D3D11ShaderProgram::Create: CreatePixelShader failed with hResult %08X.", hResult);
                    delete[] pStageByteCode;
                    return false;
                }

                delete[] pStageByteCode;
            }
            break;

        case SHADER_PROGRAM_STAGE_COMPUTE_SHADER:
            {
                if ((hResult = pD3DDevice->CreateComputeShader(pStageByteCode, stageByteCodeSize, nullptr, &m_pD3DComputeShader)) != S_OK)
                {
                    Log_ErrorPrintf("D3D11ShaderProgram::Create: CreateComputeShader failed with hResult %08X.", hResult);
                    delete[] pStageByteCode;
                    return false;
                }

                delete[] pStageByteCode;
            }
            break;

        default:
            UnreachableCode();
            break;
        }
    }

    if (header.UniformCount > 0)
    {
        // Create local buffer
        m_uniform_buffer_size = header.UniformBufferSize;
        m_uniform_buffer_values = new byte[m_uniform_buffer_size];
        Y_memzero(m_uniform_buffer_values, m_uniform_buffer_size);

        // Create GPU buffer
        CD3D11_BUFFER_DESC uniformBufferDesc(header.UniformBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0, 0);
        hResult = pDevice->GetD3DDevice()->CreateBuffer(&uniformBufferDesc, nullptr, &m_uniform_buffer);
        if (FAILED(hResult))
        {
            Log_ErrorPrintf("Failed to create uniform buffer");
            return false;
        }

        // Fill array
        m_uniforms.Resize(header.UniformCount);
        Y_memcpy(m_uniform_buffer_bind_point, header.UniformBufferBindPoints, sizeof(m_uniform_buffer_bind_point));
        for (uint32 i = 0; i < header.UniformCount; i++)
        {
            Uniform& dstUniform = m_uniforms[i];
            D3DShaderCacheEntryUniform srcUniform;
            if (!binaryReader.SafeReadType(&srcUniform))
                return false;

            if (!binaryReader.SafeReadFixedString(srcUniform.NameLength, &dstUniform.Name))
                return false;

            dstUniform.Index = i;
            dstUniform.Type = (SHADER_PARAMETER_TYPE)srcUniform.Type;
            dstUniform.ArraySize = srcUniform.ArraySize;
            dstUniform.ArrayStride = srcUniform.ArrayStride;
            dstUniform.BufferOffset = srcUniform.BufferOffset;
        }
    }

    if (header.ConstantBufferCount > 0)
    {
        m_constant_buffers.Resize(header.ConstantBufferCount);
        for (uint32 i = 0; i < m_constant_buffers.GetSize(); i++)
        {
            D3DShaderCacheEntryConstantBuffer srcConstantBuffer;
            ConstantBuffer& dstConstantBuffer = m_constant_buffers[i];
            if (!binaryReader.SafeReadBytes(&srcConstantBuffer, sizeof(srcConstantBuffer)))
                return false;

            if (!binaryReader.SafeReadFixedString(srcConstantBuffer.NameLength, &dstConstantBuffer.Name))
                return false;

            dstConstantBuffer.Slot = srcConstantBuffer.Slot;
            dstConstantBuffer.Size = srcConstantBuffer.Size;
            Y_memcpy(dstConstantBuffer.BindPoint, srcConstantBuffer.BindPoint, sizeof(dstConstantBuffer.BindPoint));
        }
    }

    if (header.SamplerCount > 0)
    {
        m_samplers.Resize(header.SamplerCount);
        for (uint32 i = 0; i < m_samplers.GetSize(); i++)
        {
            D3DShaderCacheEntrySampler srcSampler;
            Sampler& dstSampler = m_samplers[i];
            if (!binaryReader.SafeReadBytes(&srcSampler, sizeof(srcSampler)))
                return false;

            if (!binaryReader.SafeReadFixedString(srcSampler.NameLength, &dstSampler.Name))
                return false;

            dstSampler.Slot = srcSampler.Slot;
            Y_memcpy(dstSampler.BindPoint, srcSampler.BindPoint, sizeof(dstSampler.BindPoint));
        }
    }

    if (header.ResourceCount > 0)
    {
        m_resources.Resize(header.ResourceCount);
        for (uint32 i = 0; i < m_resources.GetSize(); i++)
        {
            D3DShaderCacheEntryResource srcResource;
            Resource& dstResource = m_resources[i];
            if (!binaryReader.SafeReadBytes(&srcResource, sizeof(srcResource)))
                return false;

            if (!binaryReader.SafeReadFixedString(srcResource.NameLength, &dstResource.Name))
                return false;

            dstResource.Slot = srcResource.Slot;
            dstResource.Type = (GPU_RESOURCE_TYPE)srcResource.Type;
            dstResource.LinkedSamplerIndex = srcResource.LinkedSamplerIndex;
            Y_memcpy(dstResource.BindPoint, srcResource.BindPoint, sizeof(dstResource.BindPoint));
        }
    }

    if (header.RWResourceCount > 0)
    {
        m_rw_resources.Resize(header.RWResourceCount);
        for (uint32 i = 0; i < m_rw_resources.GetSize(); i++)
        {
            D3DShaderCacheEntryRWResource srcRWResource;
            RWResource& dstRWResource = m_rw_resources[i];
            if (!binaryReader.SafeReadBytes(&srcRWResource, sizeof(srcRWResource)))
                return false;

            if (!binaryReader.SafeReadFixedString(srcRWResource.NameLength, &dstRWResource.Name))
                return false;

            dstRWResource.Slot = srcRWResource.Slot;
            dstRWResource.Type = (GPU_RESOURCE_TYPE)srcRWResource.Type;
            Y_memcpy(dstRWResource.BindPoint, srcRWResource.BindPoint, sizeof(dstRWResource.BindPoint));
        }
    }

    return true;
}

void D3D11GPUShaderProgram::Bind(D3D11GPUContext *pContext, D3D11GPUShaderProgram *pPreviousProgram)
{
    ID3D11DeviceContext *pD3DDeviceContext = pContext->GetD3DContext();

    // ------------------ VS ------------------
    if (m_pD3DVertexShader != nullptr)
        pD3DDeviceContext->VSSetShader(m_pD3DVertexShader, nullptr, 0);
    else if (pPreviousProgram && pPreviousProgram->m_pD3DVertexShader != nullptr)
        pD3DDeviceContext->VSSetShader(nullptr, nullptr, 0);

    // ------------------ HS ------------------
    if (m_pD3DHullShader != nullptr)
        pD3DDeviceContext->HSSetShader(m_pD3DHullShader, nullptr, 0);
    else if (pPreviousProgram && pPreviousProgram->m_pD3DHullShader != nullptr)
        pD3DDeviceContext->HSSetShader(nullptr, nullptr, 0);

    // ------------------ DS ------------------
    if (m_pD3DDomainShader != nullptr)
        pD3DDeviceContext->DSSetShader(m_pD3DDomainShader, nullptr, 0);
    else if (pPreviousProgram && pPreviousProgram->m_pD3DDomainShader != nullptr)
        pD3DDeviceContext->DSSetShader(nullptr, nullptr, 0);

    // ------------------ GS ------------------
    if (m_pD3DGeometryShader != nullptr)
        pD3DDeviceContext->GSSetShader(m_pD3DGeometryShader, nullptr, 0);
    else if (pPreviousProgram && pPreviousProgram->m_pD3DGeometryShader != nullptr)
        pD3DDeviceContext->GSSetShader(nullptr, nullptr, 0);

    // ------------------ PS ------------------
    if (m_pD3DPixelShader != nullptr)
        pD3DDeviceContext->PSSetShader(m_pD3DPixelShader, nullptr, 0);
    else if (pPreviousProgram && pPreviousProgram->m_pD3DPixelShader != nullptr)
        pD3DDeviceContext->PSSetShader(nullptr, nullptr, 0);

    // ------------------ CS ------------------
    if (m_pD3DComputeShader != nullptr)
        pD3DDeviceContext->CSSetShader(m_pD3DComputeShader, nullptr, 0);
    else if (pPreviousProgram && pPreviousProgram->m_pD3DComputeShader != nullptr)
        pD3DDeviceContext->CSSetShader(nullptr, nullptr, 0);

    // ------------------ IA ------------------
    if (m_pD3DInputLayout != nullptr)
        pD3DDeviceContext->IASetInputLayout(m_pD3DInputLayout);
}

void D3D11GPUShaderProgram::Unbind(D3D11GPUContext *pContext)
{
    ID3D11DeviceContext *pD3DDeviceContext = pContext->GetD3DContext();

    // ------------------ VS ------------------
    if (m_pD3DVertexShader != nullptr)
        pD3DDeviceContext->VSSetShader(nullptr, nullptr, 0);

    // ------------------ HS ------------------
    if (m_pD3DHullShader != nullptr)
        pD3DDeviceContext->HSSetShader(nullptr, nullptr, 0);

    // ------------------ DS ------------------
    if (m_pD3DDomainShader != nullptr)
        pD3DDeviceContext->DSSetShader(nullptr, nullptr, 0);

    // ------------------ GS ------------------
    if (m_pD3DGeometryShader != nullptr)
        pD3DDeviceContext->GSSetShader(nullptr, nullptr, 0);

    // ------------------ PS ------------------
    if (m_pD3DPixelShader != nullptr)
        pD3DDeviceContext->PSSetShader(nullptr, nullptr, 0);

    // ------------------ CS ------------------
    if (m_pD3DComputeShader != nullptr)
        pD3DDeviceContext->CSSetShader(nullptr, nullptr, 0);
}

void D3D11GPUShaderProgram::UpdateUniforms(D3D11GPUContext* context)
{
    if (!m_uniform_buffer)
        return;

    if (m_uniforms_dirty)
    {
        D3D11_MAPPED_SUBRESOURCE subres;
        HRESULT hr = context->GetD3DContext()->Map(m_uniform_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
        if (FAILED(hr))
        {
            Log_ErrorPrintf("Failed to map uniform buffer");
            return;
        }

        Y_memcpy(subres.pData, m_uniform_buffer_values, m_uniform_buffer_size);
        context->GetD3DContext()->Unmap(m_uniform_buffer, 0);
    }

    for (uint32 i = 0; i < SHADER_PROGRAM_STAGE_COUNT; i++)
    {
        if (m_uniform_buffer_bind_point[i] >= 0)
            context->SetShaderConstantBuffers((SHADER_PROGRAM_STAGE)i, (uint32)m_uniform_buffer_bind_point[i], m_uniform_buffer);
    }
}

void D3D11GPUShaderProgram::UpdateConstantBuffers(D3D11GPUContext* context, uint32 startIndex, uint32 endIndex, D3D11GPUBuffer* const* buffers)
{
    for (const ConstantBuffer& cbuffer : m_constant_buffers)
    {
        if (cbuffer.Slot >= startIndex && cbuffer.Slot <= endIndex)
        {
            ID3D11Buffer* D3DBuffer = (buffers[cbuffer.Slot]) ? buffers[cbuffer.Slot]->GetD3DBuffer() : nullptr;
            for (uint32 i = 0; i < SHADER_PROGRAM_STAGE_COUNT; i++)
            {
                if (cbuffer.BindPoint[i] >= 0)
                    context->SetShaderConstantBuffers((SHADER_PROGRAM_STAGE)i, (uint32)cbuffer.BindPoint[i], D3DBuffer);
            }
        }
    }
}

void D3D11GPUShaderProgram::UpdateSamplers(D3D11GPUContext* context, uint32 startIndex, uint32 endIndex, D3D11GPUSamplerState* const* samplers)
{
    for (const Sampler& sampler : m_samplers)
    {
        if (sampler.Slot >= startIndex && sampler.Slot <= endIndex)
        {
            ID3D11SamplerState* D3DSampler = (samplers[sampler.Slot]) ? samplers[sampler.Slot]->GetD3DSamplerState() : nullptr;
            for (uint32 i = 0; i < SHADER_PROGRAM_STAGE_COUNT; i++)
            {
                if (sampler.BindPoint[i] >= 0)
                    context->SetShaderSamplers((SHADER_PROGRAM_STAGE)i, (uint32)sampler.BindPoint[i], D3DSampler);
            }
        }
    }
}

void D3D11GPUShaderProgram::UpdateResources(D3D11GPUContext* context, uint32 startIndex, uint32 endIndex, GPUResource* const* resources, D3D11GPUSamplerState* const* samplers)
{
    for (const Resource& resource : m_resources)
    {
        if (resource.Slot >= startIndex && resource.Slot <= endIndex)
        {
            ID3D11ShaderResourceView* srv = (resources[resource.Slot]) ? D3D11Helpers::GetResourceShaderResourceView(resources[resource.Slot]) : nullptr;
            for (uint32 i = 0; i < SHADER_PROGRAM_STAGE_COUNT; i++)
            {
                if (resource.BindPoint[i] >= 0)
                    context->SetShaderResources((SHADER_PROGRAM_STAGE)i, (uint32)resource.BindPoint[i], srv);
            }

            if (resource.LinkedSamplerIndex >= 0 && resources[resource.Slot])
            {
                const Sampler& sampler = m_samplers[resource.LinkedSamplerIndex];

                // prefer user-provided samplers, otherwise use the texture's sampler
                ID3D11SamplerState* D3DSampler = D3D11Helpers::GetResourceSamplerState(resources[resource.Slot]);
                if (!samplers[sampler.Slot])
                {
                    for (uint32 i = 0; i < SHADER_PROGRAM_STAGE_COUNT; i++)
                    {
                        if (sampler.BindPoint[i] >= 0)
                            context->SetShaderSamplers((SHADER_PROGRAM_STAGE)i, (uint32)resource.BindPoint[i], D3DSampler);
                    }
                }
            }
        }
    }
}

void D3D11GPUShaderProgram::UpdateRWResources(D3D11GPUContext* context, uint32 startIndex, uint32 endIndex, GPUResource* const* resources)
{
    for (const Resource& resource : m_resources)
    {
        if (resource.Slot >= startIndex && resource.Slot <= endIndex)
        {
            ID3D11UnorderedAccessView* uav = (resources[resource.Slot]) ? D3D11Helpers::GetResourceUnorderedAccessView(resources[resource.Slot]) : nullptr;
            for (uint32 i = 0; i < SHADER_PROGRAM_STAGE_COUNT; i++)
            {
                if (resource.BindPoint[i] >= 0)
                    context->SetShaderUAVs((SHADER_PROGRAM_STAGE)i, (uint32)resource.BindPoint[i], uav);
            }
        }
    }
}

void D3D11GPUShaderProgram::GetMemoryUsage(uint32 *cpuMemoryUsage, uint32 *gpuMemoryUsage) const
{
    // dummy values for now
    if (cpuMemoryUsage != nullptr)
        *cpuMemoryUsage = sizeof(*this);

    if (gpuMemoryUsage != nullptr)
        *gpuMemoryUsage = 128;
}

void D3D11GPUShaderProgram::SetDebugName(const char *name)
{
    if (m_pD3DVertexShader != nullptr)
        D3D11Helpers::SetD3D11DeviceChildDebugName(m_pD3DVertexShader, name);
    if (m_pD3DHullShader != nullptr)
        D3D11Helpers::SetD3D11DeviceChildDebugName(m_pD3DHullShader, name);
    if (m_pD3DDomainShader != nullptr)
        D3D11Helpers::SetD3D11DeviceChildDebugName(m_pD3DDomainShader, name);
    if (m_pD3DGeometryShader != nullptr)
        D3D11Helpers::SetD3D11DeviceChildDebugName(m_pD3DGeometryShader, name);
    if (m_pD3DPixelShader != nullptr)
        D3D11Helpers::SetD3D11DeviceChildDebugName(m_pD3DPixelShader, name);
    if (m_pD3DComputeShader != nullptr)
        D3D11Helpers::SetD3D11DeviceChildDebugName(m_pD3DComputeShader, name);
}

uint32 D3D11GPUShaderProgram::GetUniformCount() const
{
    return m_uniforms.GetSize();
}

const GPUShaderProgram::Uniform* D3D11GPUShaderProgram::GetUniformByIndex(uint32 index) const
{
    return (index < m_uniforms.GetSize()) ? &m_uniforms[index] : nullptr;
}

const GPUShaderProgram::Uniform* D3D11GPUShaderProgram::GetUniformByName(const char* name) const
{
    for (const Uniform& uniform : m_uniforms)
    {
        if (uniform.Name.Compare(name))
            return &uniform;
    }

    return nullptr;
}

void D3D11GPUShaderProgram::SetUniform(uint32 index, SHADER_PARAMETER_TYPE valueType, const void *pValue)
{
    const Uniform *parameterInfo = &m_uniforms[index];
    DebugAssert(parameterInfo->Type == valueType);

    uint32 valueSize = ShaderParameterValueTypeSize(parameterInfo->Type);
    byte* bufferPtr = m_uniform_buffer_values + parameterInfo->BufferOffset;
    if (Y_memcmp(bufferPtr, pValue, valueSize))
    {
        Y_memcpy(bufferPtr, pValue, valueSize);
        m_uniforms_dirty = true;
    }
}

void D3D11GPUShaderProgram::SetUniformArray(uint32 index, SHADER_PARAMETER_TYPE valueType, const void *pValue, uint32 firstElement, uint32 numElements)
{
    const Uniform *parameterInfo = &m_uniforms[index];
    DebugAssert(parameterInfo->Type == valueType);

    uint32 valueSize = ShaderParameterValueTypeSize(parameterInfo->Type);
    DebugAssert(parameterInfo->Type == valueType && valueSize > 0);
    DebugAssert(numElements > 0 && (firstElement + numElements) <= parameterInfo->ArraySize);

    // if there is no padding, this can be done in a single operation
    byte* bufferPtr = m_uniform_buffer_values + parameterInfo->BufferOffset + (firstElement * parameterInfo->ArrayStride);
    if (valueSize == parameterInfo->ArrayStride)
    {
        if (Y_memcmp(bufferPtr, pValue, valueSize))
        {
            Y_memcpy(bufferPtr, pValue, valueSize * numElements);
            m_uniforms_dirty = true;
        }
    }
    else
    {
        if (Y_memcmp_stride(bufferPtr, parameterInfo->ArrayStride, pValue, valueSize, valueSize, numElements))
        {
            Y_memcpy_stride(bufferPtr, parameterInfo->ArrayStride, pValue, valueSize, valueSize, numElements);
            m_uniforms_dirty = true;
        }
    }
}

uint32 D3D11GPUShaderProgram::GetConstantBufferCount() const
{
    return m_constant_buffers.GetSize();
}

const GPUShaderProgram::ConstantBuffer* D3D11GPUShaderProgram::GetConstantBufferByIndex(uint32 index) const
{
    return (index < m_constant_buffers.GetSize()) ? &m_constant_buffers[index] : nullptr;
}

const GPUShaderProgram::ConstantBuffer* D3D11GPUShaderProgram::GetConstantBufferByName(const char* name) const
{
    for (const ConstantBuffer& cbuffer : m_constant_buffers)
    {
        if (cbuffer.Name.Compare(name))
            return &cbuffer;
    }

    return nullptr;
}

uint32 D3D11GPUShaderProgram::GetSamplerCount() const
{
    return m_samplers.GetSize();
}

const GPUShaderProgram::Sampler* D3D11GPUShaderProgram::GetSamplerByIndex(uint32 index) const
{
    return (index < m_samplers.GetSize()) ? &m_samplers[index] : nullptr;
}

const GPUShaderProgram::Sampler* D3D11GPUShaderProgram::GetSamplerByName(const char* name) const
{
    for (const Sampler& sampler : m_samplers)
    {
        if (sampler.Name.Compare(name))
            return &sampler;
    }

    return nullptr;
}

uint32 D3D11GPUShaderProgram::GetResourceCount() const
{
    return m_resources.GetSize();
}

const GPUShaderProgram::Resource* D3D11GPUShaderProgram::GetResourceByIndex(uint32 index) const
{
    return (index < m_resources.GetSize()) ? &m_resources[index] : nullptr;
}

const GPUShaderProgram::Resource* D3D11GPUShaderProgram::GetResourceByName(const char* name) const
{
    for (const Resource& resource : m_resources)
    {
        if (resource.Name.Compare(name))
            return &resource;
    }

    return nullptr;
}

uint32 D3D11GPUShaderProgram::GetRWResourceCount() const
{
    return m_rw_resources.GetSize();
}

const GPUShaderProgram::RWResource* D3D11GPUShaderProgram::GetRWResourceByIndex(uint32 index) const
{
    return (index < m_rw_resources.GetSize()) ? &m_rw_resources[index] : nullptr;
}

const GPUShaderProgram::RWResource* D3D11GPUShaderProgram::GetRWResourceByName(const char* name) const
{
    for (const RWResource& resource : m_rw_resources)
    {
        if (resource.Name.Compare(name))
            return &resource;
    }

    return nullptr;
}

GPUShaderProgram *D3D11GPUDevice::CreateGraphicsProgram(ByteStream *pByteCodeStream)
{
    D3D11GPUShaderProgram *pProgram = new D3D11GPUShaderProgram();
    if (!pProgram->Create(this, pByteCodeStream))
    {
        pProgram->Release();
        return nullptr;
    }

    return pProgram;
}

GPUShaderProgram *D3D11GPUDevice::CreateComputeProgram(ByteStream *pByteCodeStream)
{
    D3D11GPUShaderProgram *pProgram = new D3D11GPUShaderProgram();
    if (!pProgram->Create(this, pByteCodeStream))
    {
        pProgram->Release();
        return nullptr;
    }

    return pProgram;
}

#pragma once
#include "YBaseLib/Array.h"
#include "YRenderLib/D3D11/D3D11Common.h"
#include "YRenderLib/D3D11/D3DShaderCacheEntry.h"
#include "YRenderLib/Renderer.h"

class D3D11GPUShaderProgram : public GPUShaderProgram
{
public:
    struct Uniform : public GPUShaderProgram::Uniform
    {
        uint32 BufferOffset;
    };

    struct ConstantBuffer : public GPUShaderProgram::ConstantBuffer
    {
        int32 BindPoint[SHADER_PROGRAM_STAGE_COUNT];
    };

    struct Sampler : public GPUShaderProgram::Sampler
    {
        int32 BindPoint[SHADER_PROGRAM_STAGE_COUNT];
    };

    struct Resource : public GPUShaderProgram::Resource
    {
        int32 BindPoint[SHADER_PROGRAM_STAGE_COUNT];
    };

    struct RWResource : public GPUShaderProgram::RWResource
    {
        int32 BindPoint[SHADER_PROGRAM_STAGE_COUNT];
    };

public:
    D3D11GPUShaderProgram();
    virtual ~D3D11GPUShaderProgram();

    // create
    bool Create(D3D11GPUDevice *pDevice, ByteStream *pByteCodeStream);

    // bind to a context that has no current shader
    void Bind(D3D11GPUContext *pContext, D3D11GPUShaderProgram *pPreviousProgram);

    // unbind whatever stages exist on this shader
    void Unbind(D3D11GPUContext* pContext);

    // bind parameters
    void UpdateUniforms(D3D11GPUContext* context);
    void UpdateConstantBuffers(D3D11GPUContext* context, uint32 startIndex, uint32 endIndex, D3D11GPUBuffer* const* buffers);
    void UpdateSamplers(D3D11GPUContext* context, uint32 startIndex, uint32 endIndex, D3D11GPUSamplerState* const* samplers);
    void UpdateResources(D3D11GPUContext* context, uint32 startIndex, uint32 endIndex, GPUResource* const* resources, D3D11GPUSamplerState* const* samplers);
    void UpdateRWResources(D3D11GPUContext* context, uint32 startIndex, uint32 endIndex, GPUResource* const* resources);

    // resource virtuals
    virtual void GetMemoryUsage(uint32 *cpuMemoryUsage, uint32 *gpuMemoryUsage) const override;
    virtual void SetDebugName(const char *name) override;

    // uniform management
    virtual uint32 GetUniformCount() const override final;
    virtual const GPUShaderProgram::Uniform* GetUniformByIndex(uint32 index) const override final;
    virtual const GPUShaderProgram::Uniform* GetUniformByName(const char* name) const override final;
    virtual void SetUniform(uint32 index, SHADER_PARAMETER_TYPE valueType, const void *pValue) override final;
    virtual void SetUniformArray(uint32 index, SHADER_PARAMETER_TYPE valueType, const void *pValue, uint32 firstElement, uint32 numElements) override final;

    // constant buffers
    virtual uint32 GetConstantBufferCount() const override final;
    virtual const GPUShaderProgram::ConstantBuffer* GetConstantBufferByIndex(uint32 index) const override final;
    virtual const GPUShaderProgram::ConstantBuffer* GetConstantBufferByName(const char* name) const override final;

    // samplers
    virtual uint32 GetSamplerCount() const override final;
    virtual const GPUShaderProgram::Sampler* GetSamplerByIndex(uint32 index) const override final;
    virtual const GPUShaderProgram::Sampler* GetSamplerByName(const char* name) const override final;

    // resources
    virtual uint32 GetResourceCount() const override final;
    virtual const GPUShaderProgram::Resource* GetResourceByIndex(uint32 index) const override final;
    virtual const GPUShaderProgram::Resource* GetResourceByName(const char* name) const override final;

    // rw resources
    virtual uint32 GetRWResourceCount() const override final;
    virtual const GPUShaderProgram::RWResource* GetRWResourceByIndex(uint32 index) const override final;
    virtual const GPUShaderProgram::RWResource* GetRWResourceByName(const char* name) const override final;

protected:
    // type declarations
    typedef Array<Uniform> UniformArray;
    typedef Array<ConstantBuffer> ConstantBufferArray;
    typedef Array<Sampler> SamplerArray;
    typedef Array<Resource> ResourceArray;
    typedef Array<RWResource> RWResourceArray;

    // shader objects
    ID3D11InputLayout *m_pD3DInputLayout;
    ID3D11VertexShader *m_pD3DVertexShader;
    ID3D11HullShader *m_pD3DHullShader;
    ID3D11DomainShader *m_pD3DDomainShader;
    ID3D11GeometryShader *m_pD3DGeometryShader;
    ID3D11PixelShader *m_pD3DPixelShader;
    ID3D11ComputeShader *m_pD3DComputeShader;

    // arrays of above
    UniformArray m_uniforms;
    ConstantBufferArray m_constant_buffers;
    SamplerArray m_samplers;
    ResourceArray m_resources;
    RWResourceArray m_rw_resources;

    // uniform buffer (may be unused)
    ID3D11Buffer* m_uniform_buffer;
    byte* m_uniform_buffer_values;
    uint32 m_uniform_buffer_size;
    int32 m_uniform_buffer_bind_point[SHADER_PROGRAM_STAGE_COUNT];
    bool m_uniforms_dirty;
};

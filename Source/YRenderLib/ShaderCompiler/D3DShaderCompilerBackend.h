#pragma once
#include "YBaseLib/MemArray.h"
#include "YRenderLib/Common.h"
#include "YRenderLib/ShaderCompiler/ShaderCompilerBackend.h"

#ifdef Y_PLATFORM_WINDOWS

#include "YRenderLib/D3D11/D3D11Common.h"
#include "YRenderLib/D3D11/D3DShaderCacheEntry.h"

class D3DShaderCompilerBackend : public ShaderCompilerBackend
{
public:
    D3DShaderCompilerBackend(const ShaderCompiler* pFrontend, SHADER_PROGRAM_BYTECODE_TYPE type, uint32 compileFlags, ByteStream* pOutputStream, ByteStream* pErrorLogStream, ByteStream* pInfoLogStream);
    virtual ~D3DShaderCompilerBackend();

    virtual bool Compile() override final;

protected:
    // helper functions
    void BuildD3DDefineList(SHADER_PROGRAM_STAGE stage, MemArray<D3D_SHADER_MACRO> &D3DMacroArray);
    bool CompileShaderStage(SHADER_PROGRAM_STAGE stage);
    bool ReflectShader(SHADER_PROGRAM_STAGE stage);
    void AssignResourceSlots();
    void LinkResourceSamplers();

private:
    ID3DBlob *m_pStageByteCode[SHADER_PROGRAM_STAGE_COUNT];

    MemArray<D3DShaderCacheEntryUniform> m_outUniforms;
    MemArray<D3DShaderCacheEntryConstantBuffer> m_outConstantBuffers;
    MemArray<D3DShaderCacheEntrySampler> m_outSamplers;
    MemArray<D3DShaderCacheEntryResource> m_outResources;
    MemArray<D3DShaderCacheEntryRWResource> m_outRWResources;

    Array<String> m_outUniformNames;
    Array<String> m_outConstantBufferNames;
    Array<String> m_outSamplerNames;
    Array<String> m_outResourceNames;
    Array<String> m_outRWResourceNames;

    uint32 m_uniformBufferSize;
    int32 m_uniformBufferBindPoints[SHADER_PROGRAM_STAGE_COUNT];
};

#endif      // Y_PLATFORM_WINDOWS

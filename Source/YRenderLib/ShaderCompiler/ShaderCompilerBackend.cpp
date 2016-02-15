#include "YRenderLib/ShaderCompiler/ShaderCompilerBackend.h"

ShaderCompilerBackend::ShaderCompilerBackend(const ShaderCompiler* pFrontend, SHADER_PROGRAM_BYTECODE_TYPE type, uint32 compileFlags, ByteStream* pOutputStream, ByteStream* pErrorLogStream, ByteStream* pInfoLogStream)
    : m_pFrontend(pFrontend)
    , m_type(type)
    , m_compileFlags(compileFlags)
    , m_pOutputStream(pOutputStream)
    , m_pInfoLogStream((pInfoLogStream) ? AddRefAndReturn(pInfoLogStream) : ByteStream_CreateNullStream())
    , m_pErrorLogStream((pErrorLogStream) ? AddRefAndReturn(pErrorLogStream) : ByteStream_CreateNullStream())
    , m_outputWriter(m_pOutputStream)
    , m_infoLogWriter(m_pInfoLogStream)
    , m_errorLogWriter(m_pErrorLogStream)
{

}

ShaderCompilerBackend::~ShaderCompilerBackend()
{
    m_pErrorLogStream->Release();
    m_pInfoLogStream->Release();
}

bool ShaderCompilerBackend::ReadIncludeFile(bool systemInclude, const char *filename, void **ppOutFileContents, uint32 *pOutFileLength)
{
    ShaderCompiler::IncludeInterface* pIncludeInterface = m_pFrontend->GetIncludeInterface();
    if (pIncludeInterface)
        return pIncludeInterface->ReadIncludeFile(systemInclude, filename, ppOutFileContents, pOutFileLength);

    return false;
}

void ShaderCompilerBackend::FreeIncludeFile(void *pFileContents)
{
    ShaderCompiler::IncludeInterface* pIncludeInterface = m_pFrontend->GetIncludeInterface();
    if (pIncludeInterface)
        pIncludeInterface->FreeIncludeFile(pFileContents);
}

ShaderCompilerBackend* ShaderCompilerBackend::CreateBackend(const ShaderCompiler* pFrontend, SHADER_PROGRAM_BYTECODE_TYPE type, uint32 compileFlags, ByteStream* pOutputStream, ByteStream* pErrorLogStream, ByteStream* pInfoLogStream)
{
    switch (type)
    {
    case SHADER_PROGRAM_BYTECODE_TYPE_D3D_SM40:
    case SHADER_PROGRAM_BYTECODE_TYPE_D3D_SM41:
    case SHADER_PROGRAM_BYTECODE_TYPE_D3D_SM50:
        return CreateD3DBackend(pFrontend, type, compileFlags, pOutputStream, pErrorLogStream, pInfoLogStream);
    }

    return nullptr;
}

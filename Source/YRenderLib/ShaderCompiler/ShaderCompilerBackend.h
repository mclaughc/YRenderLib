#pragma once
#include "YBaseLib/BinaryWriter.h"
#include "YBaseLib/ByteStream.h"
#include "YBaseLib/ReferenceCounted.h"
#include "YBaseLib/TextWriter.h"
#include "YRenderLib/Common.h"
#include "YRenderLib/RendererTypes.h"
#include "YRenderLib/ShaderCompiler/ShaderCompiler.h"

class ShaderCompilerBackend : public ReferenceCounted
{
public:
    ShaderCompilerBackend(const ShaderCompiler* pFrontend, SHADER_PROGRAM_BYTECODE_TYPE type, uint32 compileFlags, ByteStream* pOutputStream, ByteStream* pErrorLogStream, ByteStream* pInfoLogStream);
    virtual ~ShaderCompilerBackend();

    // accessors
    const ShaderCompiler* GetFrontend() const { return m_pFrontend; }
    SHADER_PROGRAM_BYTECODE_TYPE GetType() const { return m_type; }
    uint32 GetCompileFlags() const { return m_compileFlags; }

    // Helper methods for reading various include files.
    bool ReadIncludeFile(bool systemInclude, const char *filename, void **ppOutFileContents, uint32 *pOutFileLength);
    void FreeIncludeFile(void *pFileContents);

    // compile
    virtual bool Compile() = 0;

    // backend factory
    static ShaderCompilerBackend* CreateBackend(const ShaderCompiler* pFrontend, SHADER_PROGRAM_BYTECODE_TYPE type, uint32 compileFlags, ByteStream* pOutputStream, ByteStream* pErrorLogStream, ByteStream* pInfoLogStream);

protected:
    const ShaderCompiler* m_pFrontend;
    SHADER_PROGRAM_BYTECODE_TYPE m_type;
    uint32 m_compileFlags;

    ByteStream* m_pOutputStream;
    ByteStream* m_pErrorLogStream;
    ByteStream* m_pInfoLogStream;

    BinaryWriter m_outputWriter;
    TextWriter m_errorLogWriter;
    TextWriter m_infoLogWriter;

    // Platform entry points
    static ShaderCompilerBackend* CreateD3DBackend(const ShaderCompiler* pFrontend, SHADER_PROGRAM_BYTECODE_TYPE type, uint32 compileFlags, ByteStream* pOutputStream, ByteStream* pErrorLogStream, ByteStream* pInfoLogStream);
};

#include "YBaseLib/Array.h"
#include "YBaseLib/AutoReleasePtr.h"
#include "YBaseLib/ByteStream.h"
#include "YBaseLib/FileSystem.h"
#include "YBaseLib/Log.h"
#include "YBaseLib/StringConverter.h"
#include "YRenderLib/ShaderCompiler/ShaderCompiler.h"
#include "YRenderLib/ShaderCompiler/ShaderCompilerBackend.h"
Log_SetChannel(ShaderCompiler);

class LocalIncludeInterface : public ShaderCompiler::IncludeInterface
{
    virtual void SetBaseFileName(const char* filename) override final
    {
        includeStack.Clear();
        includeStack.Add(filename);
    }
    
    virtual bool ReadIncludeFile(bool systemInclude, const char *filename, void **ppOutFileContents, uint32 *pOutFileLength) override final
    {
        PathString searchFileName;
        FileSystem::BuildPathRelativeToFile(searchFileName, includeStack.LastElement(), filename, true, true);

        ByteStream* pStream = FileSystem::OpenFile(searchFileName, BYTESTREAM_OPEN_READ | BYTESTREAM_OPEN_STREAMED);
        if (!pStream)
        {
            Log_ErrorPrintf("LocalIncludeInterface: Failed to open '%s'", searchFileName.GetCharArray());
            return false;
        }

        uint32 length = (uint32)pStream->GetSize();
        byte* data = new byte[length];
        if (!pStream->Read2(data, length))
        {
            Log_ErrorPrintf("LocalIncludeInterface: Failed to read '%s'", searchFileName.GetCharArray());
            pStream->Release();
            delete[] data;
            return false;
        }

        pStream->Release();
        *ppOutFileContents = data;
        *pOutFileLength = length;
        includeStack.Add(searchFileName);
        return true;
    }

    virtual void FreeIncludeFile(void* pFileContents) override final
    {
        byte* data = reinterpret_cast<byte*>(pFileContents);
        delete[] data;

        includeStack.Pop();
    }

    Array<String> includeStack;
};

ShaderCompiler::IncludeInterface* ShaderCompiler::CreateLocalIncludeInterface()
{
    return new LocalIncludeInterface();
}

ShaderCompiler* ShaderCompiler::Create()
{
    return new ShaderCompiler();
}

void ShaderCompiler::AddPreprocessorMacro(const char *define, const char *value)
{
    m_macros.Add(PreprocessorMacro(define, value));
}

void ShaderCompiler::AddPreprocessorMacro(const String &define, const String &value)
{
    m_macros.Add(PreprocessorMacro(define, value));
}

void ShaderCompiler::SetIncludeInterface(IncludeInterface* pInterface)
{
    if (m_pIncludeInterface == pInterface)
        return;

    if (m_pIncludeInterface)
        m_pIncludeInterface->Release();

    m_pIncludeInterface = pInterface;
    if (m_pIncludeInterface)
        m_pIncludeInterface->AddRef();
}

void ShaderCompiler::SetLocalIncludeInterface()
{
    IncludeInterface* pInterface = CreateLocalIncludeInterface();
    SetIncludeInterface(pInterface);
    pInterface->Release();
}

bool ShaderCompiler::LoadStageSourceCode(SHADER_PROGRAM_STAGE stage, const char* filename, const char *entrypoint)
{
    AutoReleasePtr<ByteStream> pStream = FileSystem::OpenFile(filename, BYTESTREAM_OPEN_READ | BYTESTREAM_OPEN_STREAMED);
    if (!pStream)
        return false;

    m_stageFileNames[stage] = filename;
    m_stageSourceCode[stage].Clear();
    m_stageEntryPoints[stage] = entrypoint;
    return StringConverter::StreamToString(m_stageSourceCode[stage], pStream);
}

void ShaderCompiler::SetStageSourceCode(SHADER_PROGRAM_STAGE stage, const char* filename, const char* source, const char *entrypoint)
{
    m_stageFileNames[stage] = filename;
    m_stageSourceCode[stage] = source;
    m_stageEntryPoints[stage] = entrypoint;
}

bool ShaderCompiler::CompileSingleTypeProgram(SHADER_PROGRAM_BYTECODE_TYPE type, uint32 compileFlags, ByteStream* pOutputStream, ByteStream* pErrorLogStream, ByteStream* pInfoLogStream)
{
    AutoReleasePtr<ShaderCompilerBackend> pBackend = ShaderCompilerBackend::CreateBackend(this, type, compileFlags, pOutputStream, pInfoLogStream, pErrorLogStream);
    if (pBackend == nullptr)
        return false;

    return pBackend->Compile();
}

bool ShaderCompiler::CompilePortableProgram(uint32 compileFlags, ByteStream* pOutputStream, ByteStream* pErrorLogStream, ByteStream* pInfoLogStream)
{
    Panic("Not implemented yet");
    return false;
}

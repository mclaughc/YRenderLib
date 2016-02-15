#pragma once
#include "YBaseLib/Array.h"
#include "YBaseLib/KeyValuePair.h"
#include "YBaseLib/ReferenceCounted.h"
#include "YBaseLib/String.h"
#include "YRenderLib/Common.h"
#include "YRenderLib/RendererTypes.h"

class ByteStream;

enum SHADER_COMPILER_FLAGS
{
    SHADER_COMPILER_FLAG_ENABLE_DEBUG_INFO      = (1 << 0),
    SHADER_COMPILER_FLAG_DISABLE_OPTIMIZATIONS  = (1 << 1),
};

class ShaderCompiler : public ReferenceCounted
{
public:
    typedef KeyValuePair<String, String> PreprocessorMacro;

    struct IncludeInterface : public ReferenceCounted
    {
        virtual void SetBaseFileName(const char* filename) = 0;
        virtual bool ReadIncludeFile(bool systemInclude, const char *filename, void **ppOutFileContents, uint32 *pOutFileLength) = 0;
        virtual void FreeIncludeFile(void* pFileContents) = 0;
    };

    // local, i.e. file, include interface
    static IncludeInterface* CreateLocalIncludeInterface();

    // create a shader compiler interface
    static ShaderCompiler* Create();

    // accessors
    const Array<PreprocessorMacro>& GetMacros() const { return m_macros; }
    const String& GetStageFileName(SHADER_PROGRAM_STAGE stage) const { return m_stageFileNames[stage]; }
    const String& GetStageSourceCode(SHADER_PROGRAM_STAGE stage) const { return m_stageSourceCode[stage]; }
    const String& GetStageEntryPoint(SHADER_PROGRAM_STAGE stage) const { return m_stageEntryPoints[stage]; }
    IncludeInterface* GetIncludeInterface() const { return m_pIncludeInterface; }

    // change include interface
    void SetIncludeInterface(IncludeInterface* pInterface);
    void SetLocalIncludeInterface();

    // adds preprocesor macros
    void AddPreprocessorMacro(const char *define, const char *value);
    void AddPreprocessorMacro(const String &define, const String &value);

    // loads source code from include interface
    bool LoadStageSourceCode(SHADER_PROGRAM_STAGE stage, const char* filename, const char *entrypoint);

    // sets source code
    void SetStageSourceCode(SHADER_PROGRAM_STAGE stage, const char* filename, const char* source, const char *entrypoint);

    // compiles the current config to a single, backend shader
    bool CompileSingleTypeProgram(SHADER_PROGRAM_BYTECODE_TYPE type, uint32 compileFlags, ByteStream* pOutputStream, ByteStream* pErrorLogStream, ByteStream* pInfoLogStream);

    // compiles the current config to a global, backend-independant shader
    bool CompilePortableProgram(uint32 compileFlags, ByteStream* pOutputStream, ByteStream* pErrorLogStream, ByteStream* pInfoLogStream);

protected:
    ShaderCompiler() = default;
    virtual ~ShaderCompiler() = default;

private:
    Array<PreprocessorMacro> m_macros;
    String m_stageFileNames[SHADER_PROGRAM_STAGE_COUNT];
    String m_stageSourceCode[SHADER_PROGRAM_STAGE_COUNT];
    String m_stageEntryPoints[SHADER_PROGRAM_STAGE_COUNT];
    IncludeInterface* m_pIncludeInterface;
};


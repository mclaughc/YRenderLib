#include "YBaseLib/Log.h"
#include "YBaseLib/NumericLimits.h"
#include "YBaseLib/Timer.h"
#include "YRenderLib/ShaderCompiler/D3DShaderCompilerBackend.h"

#ifdef Y_PLATFORM_WINDOWS
#include <wrl/client.h>
Log_SetChannel(D3DShaderCompilerBackend);

#pragma comment(lib, "d3dcompiler.lib")

struct PlatformProfileInfo
{
    SHADER_PROGRAM_BYTECODE_TYPE Type;
    const char *StageProfiles[SHADER_PROGRAM_STAGE_COUNT];
};

static const PlatformProfileInfo s_internalProfileInfo[RENDERER_FEATURE_LEVEL_COUNT] =
{
    //type                                      vs profile          hs profile          ds profile          gs profile          ps profile          cs profile  
    { SHADER_PROGRAM_BYTECODE_TYPE_D3D_SM40,    { "vs_4_0",         nullptr,            nullptr,            "gs_4_0",           "ps_4_0",           "cs_4_0"    } },
    { SHADER_PROGRAM_BYTECODE_TYPE_D3D_SM41,    { "vs_4_1",         nullptr,            nullptr,            "gs_4_1",           "ps_4_1",           "cs_4_1"    } },
    { SHADER_PROGRAM_BYTECODE_TYPE_D3D_SM50,    { "vs_5_0",         "hs_5_0",           "ds_5_0",           "gs_5_0",           "ps_5_0",           "cs_5_0"    } }
};

// d3d11 include interface
struct D3DIncludeInterface : public ID3DInclude
{
    D3DIncludeInterface(D3DShaderCompilerBackend *pParent) : m_pParent(pParent) {}

    STDOVERRIDEMETHODIMP Open(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
    {
        return (m_pParent->ReadIncludeFile((IncludeType == D3D_INCLUDE_SYSTEM), pFileName, const_cast<LPVOID *>(ppData), pBytes)) ? S_OK : E_FAIL;
    }

    STDOVERRIDEMETHODIMP Close(THIS_ LPCVOID pData)
    {
        m_pParent->FreeIncludeFile(const_cast<LPVOID>(pData));
        return S_OK;
    }

private:
    D3DShaderCompilerBackend *m_pParent;
};

D3DShaderCompilerBackend::D3DShaderCompilerBackend(const ShaderCompiler* pFrontend, SHADER_PROGRAM_BYTECODE_TYPE type, uint32 compileFlags, ByteStream* pOutputStream, ByteStream* pErrorLogStream, ByteStream* pInfoLogStream)
    : ShaderCompilerBackend(pFrontend, type, compileFlags, pOutputStream, pErrorLogStream, pInfoLogStream)
{
    Y_memzero(m_pStageByteCode, sizeof(m_pStageByteCode));

    m_uniformBufferSize = 0;
    for (uint32 i = 0; i < SHADER_PROGRAM_STAGE_COUNT; i++)
        m_uniformBufferBindPoints[i] = -1;
}

D3DShaderCompilerBackend::~D3DShaderCompilerBackend()
{
    for (uint32 i = 0; i < countof(m_pStageByteCode); i++)
    {
        if (m_pStageByteCode[i] != nullptr)
            m_pStageByteCode[i]->Release();
    }
}

void D3DShaderCompilerBackend::BuildD3DDefineList(SHADER_PROGRAM_STAGE stage, MemArray<D3D_SHADER_MACRO> &D3DMacroArray)
{
#define ADD_D3D_SHADER_MACRO(MacroName, MacroDefinition) MULTI_STATEMENT_MACRO_BEGIN \
                                                    Macro.Name = MacroName; \
                                                    Macro.Definition = MacroDefinition; \
                                                    D3DMacroArray.Add(Macro); \
                                               MULTI_STATEMENT_MACRO_END

    D3D_SHADER_MACRO Macro;  

    // stage-specific defines
    switch (stage)
    {
    case SHADER_PROGRAM_STAGE_VERTEX_SHADER:
        ADD_D3D_SHADER_MACRO("VERTEX_SHADER",        "1");
        break;

    case SHADER_PROGRAM_STAGE_HULL_SHADER:
        ADD_D3D_SHADER_MACRO("HULL_SHADER",          "1");
        break;

    case SHADER_PROGRAM_STAGE_DOMAIN_SHADER:
        ADD_D3D_SHADER_MACRO("DOMAIN_SHADER",        "1");
        break;

    case SHADER_PROGRAM_STAGE_GEOMETRY_SHADER:
        ADD_D3D_SHADER_MACRO("GEOMETRY_SHADER",      "1");
        break;

    case SHADER_PROGRAM_STAGE_PIXEL_SHADER:
        ADD_D3D_SHADER_MACRO("PIXEL_SHADER",         "1");
        break;

    case SHADER_PROGRAM_STAGE_COMPUTE_SHADER:
        ADD_D3D_SHADER_MACRO("COMPUTE_SHADER",       "1");
        break;
    }

    // build the d3d list
    for (const ShaderCompiler::PreprocessorMacro& macro : m_pFrontend->GetMacros())
        ADD_D3D_SHADER_MACRO(macro.Key, macro.Value);

    // NULL end-macro
    ADD_D3D_SHADER_MACRO(NULL, NULL);
#undef ADD_D3D_SHADER_MACRO
}

bool D3DShaderCompilerBackend::CompileShaderStage(SHADER_PROGRAM_STAGE stage)
{
    D3DIncludeInterface includeInterface(this);
    HRESULT hResult;
    
    // lookup profile
    const PlatformProfileInfo *pPlatformProfileInfo = nullptr;
    for (uint32 i = 0; i < countof(s_internalProfileInfo); i++)
    {
        if (s_internalProfileInfo[i].Type == m_type)
        {
            pPlatformProfileInfo = &s_internalProfileInfo[i];
            break;
        }
    }
    if (pPlatformProfileInfo == nullptr)
        return false;
    
    // check for unspecified stage entry points
    if (m_pFrontend->GetStageSourceCode(stage).IsEmpty() || m_pFrontend->GetStageEntryPoint(stage).IsEmpty())
        return true;

    // get define list
    MemArray<D3D_SHADER_MACRO> D3DMacroArray;
    BuildD3DDefineList(stage, D3DMacroArray);

    // flags
    uint32 D3DCompileFlags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

    // debug flags
    if (m_compileFlags & SHADER_COMPILER_FLAG_ENABLE_DEBUG_INFO)
        D3DCompileFlags |= D3DCOMPILE_DEBUG;

    // optimization flags
    if (m_compileFlags & SHADER_COMPILER_FLAG_DISABLE_OPTIMIZATIONS)
        D3DCompileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
    else
        D3DCompileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;

    ID3DBlob *pCodeBlob;
    ID3DBlob *pErrorBlob;

    // shader dumping
    if (m_pInfoLogStream != nullptr)
    {
        ID3DBlob *pCodeText;
        hResult = D3DPreprocess(m_pFrontend->GetStageSourceCode(stage),
                                m_pFrontend->GetStageSourceCode(stage).GetLength(),
                                m_pFrontend->GetStageFileName(stage),
                                D3DMacroArray.GetBasePointer(),
                                &includeInterface,
                                &pCodeText,
                                &pErrorBlob);

        if (FAILED(hResult))
        {
            Log_ErrorPrintf("CompileShaderStage: D3DPreprocess failed with HResult %08X", hResult);
            if (pErrorBlob)
            {
                Log_ErrorPrint((const char *)pErrorBlob->GetBufferPointer());
                m_errorLogWriter.WriteLine((const char *)pErrorBlob->GetBufferPointer(), pErrorBlob->GetBufferSize());
                pErrorBlob->Release();
            }

            return false;
        }

        if (pErrorBlob)
        {
            Log_WarningPrintf("CompileShaderStage: Preprocess succeeded with warnings.");
            Log_WarningPrint((const char *)pErrorBlob->GetBufferPointer());
            m_errorLogWriter.WriteLine((const char *)pErrorBlob->GetBufferPointer(), pErrorBlob->GetBufferSize());
            pErrorBlob->Release();
        }

        m_errorLogWriter.WriteFormattedLine("Preprocessed shader %s:", pPlatformProfileInfo->StageProfiles[stage]);
        m_errorLogWriter.WriteWithLineNumbers(reinterpret_cast<const char *>(pCodeText->GetBufferPointer()), (uint32)pCodeText->GetBufferSize());
        pCodeText->Release();
    }

    // invoke compiler
    hResult = D3DCompile(m_pFrontend->GetStageSourceCode(stage),
                         m_pFrontend->GetStageSourceCode(stage).GetLength(),
                         m_pFrontend->GetStageFileName(stage),
                         D3DMacroArray.GetBasePointer(),
                         &includeInterface,
                         m_pFrontend->GetStageEntryPoint(stage),
                         pPlatformProfileInfo->StageProfiles[stage],
                         D3DCompileFlags,
                         0,
                         &pCodeBlob,
                         &pErrorBlob);

    if (FAILED(hResult))
    {
        Log_ErrorPrintf("CompileShaderStage: D3DCompile for %s failed with HResult %08X", pPlatformProfileInfo->StageProfiles[stage], hResult);
        if (pErrorBlob)
        {
            Log_ErrorPrint((const char *)pErrorBlob->GetBufferPointer());
            m_errorLogWriter.WriteLine((const char *)pErrorBlob->GetBufferPointer(), pErrorBlob->GetBufferSize());
            pErrorBlob->Release();
        }

        return false;
    }

    if (pErrorBlob)
    {
        m_infoLogWriter.WriteLine("CompileShaderStage: Compile succeeded with warnings.");
        m_infoLogWriter.WriteLine((const char *)pErrorBlob->GetBufferPointer(), (uint32)pErrorBlob->GetBufferSize());

        Log_WarningPrintf("CompileShaderStage: Compile succeeded with warnings.");
        Log_WarningPrint((const char *)pErrorBlob->GetBufferPointer());
        pErrorBlob->Release();
    }

    if (m_pErrorLogStream)
    {
        ID3DBlob *pDisasmBlob;
        hResult = D3DDisassemble(pCodeBlob->GetBufferPointer(), pCodeBlob->GetBufferSize(), D3D_DISASM_ENABLE_INSTRUCTION_NUMBERING, nullptr, &pDisasmBlob);
        if (FAILED(hResult))
        {
            Log_WarningPrintf("D3DDisassemble failed with hResult %08X", hResult);
            m_infoLogWriter.WriteFormattedLine("D3DDisassemble failed with hResult %08X", hResult);
        }
        else
        {
            m_infoLogWriter.WriteFormattedLine("%s shader disassembly:", pPlatformProfileInfo->StageProfiles[stage]);
            m_infoLogWriter.Write((const char *)pDisasmBlob->GetBufferPointer(), (uint32)pDisasmBlob->GetBufferSize());
            pDisasmBlob->Release();
        }
    }

    // set blob
    Log_DevPrintf("CompileShaderStage: %s compiled to %u bytes.", pPlatformProfileInfo->StageProfiles[stage], pCodeBlob->GetBufferSize());
    m_pStageByteCode[stage] = pCodeBlob;
    return true;
}

static bool MapD3D11ShaderTypeDescToParameterType(const D3D11_SHADER_TYPE_DESC *pVariableTypeDesc, SHADER_PARAMETER_TYPE *pParameterType)
{
    struct TypeTranslation { SHADER_PARAMETER_TYPE UniformType; D3D_SHADER_VARIABLE_CLASS D3DClass; D3D_SHADER_VARIABLE_TYPE D3DType; uint32 Rows; uint32 Cols; };
    static const TypeTranslation TranslationTable[] =
    {
        { SHADER_PARAMETER_TYPE_BOOL,               D3D_SVC_SCALAR,             D3D_SVT_BOOL,           1,      1   },
        { SHADER_PARAMETER_TYPE_INT,                D3D_SVC_SCALAR,             D3D_SVT_INT,            1,      1   },
        { SHADER_PARAMETER_TYPE_INT2,               D3D_SVC_SCALAR,             D3D_SVT_INT,            1,      2   },
        { SHADER_PARAMETER_TYPE_INT3,               D3D_SVC_SCALAR,             D3D_SVT_INT,            1,      3   },
        { SHADER_PARAMETER_TYPE_INT4,               D3D_SVC_SCALAR,             D3D_SVT_INT,            1,      4   },
        { SHADER_PARAMETER_TYPE_UINT,               D3D_SVC_SCALAR,             D3D_SVT_UINT,           1,      1   },
        { SHADER_PARAMETER_TYPE_UINT2,              D3D_SVC_SCALAR,             D3D_SVT_UINT,           1,      2   },
        { SHADER_PARAMETER_TYPE_UINT3,              D3D_SVC_SCALAR,             D3D_SVT_UINT,           1,      3   },
        { SHADER_PARAMETER_TYPE_UINT4,              D3D_SVC_SCALAR,             D3D_SVT_UINT,           1,      4   },
        { SHADER_PARAMETER_TYPE_FLOAT,              D3D_SVC_SCALAR,             D3D_SVT_FLOAT,          1,      1   },
        { SHADER_PARAMETER_TYPE_FLOAT2,             D3D_SVC_VECTOR,             D3D_SVT_FLOAT,          1,      2   },
        { SHADER_PARAMETER_TYPE_FLOAT3,             D3D_SVC_VECTOR,             D3D_SVT_FLOAT,          1,      3   },
        { SHADER_PARAMETER_TYPE_FLOAT4,             D3D_SVC_VECTOR,             D3D_SVT_FLOAT,          1,      4   },
        { SHADER_PARAMETER_TYPE_FLOAT2X2,           D3D_SVC_MATRIX_ROWS,        D3D_SVT_FLOAT,          2,      2   },
        { SHADER_PARAMETER_TYPE_FLOAT3X3,           D3D_SVC_MATRIX_ROWS,        D3D_SVT_FLOAT,          3,      3   },
        { SHADER_PARAMETER_TYPE_FLOAT3X4,           D3D_SVC_MATRIX_ROWS,        D3D_SVT_FLOAT,          3,      4   },
        { SHADER_PARAMETER_TYPE_FLOAT4X4,           D3D_SVC_MATRIX_ROWS,        D3D_SVT_FLOAT,          4,      4   },
    };

    for (uint32 i = 0; i < countof(TranslationTable); i++)
    {
        const TypeTranslation *pCur = &TranslationTable[i];
        if (pCur->D3DClass == pVariableTypeDesc->Class &&
            pCur->D3DType == pVariableTypeDesc->Type &&
            pCur->Rows == pVariableTypeDesc->Rows &&
            pCur->Cols == pVariableTypeDesc->Columns)
        {
            *pParameterType = pCur->UniformType;
            return true;
        }
    }

    return false;
}

bool D3DShaderCompilerBackend::ReflectShader(SHADER_PROGRAM_STAGE stage)
{
    DebugAssert(m_pStageByteCode[stage] != nullptr);

    Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pReflection;
    HRESULT hResult = D3DReflect(m_pStageByteCode[stage]->GetBufferPointer(), m_pStageByteCode[stage]->GetBufferSize(), IID_ID3D11ShaderReflection, &pReflection);
    if (FAILED(hResult))
    {
        Log_ErrorPrintf("D3DReflect failed with hResult %08X", hResult);
        return false;
    }

    // get shader descriptor
    D3D11_SHADER_DESC shaderDesc;
    hResult = pReflection->GetDesc(&shaderDesc);
    DebugAssert(SUCCEEDED(hResult));

//     ID3DBlob *pDisassembly;
//     hResult = D3DDisassemble(pByteCode, byteCodeSize, 0, NULL, &pDisassembly);
//     DebugAssert(hResult == S_OK);
//     Log_DevPrint("Shader Disassembly");
//     Log_DevPrint((const char *)pDisassembly->GetBufferPointer());
//     pDisassembly->Release();
// 
    // parse uniforms
    if (shaderDesc.ConstantBuffers > 0)
    {
        ID3D11ShaderReflectionConstantBuffer* pGlobalConstantBufferReflection = pReflection->GetConstantBufferByName("$Globals");
        if (pGlobalConstantBufferReflection)
        {
            D3D11_SHADER_BUFFER_DESC constantBufferDesc;
            hResult = pGlobalConstantBufferReflection->GetDesc(&constantBufferDesc);
            DebugAssert(SUCCEEDED(hResult));

            // Set size, and find binding points for uniform buffer
            // TODO: Issues if the variables aren't the same across all stages...
            D3D11_SHADER_INPUT_BIND_DESC inputBindDesc;
            hResult = pReflection->GetResourceBindingDescByName("$Globals", &inputBindDesc);
            if (SUCCEEDED(hResult))
            {
                m_uniformBufferSize = Max(m_uniformBufferSize, constantBufferDesc.Size);
                m_uniformBufferBindPoints[stage] = inputBindDesc.BindPoint;
            }

            for (uint32 variableIndex = 0; variableIndex < constantBufferDesc.Variables; variableIndex++)
            {
                ID3D11ShaderReflectionVariable* pVariableReflection = pGlobalConstantBufferReflection->GetVariableByIndex(variableIndex);
                D3D11_SHADER_VARIABLE_DESC variableDesc;
                hResult = pVariableReflection->GetDesc(&variableDesc);
                DebugAssert(SUCCEEDED(hResult));

                // query variable type
                ID3D11ShaderReflectionType* pVariableType = pVariableReflection->GetType();
                D3D11_SHADER_TYPE_DESC typeDesc;
                hResult = pVariableType->GetDesc(&typeDesc);
                DebugAssert(SUCCEEDED(hResult));

                // skip over unused variables
                if (!(variableDesc.uFlags & D3D_SVF_USED))
                    continue;

                // have we already done this parameter?
                uint32 outUniformIndex;
                for (outUniformIndex = 0; outUniformIndex < m_outUniforms.GetSize(); outUniformIndex++)
                {
                    if (m_outUniformNames[outUniformIndex].Compare(variableDesc.Name))
                        break;
                }

                // skip already-done parameters
                if (outUniformIndex != m_outUniforms.GetSize())
                    continue;

                // handle structs
                SHADER_PARAMETER_TYPE parameterType;
                uint32 uniformSize;
                if (typeDesc.Class == D3D_SVC_STRUCT)
                {
                    // columns == size in floats?
                    parameterType = SHADER_PARAMETER_TYPE_STRUCT;
                    uniformSize = typeDesc.Columns * sizeof(float);
                }
                else
                {
                    // determine type
                    if (!MapD3D11ShaderTypeDescToParameterType(&typeDesc, &parameterType))
                    {
                        Log_ErrorPrintf("ReflectShader: Failed to map variable type");
                        return false;
                    }

                    // get type size
                    uniformSize = ShaderParameterValueTypeSize(parameterType);
                }

                // determine array size
                uint32 uniformArraySize = Max((uint32)1, (uint32)typeDesc.Elements);
                uint32 uniformArrayStride = uniformSize;
                if ((uniformArrayStride * uniformArraySize) != variableDesc.Size)
                {
                    // Basically, every row gets aligned to a float4.
                    // However it seems that the last element is not padded.
                    switch (parameterType)
                    {
                    case SHADER_PARAMETER_TYPE_BOOL:
                        uniformArraySize = sizeof(BOOL) * 4;
                        break;

                    case SHADER_PARAMETER_TYPE_INT:
                    case SHADER_PARAMETER_TYPE_INT2:
                    case SHADER_PARAMETER_TYPE_INT3:
                        uniformArrayStride = sizeof(int32) * 4;
                        break;

                    case SHADER_PARAMETER_TYPE_FLOAT:
                    case SHADER_PARAMETER_TYPE_FLOAT2:
                    case SHADER_PARAMETER_TYPE_FLOAT3:
                        uniformArrayStride = sizeof(float) * 4;
                        break;

                    default:
                        UnreachableCode();
                        break;
                    }

                    // should've fixed it
                    DebugAssert((uniformArrayStride * (uniformArraySize - 1) + ShaderParameterValueTypeSize(parameterType)) == variableDesc.Size);
                }

                // create parameter
                D3DShaderCacheEntryUniform uniformDecl;
                uniformDecl.NameLength = Y_strlen(variableDesc.Name);
                uniformDecl.Type = parameterType;
                uniformDecl.ArraySize = uniformArraySize;
                uniformDecl.ArrayStride = uniformArrayStride;
                uniformDecl.BufferOffset = variableDesc.StartOffset;
                m_outUniforms.Add(uniformDecl);
                m_outUniformNames.Add(variableDesc.Name);
            }            
        }
    }

    if (shaderDesc.BoundResources > 0)
    {
        // work out number of samplers/resources
        for (uint32 boundResourceIndex = 0; boundResourceIndex < shaderDesc.BoundResources; boundResourceIndex++)
        {
            D3D11_SHADER_INPUT_BIND_DESC inputBindDesc;
            hResult = pReflection->GetResourceBindingDesc(boundResourceIndex, &inputBindDesc);
            DebugAssert(SUCCEEDED(hResult));

            // Skip globals buffer (uniforms)
            if (inputBindDesc.Type == D3D_SIT_CBUFFER && Y_strcmp(inputBindDesc.Name, "$Globals") == 0)
                continue;

            switch (inputBindDesc.Type)
            {
                // Constant Buffer
            case D3D_SIT_CBUFFER:
                {
                    uint32 constantBufferIndex;
                    for (constantBufferIndex = 0; constantBufferIndex < m_outConstantBuffers.GetSize(); constantBufferIndex++)
                    {
                        if (m_outConstantBufferNames[constantBufferIndex].Compare(inputBindDesc.Name))
                            break;
                    }
                    if (constantBufferIndex == m_outConstantBuffers.GetSize())
                    {
                        D3DShaderCacheEntryConstantBuffer cbDesc;
                        cbDesc.NameLength = Y_strlen(inputBindDesc.Name);
                        cbDesc.Size = 0;
                        cbDesc.Slot = Y_UINT32_MAX;
                        for (uint32 i = 0; i < SHADER_PROGRAM_STAGE_COUNT; i++)
                            cbDesc.BindPoint[i] = -1;

                        ID3D11ShaderReflectionConstantBuffer* pConstantBufferReflection = pReflection->GetConstantBufferByName(inputBindDesc.Name);
                        if (pConstantBufferReflection)
                        {
                            D3D11_SHADER_BUFFER_DESC constantBufferDesc;
                            hResult = pConstantBufferReflection->GetDesc(&constantBufferDesc);
                            DebugAssert(SUCCEEDED(hResult));

                            cbDesc.Size = constantBufferDesc.Size;
                        }

                        cbDesc.BindPoint[stage] = inputBindDesc.BindPoint;
                        m_outConstantBuffers.Add(cbDesc);
                        m_outConstantBufferNames.Add(inputBindDesc.Name);
                    }
                    else
                    {
                        m_outConstantBuffers[constantBufferIndex].BindPoint[stage] = inputBindDesc.BindPoint;
                    }
                }
                break;

                // Sampler
            case D3D_SIT_SAMPLER:
                {
                    uint32 samplerIndex;
                    for (samplerIndex = 0; samplerIndex < m_outSamplers.GetSize(); samplerIndex++)
                    {
                        if (m_outSamplerNames[samplerIndex].Compare(inputBindDesc.Name))
                            break;
                    }
                    if (samplerIndex == m_outSamplers.GetSize())
                    {
                        D3DShaderCacheEntrySampler cbDesc;
                        cbDesc.NameLength = Y_strlen(inputBindDesc.Name);
                        cbDesc.Slot = Y_UINT32_MAX;
                        for (uint32 i = 0; i < SHADER_PROGRAM_STAGE_COUNT; i++)
                            cbDesc.BindPoint[i] = -1;

                        cbDesc.BindPoint[stage] = inputBindDesc.BindPoint;
                        m_outSamplers.Add(cbDesc);
                        m_outSamplerNames.Add(inputBindDesc.Name);
                    }
                    else
                    {
                        m_outSamplers[samplerIndex].BindPoint[stage] = inputBindDesc.BindPoint;
                    }
                }
                break;

                // Resource
            case D3D_SIT_TEXTURE:
            case D3D_SIT_TBUFFER:
            case D3D_SIT_STRUCTURED:
                {
                    uint32 resourceIndex;
                    for (resourceIndex = 0; resourceIndex < m_outResources.GetSize(); resourceIndex++)
                    {
                        if (m_outResourceNames[resourceIndex].Compare(inputBindDesc.Name))
                            break;
                    }
                    if (resourceIndex == m_outResources.GetSize())
                    {
                        D3DShaderCacheEntryResource resDesc;
                        resDesc.NameLength = Y_strlen(inputBindDesc.Name);
                        resDesc.Slot = Y_UINT32_MAX;
                        for (uint32 i = 0; i < SHADER_PROGRAM_STAGE_COUNT; i++)
                            resDesc.BindPoint[i] = -1;

                        if (inputBindDesc.Type == D3D_SIT_TEXTURE)
                        {
                            switch (inputBindDesc.Dimension)
                            {
                            case D3D_SRV_DIMENSION_BUFFER:
                            case D3D_SRV_DIMENSION_BUFFEREX:
                                resDesc.Type = GPU_RESOURCE_TYPE_TEXTUREBUFFER;
                                break;

                            case D3D_SRV_DIMENSION_TEXTURE1D:
                                resDesc.Type = GPU_RESOURCE_TYPE_TEXTURE1D;
                                break;

                            case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
                                resDesc.Type = GPU_RESOURCE_TYPE_TEXTURE1DARRAY;
                                break;

                            case D3D_SRV_DIMENSION_TEXTURE2D:
                                resDesc.Type = GPU_RESOURCE_TYPE_TEXTURE2D;
                                break;

                            case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
                                resDesc.Type = GPU_RESOURCE_TYPE_TEXTURE2DARRAY;
                                break;

                            case D3D_SRV_DIMENSION_TEXTURE3D:
                                resDesc.Type = GPU_RESOURCE_TYPE_TEXTURE3D;
                                break;

                            case D3D_SRV_DIMENSION_TEXTURECUBE:
                                resDesc.Type = GPU_RESOURCE_TYPE_TEXTURECUBE;
                                break;

                            case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
                                resDesc.Type = GPU_RESOURCE_TYPE_TEXTURECUBEARRAY;
                                break;

                            default:
                                m_errorLogWriter.WriteFormattedLine("Unhandled input dimension %u", inputBindDesc.Dimension);
                                return false;
                            }
                        }
                        else
                        {
                            // Everything else is a buffer type.
                            resDesc.Type = GPU_RESOURCE_TYPE_BUFFER;
                        }

                        resDesc.BindPoint[stage] = inputBindDesc.BindPoint;
                        m_outResources.Add(resDesc);
                        m_outResourceNames.Add(inputBindDesc.Name);
                    }
                    else
                    {
                        m_outResources[resourceIndex].BindPoint[stage] = inputBindDesc.BindPoint;
                    }
                }
                break;

                // Resource
            case D3D_SIT_UAV_RWTYPED:
            case D3D_SIT_UAV_RWSTRUCTURED:
            case D3D_SIT_UAV_RWBYTEADDRESS:
            case D3D_SIT_UAV_APPEND_STRUCTURED:
            case D3D_SIT_UAV_CONSUME_STRUCTURED:
            case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
                {
                    uint32 rwResourceIndex;
                    for (rwResourceIndex = 0; rwResourceIndex < m_outRWResources.GetSize(); rwResourceIndex++)
                    {
                        if (m_outRWResourceNames[rwResourceIndex].Compare(inputBindDesc.Name))
                            break;
                    }
                    if (rwResourceIndex == m_outRWResources.GetSize())
                    {
                        D3DShaderCacheEntryRWResource resDesc;
                        resDesc.NameLength = Y_strlen(inputBindDesc.Name);
                        resDesc.Slot = Y_UINT32_MAX;
                        for (uint32 i = 0; i < SHADER_PROGRAM_STAGE_COUNT; i++)
                            resDesc.BindPoint[i] = -1;

                        if (inputBindDesc.Type == D3D_SIT_UAV_RWTYPED)
                        {
                            switch (inputBindDesc.Dimension)
                            {
                            case D3D_SRV_DIMENSION_BUFFER:
                            case D3D_SRV_DIMENSION_BUFFEREX:
                                resDesc.Type = GPU_RESOURCE_TYPE_BUFFER;
                                break;

                            case D3D_SRV_DIMENSION_TEXTURE1D:
                                resDesc.Type = GPU_RESOURCE_TYPE_TEXTURE1D;
                                break;

                            case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
                                resDesc.Type = GPU_RESOURCE_TYPE_TEXTURE1DARRAY;
                                break;

                            case D3D_SRV_DIMENSION_TEXTURE2D:
                                resDesc.Type = GPU_RESOURCE_TYPE_TEXTURE2D;
                                break;

                            case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
                                resDesc.Type = GPU_RESOURCE_TYPE_TEXTURE2DARRAY;
                                break;

                            case D3D_SRV_DIMENSION_TEXTURE3D:
                                resDesc.Type = GPU_RESOURCE_TYPE_TEXTURE3D;
                                break;

                            case D3D_SRV_DIMENSION_TEXTURECUBE:
                                resDesc.Type = GPU_RESOURCE_TYPE_TEXTURECUBE;
                                break;

                            case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
                                resDesc.Type = GPU_RESOURCE_TYPE_TEXTURECUBEARRAY;
                                break;

                            default:
                                m_errorLogWriter.WriteFormattedLine("Unhandled input dimension %u", inputBindDesc.Dimension);
                                return false;
                            }
                        }
                        else
                        {
                            // Everything else is a buffer type.
                            resDesc.Type = GPU_RESOURCE_TYPE_BUFFER;
                        }

                        resDesc.BindPoint[stage] = inputBindDesc.BindPoint;
                        m_outRWResources.Add(resDesc);
                        m_outRWResourceNames.Add(inputBindDesc.Name);
                    }
                    else
                    {
                        m_outRWResources[rwResourceIndex].BindPoint[stage] = inputBindDesc.BindPoint;
                    }
                }
                break;

            default:
                m_errorLogWriter.WriteFormattedLine("Unhandled input type %u", inputBindDesc.Dimension);
                return false;
            }
        }
    }

    return true;
}

template<typename T>
static void InternalAssignResourceSlots(T& resources)
{
    // Assign everything that has common input slots first.
    for (auto& desc : resources)
    {
        int32 slot = -1;
        for (uint32 i = 0; i < SHADER_PROGRAM_STAGE_COUNT; i++)
        {
            if (desc.BindPoint[i] < 0)
                continue;

            if (slot < 0)
            {
                slot = desc.BindPoint[i];
                continue;
            }

            if (slot != desc.BindPoint[i])
            {
                // Bound to different registers in different stages.
                slot = -1;
                break;
            }
        }

        if (slot >= 0)
        {
            for (const auto& otherDesc : resources)
            {
                if (otherDesc.Slot == (uint32)slot)
                {
                    // Another resource is using this slot
                    slot = -1;
                    break;
                }
            }
        }

        if (slot >= 0)
        {
            // Assign slot
            desc.Slot = (uint32)slot;
        }
    }

    // Assign conflicting resources.
    for (auto& desc : resources)
    {
        if (desc.Slot != Y_UINT32_MAX)
            continue;

        desc.Slot = 0;
        for (;;)
        {
            bool hasConflict = true;
            for (const auto& otherDesc : resources)
            {
                if (otherDesc.Slot == desc.Slot)
                {
                    hasConflict = true;
                    break;
                }
            }

            if (!hasConflict)
                break;

            desc.Slot++;
        }
    }
}

void D3DShaderCompilerBackend::AssignResourceSlots()
{
    InternalAssignResourceSlots(m_outConstantBuffers);
    InternalAssignResourceSlots(m_outSamplers);
    InternalAssignResourceSlots(m_outResources);
    InternalAssignResourceSlots(m_outRWResources);
}

void D3DShaderCompilerBackend::LinkResourceSamplers()
{
    SmallString samplerParameterName;

    // find any resources with a matching _SamplerState suffix sampler
    for (uint32 resourceIndex = 0; resourceIndex < m_outResources.GetSize(); resourceIndex++)
    {
        D3DShaderCacheEntryResource* resource = &m_outResources[resourceIndex];
        if (resource->Type >= GPU_RESOURCE_TYPE_TEXTURE1D && resource->Type <= GPU_RESOURCE_TYPE_TEXTURECUBEARRAY)
        {
            samplerParameterName.Clear();
            samplerParameterName.AppendString(m_outResourceNames[resourceIndex]);
            samplerParameterName.AppendString("_SamplerState");

            for (uint32 samplerIndex = 0; samplerIndex < m_outSamplers.GetSize(); samplerIndex++)
            {
                if (m_outSamplerNames[samplerIndex].Compare(samplerParameterName))
                {
                    // found a match
                    m_outResources[resourceIndex].LinkedSamplerIndex = samplerIndex;
                    Log_DevPrintf("LinkResourceSamplerParameters: Linked '%s' to '%s'", m_outResourceNames[resourceIndex].GetCharArray(), m_outSamplerNames[samplerIndex].GetCharArray());
                    break;
                }
            }
        }
    }
}

bool D3DShaderCompilerBackend::Compile()
{
    SmallString tempString;
    Timer compileTimer, stageCompileTimer;
    float compileTimeCompile, compileTimeReflect;

    // compile stages
    for (uint32 stageIndex = 0; stageIndex < SHADER_PROGRAM_STAGE_COUNT; stageIndex++)
    {
        if (!CompileShaderStage((SHADER_PROGRAM_STAGE)stageIndex))
        {
            Log_ErrorPrintf("Failed to compile stage %u.", stageIndex);
            return false;
        }
    }
    compileTimeCompile = (float)stageCompileTimer.GetTimeMilliseconds();
    stageCompileTimer.Reset();

    // Reflect each present shader stage.
    for (uint32 stageIndex = 0; stageIndex < SHADER_PROGRAM_STAGE_COUNT; stageIndex++)
    {
        if (m_pStageByteCode[stageIndex] != nullptr && !ReflectShader((SHADER_PROGRAM_STAGE)stageIndex))
        {
            Log_ErrorPrintf("Failed to reflect stage %u.", stageIndex);
            return false;
        }
    }

    // Assign resource slots
    AssignResourceSlots();

    // Link sampler states to resources
    LinkResourceSamplers();

#if 1
    // Dump out variables
    for (uint32 samplerIndex = 0; samplerIndex < m_outUniforms.GetSize(); samplerIndex++)
        Log_DevPrintf("Shader Uniform [%u] : %s, type %s", samplerIndex, m_outUniformNames[samplerIndex].GetCharArray(), NameTable_GetNameString(NameTables::ShaderParameterType, m_outUniforms[samplerIndex].Type));
    for (uint32 constantBufferIndex = 0; constantBufferIndex < m_outConstantBuffers.GetSize(); constantBufferIndex++)
        Log_DevPrintf("Shader Constant Buffer [%u] : %s, %u bytes", constantBufferIndex, m_outConstantBufferNames[constantBufferIndex].GetCharArray(), m_outConstantBuffers[constantBufferIndex].Size);
    for (uint32 samplerIndex = 0; samplerIndex < m_outSamplers.GetSize(); samplerIndex++)
        Log_DevPrintf("Shader Sampler [%u] : %s", samplerIndex, m_outSamplerNames[samplerIndex].GetCharArray());
    for (uint32 resourceIndex = 0; resourceIndex < m_outResources.GetSize(); resourceIndex++)
        Log_DevPrintf("Shader Resource [%u] : %s (%s)", resourceIndex, m_outResourceNames[resourceIndex].GetCharArray(), NameTable_GetNameString(NameTables::GPUResourceType, m_outResources[resourceIndex].Type));
    for (uint32 rwResourceIndex = 0; rwResourceIndex < m_outRWResources.GetSize(); rwResourceIndex++)
        Log_DevPrintf("Shader Resource [%u] : %s (%s)", rwResourceIndex, m_outRWResourceNames[rwResourceIndex].GetCharArray(), NameTable_GetNameString(NameTables::GPUResourceType, m_outRWResources[rwResourceIndex].Type));
#endif

    // Create the header of the shader data.
    // The zeros on the strings prevent stack memory getting dropped into the output.
    D3DShaderCacheEntryHeader cacheEntryHeader;
    Y_memzero(&cacheEntryHeader, sizeof(cacheEntryHeader));
    cacheEntryHeader.Signature = D3D_SHADER_CACHE_ENTRY_HEADER;

    // Store shader stage sizes.
    for (uint32 stageIndex = 0; stageIndex < SHADER_PROGRAM_STAGE_COUNT; stageIndex++)
        cacheEntryHeader.StageSize[stageIndex] = (m_pStageByteCode[stageIndex] != nullptr) ? m_pStageByteCode[stageIndex]->GetBufferSize() : 0;

    // Store counts.
    cacheEntryHeader.UniformCount = m_outUniforms.GetSize();
    Y_memcpy(cacheEntryHeader.UniformBufferBindPoints, m_uniformBufferBindPoints, sizeof(cacheEntryHeader.UniformBufferBindPoints));
    cacheEntryHeader.UniformBufferSize = m_uniformBufferSize;
    cacheEntryHeader.ConstantBufferCount = m_outConstantBuffers.GetSize();
    cacheEntryHeader.SamplerCount = m_outSamplers.GetSize();
    cacheEntryHeader.ResourceCount = m_outResources.GetSize();
    cacheEntryHeader.RWResourceCount = m_outRWResources.GetSize();

    // Write header.
    m_outputWriter.WriteBytes(&cacheEntryHeader, sizeof(cacheEntryHeader));

    // Write each of the shader stage's bytecodes out.
    for (uint32 stageIndex = 0; stageIndex < SHADER_PROGRAM_STAGE_COUNT; stageIndex++)
    {
        if (m_pStageByteCode[stageIndex] != nullptr)
        {
            // write stage bytecode
            m_outputWriter.WriteBytes(m_pStageByteCode[stageIndex]->GetBufferPointer(), m_pStageByteCode[stageIndex]->GetBufferSize());
        }
    }

    // Write out declarations.
    for (uint32 i = 0; i < m_outUniforms.GetSize(); i++)
    {
        m_outputWriter.WriteBytes(&m_outUniforms[i], sizeof(D3DShaderCacheEntryUniform));
        m_outputWriter.WriteFixedString(m_outUniformNames[i], m_outUniforms[i].NameLength);
    }
    for (uint32 i = 0; i < m_outConstantBuffers.GetSize(); i++)
    {
        m_outputWriter.WriteBytes(&m_outConstantBuffers[i], sizeof(D3DShaderCacheEntryConstantBuffer));
        m_outputWriter.WriteFixedString(m_outConstantBufferNames[i], m_outConstantBuffers[i].NameLength);
    }
    for (uint32 i = 0; i < m_outSamplers.GetSize(); i++)
    {
        m_outputWriter.WriteBytes(&m_outSamplers[i], sizeof(D3DShaderCacheEntrySampler));
        m_outputWriter.WriteFixedString(m_outSamplerNames[i], m_outSamplers[i].NameLength);
    }
    for (uint32 i = 0; i < m_outResources.GetSize(); i++)
    {
        m_outputWriter.WriteBytes(&m_outResources[i], sizeof(D3DShaderCacheEntryResource));
        m_outputWriter.WriteFixedString(m_outResourceNames[i], m_outResources[i].NameLength);
    }
    for (uint32 i = 0; i < m_outRWResources.GetSize(); i++)
    {
        m_outputWriter.WriteBytes(&m_outRWResources[i], sizeof(D3DShaderCacheEntryRWResource));
        m_outputWriter.WriteFixedString(m_outRWResourceNames[i], m_outRWResources[i].NameLength);
    }

    // update timer
    compileTimeReflect = (float)stageCompileTimer.GetTimeMilliseconds();

    // Compile succeded.
    Log_DevPrintf("Shader successfully compiled. Took %.3f msec (Compile: %.3f msec, Reflect+Write: %.3f msec)",
        (float)compileTimer.GetTimeMilliseconds(), compileTimeCompile, compileTimeReflect);

    return true;
}

ShaderCompilerBackend* ShaderCompilerBackend::CreateD3DBackend(const ShaderCompiler* pFrontend, SHADER_PROGRAM_BYTECODE_TYPE type, uint32 compileFlags, ByteStream* pOutputStream, ByteStream* pErrorLogStream, ByteStream* pInfoLogStream)
{
    return new D3DShaderCompilerBackend(pFrontend, type, compileFlags, pOutputStream, pErrorLogStream, pInfoLogStream);
}

#else       // Y_PLATFORM_WINDOWS

ShaderCompilerBackend* ShaderCompilerBackend::CreateD3DBackend(const ShaderCompiler* pFrontend, SHADER_PROGRAM_BYTECODE_TYPE type, uint32 compileFlags, ByteStream* pOutputStream, ByteStream* pErrorLogStream, ByteStream* pInfoLogStream)
{
    return nullptr;
}

#endif      // Y_PLATFORM_WINDOWS

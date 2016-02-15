#pragma once
#include "YRenderLib/Common.h"
#include "YRenderLib/RendererTypes.h"

struct SHADER_BLOB_HEADER
{
    uint64 Magic;
    uint32 Version;

    uint32 BytecodeOffsets[NUM_SHADER_PROGRAM_BYTECODE_TYPES];
    uint32 BytecodeSizes[NUM_SHADER_PROGRAM_BYTECODE_TYPES];

    uint32 SourceOffset;
    uint32 SourceSize;
};

struct SHADER_BLOB_SOURCE_CHUNK
{
    uint32 CompileFlags;
    uint32 StageSourceCodeLength[SHADER_PROGRAM_STAGE_COUNT];
    uint32 StageEntryPointLengths[SHADER_PROGRAM_STAGE_COUNT];
    // for i in 0..SHADER_PROGRAM_STAGE_COUNT
    //     <char> * sourceCodeLength[i]
    //     <char> * entryPointLength[i]
};

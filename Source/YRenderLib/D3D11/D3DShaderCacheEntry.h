#pragma once

#pragma pack(push, 1)

#define D3D_SHADER_CACHE_ENTRY_HEADER ((uint32)'D3DS')
#define D3D_SHADER_CACHE_ENTRY_MAX_NAME_LENGTH 256

struct D3DShaderCacheEntryHeader
{
    uint32 Signature;

    uint32 StageSize[SHADER_PROGRAM_STAGE_COUNT];

    uint32 UniformCount;
    uint32 UniformBufferSize;
    int32 UniformBufferBindPoints[SHADER_PROGRAM_STAGE_COUNT];

    uint32 ConstantBufferCount;
    uint32 SamplerCount;
    uint32 ResourceCount;
    uint32 RWResourceCount;
};

struct D3DShaderCacheEntryUniform
{
    uint32 NameLength;
    uint32 Type;
    uint32 ArraySize;
    uint32 ArrayStride;
    uint32 BufferOffset;
    // <char> * NameLength follows
};

struct D3DShaderCacheEntryConstantBuffer
{
    uint32 NameLength;
    uint32 Slot;
    uint32 Size;
    int32 BindPoint[SHADER_PROGRAM_STAGE_COUNT];
    // <char> * NameLength follows
};

struct D3DShaderCacheEntrySampler
{
    uint32 NameLength;
    uint32 Slot;
    int32 BindPoint[SHADER_PROGRAM_STAGE_COUNT];
    // <char> * NameLength follows
};

struct D3DShaderCacheEntryResource
{
    uint32 NameLength;
    uint32 Slot;
    uint32 Type;
    int32 BindPoint[SHADER_PROGRAM_STAGE_COUNT];
    int32 LinkedSamplerIndex;
    // <char> * NameLength follows
};

struct D3DShaderCacheEntryRWResource
{
    uint32 NameLength;
    uint32 Slot;
    uint32 Type;
    int32 BindPoint[SHADER_PROGRAM_STAGE_COUNT];
    // <char> * NameLength follows
};


#pragma pack(pop)


#include "YBaseLib/Assert.h"
#include "YRenderLib/Renderer.h"
#include "YRenderLib/Util.h"

bool Util::TextureFilterRequiresMips(TEXTURE_FILTER filter)
{
    switch (filter)
    {
    case TEXTURE_FILTER_MIN_MAG_POINT_MIP_LINEAR:
    case TEXTURE_FILTER_MIN_POINT_MAG_MIP_LINEAR:
    case TEXTURE_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
    case TEXTURE_FILTER_MIN_MAG_MIP_LINEAR:
    case TEXTURE_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR:
    case TEXTURE_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR:
    case TEXTURE_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
    case TEXTURE_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR:
        return true;

    default:
        return false;
    }
}

uint32 Util::CalculateMipCount(uint32 width, uint32 height /* = 1 */, uint32 depth /* = 1 */)
{
    uint32 currentWidth = width;
    uint32 currentHeight = height;
    uint32 currentDepth = depth;
    uint32 mipCount = 1;

    // TODO: use log2 instead
    while (currentWidth > 1 || currentHeight > 1 || currentDepth > 1)
    {
        if (currentWidth > 1)
            currentWidth /= 2;
        if (currentHeight > 1)
            currentHeight /= 2;
        if (currentDepth > 1)
            currentDepth /= 2;

        mipCount++;
    }

    return mipCount;
}

void Util::GetClipSpaceCoordinatesForPixelCoordinates(float* outX, float* outY, int32 x, int32 y, uint32 windowWidth, uint32 windowHeight)
{
    DebugAssert(windowWidth > 1 && windowHeight > 1);

    float csX = (float)x / (float)(windowWidth - 1);
    float csY = (float)y / (float)(windowHeight - 1);
    
    csX *= 2.0f;
    csY *= 2.0f;
    csX -= 1.0f;
    csY = 1.0f - csY;

    *outX = csX;
    *outY = csY;
}

void Util::GetTextureSpaceCoordinatesForPixelCoordinates(float* outU, float* outV, int32 x, int32 y, uint32 textureWidth, uint32 textureHeight)
{
    DebugAssert(textureWidth > 1 && textureHeight > 1);

    *outU = (float)x / (float)(textureWidth - 1);
    *outV = (float)y / (float)(textureHeight - 1);
}

void Util::GetTextureDimensions(GPUTexture* pTexture, uint32 *pWidth, uint32 *pHeight, uint32 *pDepth)
{
    switch (pTexture->GetResourceType())
    {
    case GPU_RESOURCE_TYPE_TEXTURE1D:
        *pWidth = static_cast<GPUTexture1D*>(pTexture)->GetDesc()->Width;
        *pHeight = 1;
        *pDepth = 1;
        return;

    case GPU_RESOURCE_TYPE_TEXTURE1DARRAY:
        *pWidth = static_cast<GPUTexture1DArray*>(pTexture)->GetDesc()->Width;
        *pHeight = 1;
        *pDepth = static_cast<GPUTexture1DArray*>(pTexture)->GetDesc()->ArraySize;
        return;

    case GPU_RESOURCE_TYPE_TEXTURE2D:
        *pWidth = static_cast<GPUTexture2D*>(pTexture)->GetDesc()->Width;
        *pHeight = static_cast<GPUTexture2D*>(pTexture)->GetDesc()->Height;
        *pDepth = 1;
        return;

    case GPU_RESOURCE_TYPE_TEXTURE2DARRAY:
        *pWidth = static_cast<GPUTexture2DArray*>(pTexture)->GetDesc()->Width;
        *pHeight = static_cast<GPUTexture2DArray*>(pTexture)->GetDesc()->Height;
        *pDepth = static_cast<GPUTexture2DArray*>(pTexture)->GetDesc()->ArraySize;
        return;

    case GPU_RESOURCE_TYPE_TEXTURE3D:
        *pWidth = static_cast<GPUTexture3D*>(pTexture)->GetDesc()->Width;
        *pHeight = static_cast<GPUTexture3D*>(pTexture)->GetDesc()->Height;
        *pDepth = static_cast<GPUTexture3D*>(pTexture)->GetDesc()->Depth;
        return;

    case GPU_RESOURCE_TYPE_TEXTURECUBE:
        *pWidth = static_cast<GPUTextureCube*>(pTexture)->GetDesc()->Width;
        *pHeight = static_cast<GPUTextureCube*>(pTexture)->GetDesc()->Height;
        *pDepth = 1;
        return;

    case GPU_RESOURCE_TYPE_TEXTURECUBEARRAY:
        *pWidth = static_cast<GPUTextureCubeArray*>(pTexture)->GetDesc()->Width;
        *pHeight = static_cast<GPUTextureCubeArray*>(pTexture)->GetDesc()->Height;
        *pDepth = static_cast<GPUTextureCubeArray*>(pTexture)->GetDesc()->ArraySize;
        return;
    }

    Panic("Unknown resource type");
}

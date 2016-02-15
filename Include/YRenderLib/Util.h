#pragma once
#include "YBaseLib/Common.h"
#include "YRenderLib/RendererTypes.h"

namespace Util {
    
    bool TextureFilterRequiresMips(TEXTURE_FILTER filter);

    uint32 CalculateMipCount(uint32 width, uint32 height = 1, uint32 depth = 1);

    void GetClipSpaceCoordinatesForPixelCoordinates(float* outX, float* outY, int32 x, int32 y, uint32 windowWidth, uint32 windowHeight);

    void GetTextureSpaceCoordinatesForPixelCoordinates(float* outU, float* outV, int32 x, int32 y, uint32 textureWidth, uint32 textureHeight);

    void GetTextureDimensions(GPUTexture* pTexture, uint32 *pWidth, uint32 *pHeight, uint32 *pDepth);
}

#ifndef __TEXTURE_UTILS_H__
#define __TEXTURE_UTILS_H__

#include "DAVAEngine.h"

class TextureUtils
{
public:
    struct CompareResult
    {
        DAVA::uint32 difference;
        DAVA::uint32 bytesCount;
    };

    static DAVA::Sprite* CreateSpriteFromTexture(const DAVA::String& texturePathname);
    static CompareResult CompareImages(const DAVA::Image* first, const DAVA::Image* second, DAVA::PixelFormat format);
    static DAVA::Image* CreateImageAsRGBA8888(DAVA::Sprite* sprite);
};

#endif // __TEXTURE_UTILS_H__

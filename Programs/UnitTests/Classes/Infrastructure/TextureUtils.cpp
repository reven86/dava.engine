#include "TextureUtils.h"
#include "Render/PixelFormatDescriptor.h"

using namespace DAVA;

Sprite* TextureUtils::CreateSpriteFromTexture(const String& texturePathname)
{
    Sprite* createdSprite = NULL;

    Texture* texture = Texture::CreateFromFile(texturePathname);
    if (texture)
    {
        createdSprite = Sprite::CreateFromTexture(texture, 0, 0,
                                                  static_cast<float32>(texture->GetWidth()),
                                                  static_cast<float32>(texture->GetHeight()));
        texture->Release();
    }

    return createdSprite;
}

TextureUtils::CompareResult TextureUtils::CompareImages(const Image* first, const Image* second, PixelFormat format)
{
    CompareResult compareResult = { 0 };

    if (first->GetWidth() != second->GetWidth() ||
        first->GetHeight() != second->GetHeight())
    {
        DVASSERT(false, "Can't compare images of different dimensions.");

        compareResult.difference = 100;
        return compareResult;
    }

    uint32 step = 1;
    uint32 startIndex = 0;

    compareResult.bytesCount = first->dataSize;
    if (FORMAT_A8 == format)
    {
        step = 4;
        startIndex = 3;
    }

    for (uint32 i = startIndex; i < compareResult.bytesCount; i += step)
    {
        compareResult.difference += abs(first->GetData()[i] - second->GetData()[i]);
    }

    return compareResult;
}

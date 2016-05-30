#include "TextureUtils.h"
#include "Render/PixelFormatDescriptor.h"

Sprite* TextureUtils::CreateSpriteFromTexture(const String& texturePathname)
{
    Sprite* createdSprite = NULL;

    Texture* texture = Texture::CreateFromFile(texturePathname);
    if (texture)
    {
        createdSprite = Sprite::CreateFromTexture(texture, 0, 0, (float32)texture->GetWidth(), (float32)texture->GetHeight());
        texture->Release();
    }

    return createdSprite;
}

TextureUtils::CompareResult TextureUtils::CompareSprites(Sprite* first, Sprite* second, PixelFormat format)
{
    /*
    DVASSERT(false);
    DebugBreak();
    __debugbreak();
    */
    DVASSERT(first->GetHeight() == second->GetHeight());
    DVASSERT(first->GetWidth() == second->GetWidth());

    Image* firstComparer = CreateImageAsRGBA8888(first);
    Image* secondComparer = CreateImageAsRGBA8888(second);

    CompareResult compareResult = { 0 };

    compareResult = CompareImages(firstComparer, secondComparer, format);

    //    String documentsPath = FileSystem::Instance()->GetCurrentDocumentsDirectory();
    //    firstComparer->Save(documentsPath + Format("PVRTest/src_number_%d.png", currentTest));
    //    secondComparer->Save(documentsPath + Format("PVRTest/dst_number_%d.png", currentTest));

    SafeRelease(firstComparer);
    SafeRelease(secondComparer);
    return compareResult;
}

TextureUtils::CompareResult TextureUtils::CompareImages(Image* first, Image* second, PixelFormat format)
{
    CompareResult compareResult = { 0 };

    if (first->GetWidth() != second->GetWidth() ||
        first->GetHeight() != second->GetHeight())
    {
        DVASSERT_MSG(false, "Can't compare images of different dimensions.");

        compareResult.difference = 100;
        return compareResult;
    }

    int32 imageSizeInBytes = (int32)(first->GetWidth() * first->GetHeight() * PixelFormatDescriptor::GetPixelFormatSizeInBytes(first->format));

    int32 step = 1;
    int32 startIndex = 0;

    if (FORMAT_A8 == format)
    {
        compareResult.bytesCount = (int32)(first->GetWidth() * first->GetHeight() * PixelFormatDescriptor::GetPixelFormatSizeInBytes(FORMAT_A8));
        step = 4;
        startIndex = 3;
    }
    else
    {
        compareResult.bytesCount = imageSizeInBytes;
    }

    for (int32 i = startIndex; i < imageSizeInBytes; i += step)
    {
        compareResult.difference += abs(first->GetData()[i] - second->GetData()[i]);
    }

    return compareResult;
}

Image* TextureUtils::CreateImageAsRGBA8888(Sprite* sprite)
{
#if RHI_COMPLETE_TESTBED
    Rect oldViewport = RenderManager::Instance()->GetViewport();
    Vector2 targetSize = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(sprite->GetSize());
    Texture* fbo = Texture::CreateFBO((uint32)targetSize.dx, (uint32)targetSize.dy, FORMAT_RGBA8888, Texture::DEPTH_NONE);

    RenderManager::Instance()->SetRenderTarget(fbo);
    RenderManager::Instance()->SetViewport(Rect(Vector2(), targetSize));
    RenderSystem2D::Instance()->Draw(sprite);

    RenderManager::Instance()->SetRenderTarget(0);
    RenderManager::Instance()->SetViewport(oldViewport);

    Image* resultImage = fbo->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);

    SafeRelease(fbo);
    return resultImage;
#else
    return nullptr;
#endif //RHI_COMPLETE_TESTBED
}

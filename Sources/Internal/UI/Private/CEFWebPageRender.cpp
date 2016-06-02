#pragma once

#if defined(ENABLE_CEF_WEBVIEW)

#include "UI/Private/CEFWebPageRender.h"
#include "Platform/DeviceInfo.h"
#include "Platform/SystemTimer.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

namespace DAVA
{
struct CEFColor
{
    CEFColor() = default;
    CEFColor(const Color& davaColor)
        : red(static_cast<uint8>(davaColor.r * 255.0f))
        , green(static_cast<uint8>(davaColor.g * 255.0f))
        , blue(static_cast<uint8>(davaColor.b * 255.0f))
        , alpha(static_cast<uint8>(davaColor.a * 255.0f))
    {
    }

    bool IsTransparent()
    {
        return (red | green | blue | alpha) == 0;
    }

    uint8 red = 0;
    uint8 green = 0;
    uint8 blue = 0;
    uint8 alpha = 0;
};

CEFWebPageRender::CEFWebPageRender(UIControl& target)
    : targetControl(target)
    , contentBackground(new UIControlBackground)
{
    contentBackground->SetDrawType(UIControlBackground::DRAW_STRETCH_BOTH);
    contentBackground->SetColor(Color::White);
}

void CEFWebPageRender::ClearRenderSurface()
{
    if (imageData)
    {
        ::memset(imageData.get(), 0, imageWidth * imageHeight * 4);
        AppyTexture();
    }
}

UIControlBackground* CEFWebPageRender::GetContentBackground()
{
    return contentBackground.Get();
}

void CEFWebPageRender::SetBackgroundTransparency(bool value)
{
    transparency = value;
}

bool CEFWebPageRender::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    VirtualCoordinatesSystem* vcs = VirtualCoordinatesSystem::Instance();

    Vector2 size = targetControl.GetSize();
    float32 width = vcs->ConvertVirtualToPhysicalX(size.dx);
    float32 height = vcs->ConvertVirtualToPhysicalX(size.dy);

    rect = CefRect(0, 0, static_cast<int>(width), static_cast<int>(height));
    return true;
}

bool CEFWebPageRender::GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info)
{
    const DeviceInfo::ScreenInfo& screenInfo = DeviceInfo::GetScreenInfo();

    screen_info.device_scale_factor = screenInfo.scale;
    screen_info.depth = 32;
    screen_info.depth_per_component = 8;
    screen_info.is_monochrome = 0;
    screen_info.rect.x = 0;
    screen_info.rect.y = 0;
    screen_info.rect.width = screenInfo.width;
    screen_info.rect.height = screenInfo.height;
    screen_info.available_rect = screen_info.rect;

    return true;
}

void CEFWebPageRender::OnPaint(CefRefPtr<CefBrowser> browser,
                               PaintElementType type,
                               const RectList& dirtyRects,
                               const void* buffer, int width, int height)
{
    if (type != CefRenderHandler::PaintElementType::PET_VIEW)
    {
        return;
    }

    size_t pixelCount = static_cast<size_t>(width * height);
    if (imageWidth != width || imageHeight != height)
    {
        imageWidth = width;
        imageHeight = height;
        imageData.reset(new uint8[pixelCount * 4]);
        contentBackground->SetSprite(nullptr);
    }

    // Update texture
    ::memcpy(imageData.get(), buffer, pixelCount * 4);
    CEFColor* picture = reinterpret_cast<CEFColor*>(imageData.get());
    CEFColor backgroundColor(targetControl.GetBackground()->GetColor());

    // BGRA -> RGBA
    for (size_t i = 0; i < pixelCount; ++i)
    {
        std::swap(picture[i].blue, picture[i].red);
        if (!transparency && picture[i].IsTransparent())
        {
            picture[i] = backgroundColor;
        }
    }

    AppyTexture();
}

void CEFWebPageRender::AppyTexture()
{
    // Create texture or update texture
    if (contentBackground->GetSprite() == nullptr)
    {
        RefPtr<Texture> texture(Texture::CreateFromData(FORMAT_RGBA8888, imageData.get(),
                                                        imageWidth, imageHeight, true));
        texture->SetMinMagFilter(rhi::TEXFILTER_NEAREST, rhi::TEXFILTER_NEAREST, rhi::TEXMIPFILTER_NONE);

        RefPtr<Sprite> sprite(Sprite::CreateFromTexture(texture.Get(), 0, 0,
                                                        static_cast<float32>(texture->GetWidth()),
                                                        static_cast<float32>(texture->GetHeight())));

        contentBackground->SetSprite(sprite.Get());
    }
    else
    {
        Texture* texture = contentBackground->GetSprite()->GetTexture();
        size_t pixelCount = static_cast<size_t>(imageWidth * imageHeight);
        texture->TexImage(0, imageWidth, imageHeight, imageData.get(), pixelCount * 4, 0);
    }
}

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW

#pragma once

#if defined(ENABLE_CEF_WEBVIEW)

#include "UI/Private/CEFWebPageRender.h"
#include "Platform/DeviceInfo.h"
#include "Platform/SystemTimer.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

namespace DAVA
{
CEFWebPageRender::CEFWebPageRender(UIControl& target)
    : targetControl(target)
{
}

void CEFWebPageRender::ClearRenderSurface()
{
    UIControlBackground* background = targetControl.GetBackground();
    background->SetSprite(nullptr);
    background->ReleaseDrawData();
}

bool CEFWebPageRender::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    Vector2 size = targetControl.GetSize();
    float32 width = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(size.dx);
    float32 height = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(size.dy);
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
    }

    // BGRA -> RGBA
    ::memcpy(imageData.get(), buffer, pixelCount * 4);
    for (size_t i = 0; i < pixelCount * 4; i += 4)
    {
        std::swap(imageData[i], imageData[i + 2]);
    }

    // Create texture or update texture
    UIControlBackground* background = targetControl.GetBackground();

    if (background->GetSprite() == nullptr)
    {
        RefPtr<Texture> texture(Texture::CreateFromData(FORMAT_RGBA8888, imageData.get(), width, height, true));
        texture->SetMinMagFilter(rhi::TEXFILTER_NEAREST, rhi::TEXFILTER_NEAREST, rhi::TEXMIPFILTER_NONE);

        RefPtr<Sprite> sprite(Sprite::CreateFromTexture(texture.Get(), 0, 0,
                                                        static_cast<float32>(texture->GetWidth()),
                                                        static_cast<float32>(texture->GetHeight())));

        if (background->GetDrawType() != UIControlBackground::DRAW_STRETCH_BOTH)
        {
            background->SetDrawType(UIControlBackground::DRAW_STRETCH_BOTH);
            background->SetColor(Color::White);
        }
        background->SetSprite(sprite.Get());
    }
    else
    {
        Texture* texture = background->GetSprite()->GetTexture();
        texture->ReloadFromData(FORMAT_RGBA8888, imageData.get(), width, height);
    }
}

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW

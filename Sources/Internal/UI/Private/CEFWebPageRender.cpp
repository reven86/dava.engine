#pragma once

#if defined(ENABLE_CEF_WEBVIEW)

#include "UI/Private/CEFWebPageRender.h"
#include "Platform/DeviceInfo.h"

namespace DAVA
{
CEFWebPageRender::CEFWebPageRender(UIControl& target)
    : targetControl(target)
{
}

bool CEFWebPageRender::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    Vector2 size = targetControl.GetSize();
    rect = CefRect(0, 0, static_cast<int>(size.dx), static_cast<int>(size.dy));
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
    if (type == CefRenderHandler::PaintElementType::PET_POPUP)
    {
        return;
    }

    // BGRA -> RGBA
    size_t pixelCount = static_cast<size_t>(width * height);
    std::unique_ptr<uint8[]> data(new uint8[pixelCount * 4]);
    ::memcpy(data.get(), buffer, pixelCount * 4);
    for (size_t i = 0; i < pixelCount * 4; i += 4)
    {
        std::swap(data[i], data[i + 2]);
    }

    // Create texture
    RefPtr<Texture> texture(Texture::CreateFromData(FORMAT_RGBA8888, data.get(), width, height, true));
    texture->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NONE);

    RefPtr<Sprite> sprite(Sprite::CreateFromTexture(texture.Get(), 0, 0,
                                                    static_cast<float32>(texture->GetWidth()),
                                                    static_cast<float32>(texture->GetHeight())));

    targetControl.GetBackground()->SetSprite(sprite.Get());
}

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW

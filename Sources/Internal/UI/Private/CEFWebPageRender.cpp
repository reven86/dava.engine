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

    uint8 red = 0;
    uint8 green = 0;
    uint8 blue = 0;
    uint8 alpha = 0;
};

CEFWebPageRender::CEFWebPageRender()
    : contentBackground(new UIControlBackground)
{
    auto focusChanged = [this](bool isFocused) -> void
    {
        if (!isFocused)
        {
            ResetCursor();
        }
    };
    focusConnection = Core::Instance()->focusChanged.Connect(focusChanged);
    contentBackground->SetDrawType(UIControlBackground::DRAW_STRETCH_BOTH);
    contentBackground->SetColor(Color::White);
}

CEFWebPageRender::~CEFWebPageRender()
{
    Core::Instance()->focusChanged.Disconnect(focusConnection);
    ShutDown();
}

void CEFWebPageRender::ClearRenderSurface()
{
    if (!imageData.empty())
    {
        ::memset(imageData.data(), 0, imageWidth * imageHeight * 4);
        AppyTexture();
    }
}

UIControlBackground* CEFWebPageRender::GetContentBackground()
{
    return contentBackground.Get();
}

void CEFWebPageRender::SetVisible(bool visibility)
{
    if (isVisible == visibility)
    {
        return;
    }

    isVisible = visibility;
    if (!isVisible)
    {
        ResetCursor();
    }
}

void CEFWebPageRender::SetBackgroundTransparency(bool value)
{
    transparency = value;
}

void CEFWebPageRender::SetViewSize(Vector2 size)
{
    logicalViewSize = size;
}

void CEFWebPageRender::ShutDown()
{
    if (!isActive)
    {
        return;
    }

    isActive = false;
    ResetCursor();

    contentBackground.Set(nullptr);
    imageData.clear();
}

void CEFWebPageRender::ResetCursor()
{
    if (currentCursorType != CursorType::CT_POINTER)
    {
        currentCursorType = CursorType::CT_POINTER;
        SetCursor(GetDefaultCursor());
    }
}

bool CEFWebPageRender::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    VirtualCoordinatesSystem* vcs = VirtualCoordinatesSystem::Instance();
    float32 width = vcs->ConvertVirtualToPhysicalX(logicalViewSize.dx);
    float32 height = vcs->ConvertVirtualToPhysicalX(logicalViewSize.dy);

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
    if (type != CefRenderHandler::PaintElementType::PET_VIEW || !isActive)
    {
        return;
    }

    uint32 pixelCount = static_cast<uint32>(width * height);
    if (imageWidth != width || imageHeight != height)
    {
        imageWidth = width;
        imageHeight = height;
        imageData.resize(pixelCount * 4);
        contentBackground->SetSprite(nullptr);
    }

    // Update texture
    ::memcpy(imageData.data(), buffer, imageData.size());

    // BGRA -> RGBA, resolve transparency and apply
    PostProcessImage();
    AppyTexture();
}

void CEFWebPageRender::PostProcessImage()
{
    uint32 pixelCount = static_cast<uint32>(imageWidth * imageHeight);
    CEFColor* picture = reinterpret_cast<CEFColor*>(imageData.data());

    for (size_t i = 0; i < pixelCount; ++i)
    {
        std::swap(picture[i].blue, picture[i].red);
    }
}

void CEFWebPageRender::AppyTexture()
{
    // Create texture or update texture
    if (contentBackground->GetSprite() == nullptr)
    {
        RefPtr<Texture> texture(Texture::CreateFromData(FORMAT_RGBA8888, imageData.data(),
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
        uint32 dataSize = static_cast<uint32>(imageData.size());
        texture->TexImage(0, imageWidth, imageHeight, imageData.data(), dataSize, 0);
    }
}

void CEFWebPageRender::OnCursorChange(CefRefPtr<CefBrowser> browser,
                                      CefCursorHandle cursor,
                                      CursorType type,
                                      const CefCursorInfo& custom_cursor_info)
{
    if (currentCursorType != type)
    {
        currentCursorType = type;
        SetCursor(cursor);
    }
}

#if defined(__DAVAENGINE_WIN32__)

CefCursorHandle CEFWebPageRender::GetDefaultCursor()
{
    return LoadCursor(NULL, IDC_ARROW);
}

void CEFWebPageRender::SetCursor(CefCursorHandle cursor)
{
    HWND wnd = static_cast<HWND>(Core::Instance()->GetNativeView());
    SetClassLongPtr(wnd, GCLP_HCURSOR, static_cast<LONG>(reinterpret_cast<LONG_PTR>(cursor)));
    ::SetCursor(cursor);
}

#endif

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW

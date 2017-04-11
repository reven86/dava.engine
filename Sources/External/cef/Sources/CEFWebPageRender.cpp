#include "CEFWebPageRender.h"
#include "Platform/DeviceInfo.h"
#include "Render/Renderer.h"
#include "Render/TextureDescriptor.h"
#include "Time/SystemTimer.h"
#include "UI/UIControlSystem.h"

#include "Engine/Engine.h"
#include "Engine/Win32/PlatformApi.h"

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

#if defined(__DAVAENGINE_COREV2__)
CEFWebPageRender::CEFWebPageRender(Window* w, float32 k)
    : contentBackground(new UIControlBackground)
    , window(w)
    , scale(k)
{
    ConnectToSignals();

    Renderer::GetSignals().needRestoreResources.Connect(this, &CEFWebPageRender::RestoreTexture);

    contentBackground->SetDrawType(UIControlBackground::DRAW_STRETCH_BOTH);
    contentBackground->SetColor(Color::White);
    contentBackground->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
}
#else
CEFWebPageRender::CEFWebPageRender()
    : contentBackground(new UIControlBackground)
{
    ConnectToSignals();

    Renderer::GetSignals().needRestoreResources.Connect(this, &CEFWebPageRender::RestoreTexture);

    contentBackground->SetDrawType(UIControlBackground::DRAW_STRETCH_BOTH);
    contentBackground->SetColor(Color::White);
    contentBackground->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
}
#endif

CEFWebPageRender::~CEFWebPageRender()
{
    DisconnectFromSignals();

    Renderer::GetSignals().needRestoreResources.Disconnect(this);

    ShutDown();
}

void CEFWebPageRender::ConnectToSignals()
{
#if defined(__DAVAENGINE_COREV2__)
    Engine::Instance()->windowDestroyed.Connect(this, [this](Window* w) -> void {
        if (w == window)
        {
            DisconnectFromSignals();
        }
    });

    window->focusChanged.Connect(this, [this](Window*, bool isFocused) -> void
    {
        if (!isFocused)
        {
            ResetCursor();
        }
    });
#else
    Core::Instance()->focusChanged.Connect(this, [this](bool isFocused) -> void
    {
        if (!isFocused)
        {
            ResetCursor();
        }
    });
#endif
}

void CEFWebPageRender::DisconnectFromSignals()
{
#if defined(__DAVAENGINE_COREV2__)
    Engine::Instance()->windowDestroyed.Disconnect(this);
    window->focusChanged.Disconnect(this);
#else
    Core::Instance()->focusChanged.Disconnect(this);
#endif
}

void CEFWebPageRender::ClearRenderSurface()
{
    if (!imageData.empty())
    {
        std::fill_n(imageData.begin(), imageData.size(), 0);
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

bool CEFWebPageRender::IsVisible() const
{
    return isVisible;
}

void CEFWebPageRender::SetBackgroundTransparency(bool value)
{
    transparency = value;
}

void CEFWebPageRender::SetViewRect(const Rect& rect)
{
    logicalViewRect = rect;
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

#if defined(__DAVAENGINE_COREV2__)
void CEFWebPageRender::SetScale(float32 k)
{
    scale = k;
}
#endif

void CEFWebPageRender::ResetCursor()
{
    if (currentCursorType != CursorType::CT_POINTER)
    {
        currentCursorType = CursorType::CT_POINTER;
#if defined(__DAVAENGINE_COREV2__)
        SetCursor(nullptr);
#else
        SetCursor(GetDefaultCursor());
#endif
    }
}

bool CEFWebPageRender::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
#if defined(__DAVAENGINE_COREV2__)
    VirtualCoordinatesSystem* vcs = window->GetUIControlSystem()->vcs;
    Rect phrect = vcs->ConvertVirtualToPhysical(logicalViewRect);
    phrect.dx /= scale;
    phrect.dy /= scale;
    rect = CefRect(0, 0, static_cast<int>(phrect.dx), static_cast<int>(phrect.dy));
#else
    rect = CefRect(0, 0, static_cast<int>(logicalViewRect.dx), static_cast<int>(logicalViewRect.dy));
#endif
    return true;
}

bool CEFWebPageRender::GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info)
{
    VirtualCoordinatesSystem* vcs = UIControlSystem::Instance()->vcs;
    Rect phrect = vcs->ConvertVirtualToPhysical(logicalViewRect);

#if defined(__DAVAENGINE_COREV2__)
    screen_info.device_scale_factor = scale;
#else
    screen_info.device_scale_factor = phrect.dx / logicalViewRect.dx;
#endif
    screen_info.depth = 32;
    screen_info.depth_per_component = 8;
    screen_info.is_monochrome = 0;
    screen_info.rect.x = 0;
    screen_info.rect.y = 0;
    screen_info.rect.width = 0;
    screen_info.rect.height = 0;
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
    std::copy_n(static_cast<const uint8*>(buffer), imageData.size(), imageData.begin());

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
        texture->texDescriptor->pathname = "memoryfile_webview_page_render";
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

void CEFWebPageRender::RestoreTexture()
{
    Sprite* sprite = contentBackground->GetSprite();
    Texture* texture = sprite ? sprite->GetTexture() : nullptr;

    if (texture != nullptr && rhi::NeedRestoreTexture(texture->handle))
    {
        rhi::UpdateTexture(texture->handle, imageData.data(), 0);
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

#if !defined(__DAVAENGINE_COREV2__)
CefCursorHandle CEFWebPageRender::GetDefaultCursor()
{
    return LoadCursor(NULL, IDC_ARROW);
}
#endif

void CEFWebPageRender::SetCursor(CefCursorHandle cursor)
{
#if defined(__DAVAENGINE_COREV2__)
    PlatformApi::Win32::SetWindowCursor(window, cursor);
#else
    HWND wnd = static_cast<HWND>(Core::Instance()->GetNativeView());
    SetClassLongPtr(wnd, GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(cursor));
    ::SetCursor(cursor);
#endif
}

#endif

} // namespace DAVA

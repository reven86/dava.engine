#include "CEFWebPageRender.h"
#include "Platform/DeviceInfo.h"
#include "Platform/SystemTimer.h"
#include "Render/RenderCallbacks.h"
#include "Render/TextureDescriptor.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#include "Engine/EngineModule.h"

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
CEFWebPageRender::CEFWebPageRender(Window* w)
    : contentBackground(new UIControlBackground)
    , window(w)
{
    ConnectToSignals();

    auto restoreFunc = MakeFunction(this, &CEFWebPageRender::RestoreTexture);
    RenderCallbacks::RegisterResourceRestoreCallback(std::move(restoreFunc));

    contentBackground->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    contentBackground->SetColor(Color::White);
    contentBackground->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
}
#else
CEFWebPageRender::CEFWebPageRender()
    : contentBackground(new UIControlBackground)
{
    ConnectToSignals();

    auto restoreFunc = MakeFunction(this, &CEFWebPageRender::RestoreTexture);
    RenderCallbacks::RegisterResourceRestoreCallback(std::move(restoreFunc));

    contentBackground->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    contentBackground->SetColor(Color::White);
    contentBackground->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
}
#endif

CEFWebPageRender::~CEFWebPageRender()
{
    DisconnectFromSignals();

    auto restoreFunc = MakeFunction(this, &CEFWebPageRender::RestoreTexture);
    RenderCallbacks::UnRegisterResourceRestoreCallback(std::move(restoreFunc));

    ShutDown();
}

void CEFWebPageRender::ConnectToSignals()
{
#if defined(__DAVAENGINE_COREV2__)
    auto focusChanged = [this](Window*, bool isFocused) -> void
    {
        if (!isFocused)
        {
            ResetCursor();
        }
    };
    auto windowDestroyed = [this](Window* w) -> void {
        if (w == window)
        {
            DisconnectFromSignals();
        }
    };
    windowDestroyedConnection = Engine::Instance()->windowDestroyed.Connect(windowDestroyed);
    focusConnection = window->focusChanged.Connect(focusChanged);
#else
    auto focusChanged = [this](bool isFocused) -> void
    {
        if (!isFocused)
        {
            ResetCursor();
        }
    };
    focusConnection = Core::Instance()->focusChanged.Connect(focusChanged);
#endif
}

void CEFWebPageRender::DisconnectFromSignals()
{
#if defined(__DAVAENGINE_COREV2__)
    if (windowDestroyedConnection != 0)
        Engine::Instance()->windowDestroyed.Disconnect(windowDestroyedConnection);
    if (focusConnection != 0)
        window->focusChanged.Disconnect(focusConnection);
    windowDestroyedConnection = 0;
    focusConnection = 0;
#else
    Core::Instance()->focusChanged.Disconnect(focusConnection);
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

CefCursorHandle CEFWebPageRender::GetDefaultCursor()
{
    return LoadCursor(NULL, IDC_ARROW);
}

void CEFWebPageRender::SetCursor(CefCursorHandle cursor)
{
#if defined(__DAVAENGINE_COREV2__)
#else
    HWND wnd = static_cast<HWND>(Core::Instance()->GetNativeView());
    SetClassLongPtr(wnd, GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(cursor));
    ::SetCursor(cursor);
#endif
}

#endif

} // namespace DAVA

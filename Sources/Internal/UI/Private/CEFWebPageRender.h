#pragma once

#if defined(ENABLE_CEF_WEBVIEW)

#include <cef/include/cef_render_handler.h>

#include "UI/UIControl.h"
#include "Render/2D/Sprite.h"

namespace DAVA
{
class CEFWebPageRender : public CefRenderHandler
{
public:
    CEFWebPageRender(UIControl& target);

    void ClearRenderSurface();
    UIControlBackground* GetContentBackground();

    void SetBackgroundTransparency(bool value);

private:
    // CefRenderHandler interface implementation
    bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
    bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) override;

    void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects,
                 const void* buffer, int width, int height) override;

    void AppyTexture();

    IMPLEMENT_REFCOUNTING(CEFWebPageRender);

    UIControl& targetControl;
    int imageWidth = 0;
    int imageHeight = 0;
    std::unique_ptr<uint8[]> imageData;
    RefPtr<UIControlBackground> contentBackground;
    bool transparency = false;
};

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW

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
    CEFWebPageRender();
    ~CEFWebPageRender();

    void ClearRenderSurface();
    UIControlBackground* GetContentBackground();

    void SetVisible(bool visibility);
    void SetBackgroundTransparency(bool value);
    void SetViewSize(Vector2 size);
    void ShutDown();

private:
    // CefRenderHandler interface implementation
    bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
    bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) override;

    void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects,
                 const void* buffer, int width, int height) override;

    void OnCursorChange(CefRefPtr<CefBrowser> browser,
                        CefCursorHandle cursor,
                        CursorType type,
                        const CefCursorInfo& custom_cursor_info);

    void PostProcessImage();
    void AppyTexture();

    CefCursorHandle GetDefaultCursor();
    void SetCursor(CefCursorHandle cursor);
    void ResetCursor();
    void RestoreCursor();

    IMPLEMENT_REFCOUNTING(CEFWebPageRender);

    int imageWidth = 0;
    int imageHeight = 0;
    std::unique_ptr<uint8[]> imageData;
    Vector2 logicalViewSize;
    RefPtr<UIControlBackground> contentBackground;
    bool transparency = true;
    bool isActive = true;
    bool isVisible = true;
    bool needToRestore = false;
    CursorType currentCursorType = CursorType::CT_POINTER;
    CefCursorHandle currentCursor;
    CefCursorHandle cursorToRestore;
};

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW

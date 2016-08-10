#pragma once

#include <cef/include/cef_render_handler.h>

#include "UI/UIControl.h"
#include "Render/2D/Sprite.h"

#include "Functional/SignalBase.h"

namespace DAVA
{
class Window;
class CEFWebPageRender : public CefRenderHandler
{
    IMPLEMENT_REFCOUNTING(CEFWebPageRender);

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
    void RestoreTexture();

    CefCursorHandle GetDefaultCursor();
    void SetCursor(CefCursorHandle cursor);
    void ResetCursor();

    int imageWidth = 0;
    int imageHeight = 0;
    Vector<uint8> imageData;
    Vector2 logicalViewSize;
    RefPtr<UIControlBackground> contentBackground;
    bool transparency = true;
    bool isActive = true;
    bool isVisible = true;
    CursorType currentCursorType = CursorType::CT_POINTER;
    SigConnectionID focusConnection = SigConnectionID();
#if defined(__DAVAENGINE_COREV2__)
    Window* window = nullptr;
    SigConnectionID windowDestroyedConnection = SigConnectionID();
#endif
    unsigned webViewID = 0;
};

} // namespace DAVA

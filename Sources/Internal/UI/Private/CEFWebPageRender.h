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

private:
    // CefRenderHandler interface implementation
    bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

    bool GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY,
                        int& screenX, int& screenY) override
    {
        return false;
    }

    bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) override;

    void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override
    {
    }
    void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override
    {
    }

    void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects,
                 const void* buffer, int width, int height) override;

    void OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor,
                        CursorType type, const CefCursorInfo& custom_cursor_info)
    {
        int d = 523;
    }

    bool StartDragging(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> drag_data,
                       DragOperationsMask allowed_ops, int x, int y) override
    {
        return false;
    }

    void UpdateDragCursor(CefRefPtr<CefBrowser> browser, DragOperation operation) override
    {
        int d = 523;
    }

    void OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser, double x, double y) override
    {
        int d = 523;
    }

private:
    IMPLEMENT_REFCOUNTING(CEFWebPageRender);

    UIControl& targetControl;
    Vector2 viewSize{ 0, 0 };
    RefPtr<Sprite> sprite;
};

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW

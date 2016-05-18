/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


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

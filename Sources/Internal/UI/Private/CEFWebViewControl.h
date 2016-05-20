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

#include <cef/include/cef_client.h>

#include "UI/IWebViewControl.h"
#include "UI/Private/CEFController.h"

namespace DAVA
{
class WebViewControl : public IWebViewControl, public CefClient, public CefLoadHandler
{
public:
    WebViewControl(UIWebView& uiWebView);
    ~WebViewControl() override;

    // Initialize the control.
    void Initialize(const Rect& rect) override;

    // Open the URL requested.
    void OpenURL(const String& url) override;

    // Load html page from string
    void LoadHtmlString(const WideString& htmlString) override;
    void OpenFromBuffer(const String& htmlString, const FilePath& basePath) override;

    // Execute javascript string in webview
    void ExecuteJScript(const String& scriptString) override;

    // Delete all cookies associated with target URL
    void DeleteCookies(const String& url) override
    {
        //CefCookieManager::GetGlobalManager()
    }
    // Get cookie for specific domain and name
    String GetCookie(const String& url, const String& name) const override
    {
        return "";
    }
    // Get the list of cookies for specific domain
    Map<String, String> GetCookies(const String& url) const override
    {
        return Map<String, String>();
    }

    // Size/pos/visibility changes.
    void SetRect(const Rect& rect) override;
    void SetVisible(bool isVisible, bool hierarchic) override;
    void SetBackgroundTransparency(bool enabled) override
    {
    }

    void SetDelegate(IUIWebViewDelegate* webViewDelegate, UIWebView* webView) override;
    void SetRenderToTexture(bool value) override;
    bool IsRenderToTexture() const override;
    void Input(UIEvent* currentInput) override;
    void Update() override;

private:
    // CefClient interface realization
    CefRefPtr<CefRenderHandler> GetRenderHandler() override;
    CefRefPtr<CefLoadHandler> GetLoadHandler() override;

    // CefLoadHandler interface realization
    void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame, int httpStatusCode) override;

    void LoadHtml(const CefString& html, const CefString& url);
    void OnMouseMove(UIEvent* input);
    void OnMouseClick(UIEvent* input);
    void OnKey(UIEvent* input);

    IMPLEMENT_REFCOUNTING(WebViewControl);

    UIWebView& webView;
    IUIWebViewDelegate* delegate = nullptr;
    CEFController cefController;
    CefRefPtr<class CefBrowser> cefBrowser;
    CefRefPtr<class CEFWebPageRender> webPageRender;
};

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW

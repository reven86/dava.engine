#ifndef __DAVAENGINE_WEBVIEWCONTROL_WINUAP_H__
#define __DAVAENGINE_WEBVIEWCONTROL_WINUAP_H__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__) && !defined(DISABLE_NATIVE_WEBVIEW)

#include "UI/IWebViewControl.h"

namespace DAVA
{
class Sprite;
class CorePlatformWinUAP;
class PrivateWebViewWinUAP;

// Web View Control for WinUAP
class WebViewControl : public IWebViewControl
{
public:
    WebViewControl(UIWebView& uiWebView);
    virtual ~WebViewControl();

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
    void DeleteCookies(const String& url) override;
    // Get cookie for specific domain and name
    String GetCookie(const String& url, const String& name) const override;
    // Get the list of cookies for specific domain
    Map<String, String> GetCookies(const String& url) const override;

    // Size/pos/visibility changes.
    void SetRect(const Rect& rect) override;
    void SetVisible(bool isVisible, bool hierarchic) override;
    void SetBackgroundTransparency(bool enabled) override;

    void SetDelegate(IUIWebViewDelegate* webViewDelegate, UIWebView* webView) override;

    void SetRenderToTexture(bool value) override;
    bool IsRenderToTexture() const override;

    void Update() override;

private:
    std::shared_ptr<PrivateWebViewWinUAP> privateImpl;
};

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__ && !DISABLE_NATIVE_WEBVIEW
#endif // __DAVAENGINE_WEBVIEWCONTROL_WINUAP_H__

#pragma once

#if defined(ENABLE_CEF_WEBVIEW)

#include <cef/include/cef_client.h>

#include "UI/IWebViewControl.h"
#include "UI/Private/CEFController.h"
#include "UI/Private/CEFWebPageRender.h"

namespace DAVA
{
class CEFWebViewControl : public CefClient, public CefLoadHandler
{
public:
    CEFWebViewControl(UIWebView& uiWebView);
    ~CEFWebViewControl() = default;

    // Initialize the control.
    void Initialize(const Rect& rect);
    void Deinitialize();

    // Open the URL requested.
    void OpenURL(const String& url);

    // Load html page from string
    void LoadHtmlString(const WideString& htmlString);
    void OpenFromBuffer(const String& htmlString, const FilePath& basePath);

    // Execute javascript string in webview
    void ExecuteJScript(const String& scriptString);

    // Delete all cookies associated with target URL
    void DeleteCookies(const String& url)
    {
        //CefCookieManager::GetGlobalManager()
    }
    // Get cookie for specific domain and name
    String GetCookie(const String& url, const String& name) const
    {
        return "";
    }
    // Get the list of cookies for specific domain
    Map<String, String> GetCookies(const String& url) const
    {
        return Map<String, String>();
    }

    // Size/pos/visibility changes.
    void SetRect(const Rect& rect);
    void SetVisible(bool isVisible, bool hierarchic);
    void SetBackgroundTransparency(bool enabled)
    {
    }

    void SetDelegate(IUIWebViewDelegate* webViewDelegate, UIWebView* webView);
    void SetRenderToTexture(bool value);
    bool IsRenderToTexture() const;
    void Input(UIEvent* currentInput);
    void Update();

private:
    // CefClient interface realization
    CefRefPtr<CefRenderHandler> GetRenderHandler() override;
    CefRefPtr<CefLoadHandler> GetLoadHandler() override;
    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                  CefProcessId source_process,
                                  CefRefPtr<CefProcessMessage> message) override;

    // CefLoadHandler interface realization
    void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame, int httpStatusCode) override;

    void LoadHtml(const CefString& html, const CefString& url);
    void OnURLLoadingRequst(const struct URLLoadingRequest& request);
    void AllowURLLoading(const String& url, int64 frameID);
    void OnMouseMove(UIEvent* input);
    void OnMouseClick(UIEvent* input);
    void OnKey(UIEvent* input);

    IMPLEMENT_REFCOUNTING(CEFWebViewControl);

    UIWebView& webView;
    IUIWebViewDelegate* delegate = nullptr;
    CEFController cefController;
    CefRefPtr<CefBrowser> cefBrowser;
    CefRefPtr<CEFWebPageRender> webPageRender;
    String requestedUrl;
};

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW

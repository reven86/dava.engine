#pragma once

#include <cef/include/cef_client.h>

#include "UI/IWebViewControl.h"
#include "CEFController.h"
#include "CEFWebPageRender.h"

namespace DAVA
{
class Window;
class CEFWebViewControl : public CefClient,
                          public CefLoadHandler,
                          public CefRequestHandler,
                          public CefLifeSpanHandler
{
    IMPLEMENT_REFCOUNTING(CEFWebViewControl);

public:
#if defined(__DAVAENGINE_COREV2__)
    CEFWebViewControl(Window* w, UIWebView& uiWebView);
#else
    CEFWebViewControl(UIWebView& uiWebView);
#endif
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
    void DeleteCookies(const String& url);
    // Get cookie for specific domain and name
    String GetCookie(const String& url, const String& name) const;
    // Get the list of cookies for specific domain
    Map<String, String> GetCookies(const String& url) const;

    // Size/pos/visibility changes.
    void SetRect(const Rect& rect);
    void SetVisible(bool isVisible, bool hierarchic);
    void SetBackgroundTransparency(bool enabled);

    void SetDelegate(IUIWebViewDelegate* webViewDelegate, UIWebView* webView);
    void SetRenderToTexture(bool value);
    bool IsRenderToTexture() const;
    void Draw(const UIGeometricData& geometricData);
    void Input(UIEvent* currentInput);
    void Update();

private:
    // CefClient interface realization
    CefRefPtr<CefRenderHandler> GetRenderHandler() override;
    CefRefPtr<CefLoadHandler> GetLoadHandler() override;
    CefRefPtr<CefRequestHandler> GetRequestHandler() override;
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;

    // CefLoadHandler interface realization
    void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame, int httpStatusCode) override;

    // CefRequestHandler interface realization
    bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefRequest> request,
                        bool isRedirect) override;

    // CefLifeSpanHandler interface realization
    using WindowOpenDisposition = CefLifeSpanHandler::WindowOpenDisposition;
    bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
                       CefRefPtr<CefFrame> frame,
                       const CefString& targetUrl,
                       const CefString& targetFrameName,
                       WindowOpenDisposition targetDisposition,
                       bool userGesture,
                       const CefPopupFeatures& popupFeatures,
                       CefWindowInfo& windowInfo,
                       CefRefPtr<CefClient>& client,
                       CefBrowserSettings& settings,
                       bool* noJavascriptAccess) override;

    void LoadURL(const String& url, bool clearSurface);
    void LoadHtml(const CefString& html, const CefString& url);
    void StopLoading();
    bool IsEmail(const String& url);

    void OnWindowSizeChanged(Window*, Size2f, Size2f);
    void OnMouseMove(UIEvent* input);
    void OnMouseClick(UIEvent* input);
    void OnMouseWheel(UIEvent* input);
    void OnKey(UIEvent* input);

#if defined(__DAVAENGINE_COREV2__)
    Window* window = nullptr;
    const float defaultDpi = 96.f;
    // scale is ratio between current display DPI and default DPI (96 on Windows)
    float32 scale = 1.f;
#endif
    UIWebView& webView;
    Vector2 webViewPos;
    IUIWebViewDelegate* delegate = nullptr;
    CefRefPtr<CefBrowser> cefBrowser;
    CefRefPtr<CEFWebPageRender> webPageRender;
    String requestedUrl;
    bool pageLoaded = false;
};

} // namespace DAVA

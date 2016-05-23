#if defined(ENABLE_CEF_WEBVIEW)

#include <cef/include/cef_browser.h>
#include <cef/include/cef_scheme.h>
#include "UI/Private/CEFWebPageRender.h"

#include "UI/UIEvent.h"
#include "UI/Private/CEFWebViewControl.h"
#include "UI/UIWebView.h"

namespace DAVA
{
WebViewControl::WebViewControl(UIWebView& uiWebView)
    : webView(uiWebView)
{
    // this object will be deleted manually, but ref counting is needed for work
    // new object in CEF system has 0 references,
    // so it will be destroyed when all owning CefRefPtr will be destroyed
    // to avoid premature destruction, increase reference count manually
    AddRef();
}

WebViewControl::~WebViewControl()
{
    cefBrowser->GetHost()->CloseBrowser(true);
    cefBrowser = nullptr;
    webPageRender = nullptr;
}

void WebViewControl::Initialize(const Rect& rect)
{
    webPageRender = new CEFWebPageRender(webView);

    CefWindowInfo windowInfo;
    windowInfo.windowless_rendering_enabled = 1;
    windowInfo.transparent_painting_enabled = 1;

    CefBrowserSettings settings;
    cefBrowser = CefBrowserHost::CreateBrowserSync(windowInfo, this, "", settings, nullptr);
}

void WebViewControl::OpenURL(const String& url)
{
    cefBrowser->GetMainFrame()->LoadURL(url);
}

void WebViewControl::LoadHtmlString(const WideString& htmlString)
{
    LoadHtml(htmlString, "dava:/~res:/");
}

void WebViewControl::OpenFromBuffer(const String& htmlString, const FilePath& basePath)
{
    String fileUrl = "dava:/" + basePath.GetStringValue();
    LoadHtml(htmlString, fileUrl);
}

void WebViewControl::ExecuteJScript(const String& scriptString)
{
    cefBrowser->GetMainFrame()->ExecuteJavaScript(scriptString, "", 0);
}

void WebViewControl::SetRect(const Rect& rect)
{
    if (rect.GetSize() != webView.GetSize())
    {
        cefBrowser->GetHost()->WasResized();
    }
}

void WebViewControl::SetVisible(bool isVisible, bool /*hierarchic*/)
{
    cefBrowser->GetHost()->WasHidden(!isVisible);
}

void WebViewControl::SetDelegate(IUIWebViewDelegate* webViewDelegate, UIWebView* /*webView*/)
{
    delegate = webViewDelegate;
}

void WebViewControl::SetRenderToTexture(bool value)
{
    // Empty realization, always render to texture
}

bool WebViewControl::IsRenderToTexture() const
{
    return true;
}

void WebViewControl::Input(UIEvent* currentInput)
{
    if (currentInput->phase == UIEvent::Phase::ENDED)
    {
        OnMouseClick(currentInput);
    }
}

void WebViewControl::Update()
{
    cefController.Update();
}

CefRefPtr<CefRenderHandler> WebViewControl::GetRenderHandler()
{
    return webPageRender;
}

CefRefPtr<CefLoadHandler> WebViewControl::GetLoadHandler()
{
    return this;
}

void WebViewControl::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    if (delegate)
    {
        delegate->PageLoaded(&webView);
    }
}

void WebViewControl::LoadHtml(const CefString& html, const CefString& url)
{
    CefRefPtr<CefFrame> frame = cefBrowser->GetMainFrame();

    // loading of "about:blank" is needed for loading string
    frame->LoadURL("about:blank");
    frame->LoadString(html, url);
}

void WebViewControl::OnMouseMove(UIEvent* input)
{
}

void WebViewControl::OnMouseClick(UIEvent* input)
{
    CefRefPtr<CefBrowserHost> host = cefBrowser->GetHost();
}

void WebViewControl::OnKey(UIEvent* input)
{
}

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW

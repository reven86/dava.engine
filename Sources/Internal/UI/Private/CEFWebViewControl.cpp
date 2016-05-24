#if defined(ENABLE_CEF_WEBVIEW)

#include <cef/include/cef_browser.h>
#include <cef/include/cef_scheme.h>
#include "UI/Private/CEFInterprocessMessages.h"
#include "UI/Private/CEFWebPageRender.h"

#include "UI/UIEvent.h"
#include "UI/Private/CEFWebViewControl.h"
#include "UI/UIWebView.h"
#include "Utils/Utils.h"

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
    requestedUrl = url;
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

bool WebViewControl::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                              CefProcessId source_process,
                                              CefRefPtr<CefProcessMessage> message)
{
    // WebViewControl can process only URL loading request messages from render process
    if (source_process == PID_RENDERER &&
        cefBrowser->IsSame(browser) &&
        message->GetName() == urlLoadingRequestMessageName)
    {
        URLLoadingRequest request;
        if (!ParseUrlLoadingRequest(message, request))
        {
            return false;
        }

        OnURLLoadingRequst(request);
        return true;
    }

    return false;
}

void WebViewControl::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    if (delegate && cefBrowser->IsSame(browser))
    {
        delegate->PageLoaded(&webView);
    }
}

void WebViewControl::LoadHtml(const CefString& html, const CefString& url)
{
    requestedUrl = "";
    CefRefPtr<CefFrame> frame = cefBrowser->GetMainFrame();

    // loading of "about:blank" is needed for loading string
    frame->LoadURL("about:blank");
    frame->LoadString(html, url);
}

void WebViewControl::OnURLLoadingRequst(const URLLoadingRequest& request)
{
    // Always allow loading of URL from OpenURL method or if delegate is not set
    if (request.url == requestedUrl || delegate == nullptr)
    {
        AllowURLLoading(request.url, request.frameID);
        return;
    }

    bool isLinkActivation = request.navigation_type == NAVIGATION_LINK_CLICKED;
    IUIWebViewDelegate::eAction action;
    action = delegate->URLChanged(&webView, request.url, isLinkActivation);

    if (action == IUIWebViewDelegate::PROCESS_IN_WEBVIEW)
    {
        AllowURLLoading(request.url, request.frameID);
    }
    else if (action == IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER)
    {
        DAVA::OpenURL(request.url);
    }
}

void WebViewControl::AllowURLLoading(const String& url, int64 frameID)
{
    URLLoadingPermit permit = { url, frameID };
    CefRefPtr<CefProcessMessage> msg = CreateUrlLoadingPermitMessage(permit);
    cefBrowser->SendProcessMessage(PID_RENDERER, msg);
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

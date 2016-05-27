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
CEFWebViewControl::CEFWebViewControl(UIWebView& uiWebView)
    : webView(uiWebView)
{
}

void CEFWebViewControl::Initialize(const Rect& rect)
{
    webPageRender = new CEFWebPageRender(webView);

    CefWindowInfo windowInfo;
    windowInfo.windowless_rendering_enabled = 1;
    windowInfo.transparent_painting_enabled = 1;

    CefBrowserSettings settings;
    cefBrowser = CefBrowserHost::CreateBrowserSync(windowInfo, this, "", settings, nullptr);
}

void CEFWebViewControl::Deinitialize()
{
    // Close browser and release object
    // If we don't release cefBrowser, dtor of CEFWebViewControl will never be invoked
    cefBrowser->GetHost()->CloseBrowser(true);
    webPageRender = nullptr;
    cefBrowser = nullptr;

    // Wait until CEF release this object
    while (!this->HasOneRef())
    {
        cefController.Update();
    }
}

void CEFWebViewControl::OpenURL(const String& url)
{
    StopLoading();
    requestedUrl = url;
    cefBrowser->GetMainFrame()->LoadURL(url);
}

void CEFWebViewControl::LoadHtmlString(const WideString& htmlString)
{
    StopLoading();
    LoadHtml(htmlString, "dava:/~res:/");
}

void CEFWebViewControl::OpenFromBuffer(const String& htmlString, const FilePath& basePath)
{
    StopLoading();
    String fileUrl = "dava:/" + basePath.GetStringValue();
    LoadHtml(htmlString, fileUrl);
}

void CEFWebViewControl::ExecuteJScript(const String& scriptString)
{
    cefBrowser->GetMainFrame()->ExecuteJavaScript(scriptString, "", 0);
}

void CEFWebViewControl::SetRect(const Rect& rect)
{
    if (rect.GetSize() != webView.GetSize())
    {
        cefBrowser->GetHost()->WasResized();
    }
}

void CEFWebViewControl::SetVisible(bool isVisible, bool /*hierarchic*/)
{
    cefBrowser->GetHost()->WasHidden(!isVisible);
}

void CEFWebViewControl::SetDelegate(IUIWebViewDelegate* webViewDelegate, UIWebView* /*webView*/)
{
    delegate = webViewDelegate;
}

void CEFWebViewControl::SetRenderToTexture(bool value)
{
    // Empty realization, always render to texture
}

bool CEFWebViewControl::IsRenderToTexture() const
{
    return true;
}

void CEFWebViewControl::Input(UIEvent* currentInput)
{
    if (currentInput->phase == UIEvent::Phase::ENDED)
    {
        OnMouseClick(currentInput);
    }
}

void CEFWebViewControl::Update()
{
    cefController.Update();
}

CefRefPtr<CefRenderHandler> CEFWebViewControl::GetRenderHandler()
{
    return webPageRender;
}

CefRefPtr<CefLoadHandler> CEFWebViewControl::GetLoadHandler()
{
    return this;
}

bool CEFWebViewControl::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
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

void CEFWebViewControl::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    if (delegate && cefBrowser->IsSame(browser))
    {
        delegate->PageLoaded(&webView);
    }
}

void CEFWebViewControl::LoadHtml(const CefString& html, const CefString& url)
{
    requestedUrl = "";
    CefRefPtr<CefFrame> frame = cefBrowser->GetMainFrame();

    // loading of "about:blank" is needed for loading string
    frame->LoadURL("about:blank");
    frame->LoadString(html, url);
}

void CEFWebViewControl::StopLoading()
{
    if (cefBrowser->IsLoading())
    {
        cefBrowser->StopLoad();
    }
}

void CEFWebViewControl::OnURLLoadingRequst(const URLLoadingRequest& request)
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

void CEFWebViewControl::AllowURLLoading(const String& url, int64 frameID)
{
    URLLoadingPermit permit = { url, frameID };
    CefRefPtr<CefProcessMessage> msg = CreateUrlLoadingPermitMessage(permit);
    cefBrowser->SendProcessMessage(PID_RENDERER, msg);
}

void CEFWebViewControl::OnMouseMove(UIEvent* input)
{
}

void CEFWebViewControl::OnMouseClick(UIEvent* input)
{
    CefRefPtr<CefBrowserHost> host = cefBrowser->GetHost();
}

void CEFWebViewControl::OnKey(UIEvent* input)
{
}

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW

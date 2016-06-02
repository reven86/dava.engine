#if defined(ENABLE_CEF_WEBVIEW)

#include "UI/Private/CEFWebViewControl.h"
#include "UI/Private/CEFWebViewControlProxy.h"

namespace DAVA
{
WebViewControl::WebViewControl(UIWebView& uiWebView)
    : impl(new CEFWebViewControl(uiWebView))
{
}

WebViewControl::~WebViewControl()
{
    impl->Deinitialize();
}

void WebViewControl::Initialize(const Rect& rect)
{
    impl->Initialize(rect);
}

void WebViewControl::OpenURL(const String& url)
{
    impl->OpenURL(url);
}

void WebViewControl::LoadHtmlString(const WideString& htmlString)
{
    impl->LoadHtmlString(htmlString);
}

void WebViewControl::OpenFromBuffer(const String& htmlString, const FilePath& basePath)
{
    impl->OpenFromBuffer(htmlString, basePath);
}

void WebViewControl::ExecuteJScript(const String& scriptString)
{
    impl->ExecuteJScript(scriptString);
}

void WebViewControl::DeleteCookies(const String& url)
{
    impl->DeleteCookies(url);
}

String WebViewControl::GetCookie(const String& url, const String& name) const
{
    return impl->GetCookie(url, name);
}

Map<String, String> WebViewControl::GetCookies(const String& url) const
{
    return impl->GetCookies(url);
}

void WebViewControl::SetRect(const Rect& rect)
{
    impl->SetRect(rect);
}

void WebViewControl::SetVisible(bool isVisible, bool hierarchic)
{
    impl->SetVisible(isVisible, hierarchic);
}

void WebViewControl::SetBackgroundTransparency(bool enabled)
{
    impl->SetBackgroundTransparency(enabled);
}

UIControlBackground* WebViewControl::GetContentBackground()
{
    return impl->GetContentBackground();
}

void WebViewControl::SetDelegate(IUIWebViewDelegate* webViewDelegate, UIWebView* webView)
{
    impl->SetDelegate(webViewDelegate, webView);
}

void WebViewControl::SetRenderToTexture(bool value)
{
    impl->SetRenderToTexture(value);
}

bool WebViewControl::IsRenderToTexture() const
{
    return impl->IsRenderToTexture();
}

void WebViewControl::Input(UIEvent* currentInput)
{
    impl->Input(currentInput);
}

void WebViewControl::Update()
{
    impl->Update();
}

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW

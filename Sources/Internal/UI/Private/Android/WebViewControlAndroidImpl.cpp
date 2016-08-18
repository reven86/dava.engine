#if !defined(DISABLE_NATIVE_WEBVIEW)

#include "UI/Private/Android/WebViewControlAndroidImpl.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

#include "Math/Rect.h"

namespace DAVA
{
WebViewControlImpl::WebViewControlImpl(Window& w, UIWebView& uiWebView)
    : window(w)
    , uiWebView(&uiWebView)
{
}

WebViewControlImpl::~WebViewControlImpl()
{
}

void WebViewControlImpl::OwnerAtPremortem()
{
}

void WebViewControlImpl::Initialize(const Rect& rect)
{
}

void WebViewControlImpl::OpenURL(const String& url)
{
}

void WebViewControlImpl::LoadHtmlString(const WideString& htmlString)
{
}

void WebViewControlImpl::OpenFromBuffer(const String& htmlString, const FilePath& basePath)
{
}

void WebViewControlImpl::ExecuteJScript(const String& jsScript)
{
}

void WebViewControlImpl::SetRect(const Rect& rect)
{
}

void WebViewControlImpl::SetVisible(bool visible)
{
}

void WebViewControlImpl::SetBackgroundTransparency(bool enabled)
{
}

void WebViewControlImpl::SetDelegate(IUIWebViewDelegate* webViewDelegate)
{
    uiWebViewDelegate = webViewDelegate;
}

void WebViewControlImpl::SetRenderToTexture(bool enabled)
{
}

bool WebViewControlImpl::IsRenderToTexture() const
{
    return false;
}

void WebViewControlImpl::Update()
{
}

void WebViewControlImpl::DeleteCookies(const String& url)
{
}

String WebViewControlImpl::GetCookie(const String& url, const String& name)
{
    return String();
}

Map<String, String> WebViewControlImpl::GetCookies(const String& url)
{
    return Map<String, String>();
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__
#endif // !DISABLE_NATIVE_WEBVIEW

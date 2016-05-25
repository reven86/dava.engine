#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__) && !defined(DISABLE_NATIVE_WEBVIEW)

#include "Utils/Utils.h"

#include "Platform/TemplateWin32/WebViewControlWinUAP.h"
#include "Platform/TemplateWin32/PrivateWebViewWinUAP.h"

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Web;
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Filters;

namespace DAVA
{
WebViewControl::WebViewControl(UIWebView& uiWebView)
    : privateImpl(std::make_shared<PrivateWebViewWinUAP>(&uiWebView))
{
}

WebViewControl::~WebViewControl()
{
    // Tell private implementation that owner is sentenced to death
    privateImpl->OwnerAtPremortem();
}

void WebViewControl::Initialize(const Rect& rect)
{
    privateImpl->Initialize(rect);
}

void WebViewControl::OpenURL(const String& url)
{
    privateImpl->OpenURL(url);
}

void WebViewControl::LoadHtmlString(const WideString& htmlString)
{
    privateImpl->LoadHtmlString(htmlString);
}

void WebViewControl::OpenFromBuffer(const String& htmlString, const FilePath& basePath)
{
    privateImpl->OpenFromBuffer(htmlString, basePath);
}

void WebViewControl::ExecuteJScript(const String& scriptString)
{
    privateImpl->ExecuteJScript(scriptString);
}

void WebViewControl::SetRect(const Rect& rect)
{
    privateImpl->SetRect(rect);
}

void WebViewControl::SetVisible(bool isVisible, bool /*hierarchic*/)
{
    privateImpl->SetVisible(isVisible);
}

void WebViewControl::SetBackgroundTransparency(bool enabled)
{
    privateImpl->SetBackgroundTransparency(enabled);
}

void WebViewControl::SetDelegate(IUIWebViewDelegate* webViewDelegate, UIWebView* webView)
{
    if (webViewDelegate != nullptr && webView != nullptr)
    {
        privateImpl->SetDelegate(webViewDelegate);
    }
}

void WebViewControl::SetRenderToTexture(bool value)
{
    privateImpl->SetRenderToTexture(value);
}

bool WebViewControl::IsRenderToTexture() const
{
    return privateImpl->IsRenderToTexture();
}

void WebViewControl::Update()
{
    privateImpl->Update();
}

void WebViewControl::DeleteCookies(const String& url)
{
    Uri ^ uri = ref new Uri(ref new Platform::String(StringToWString(url).c_str()));
    HttpBaseProtocolFilter httpObj;
    HttpCookieManager ^ cookieManager = httpObj.CookieManager;
    HttpCookieCollection ^ cookies = cookieManager->GetCookies(uri);

    IIterator<HttpCookie ^> ^ it = cookies->First();
    while (it->HasCurrent)
    {
        HttpCookie ^ cookie = it->Current;
        cookieManager->DeleteCookie(cookie);
        it->MoveNext();
    }
}

String WebViewControl::GetCookie(const String& url, const String& name) const
{
    String result;

    Uri ^ uri = ref new Uri(ref new Platform::String(StringToWString(url).c_str()));
    HttpBaseProtocolFilter httpObj;
    HttpCookieCollection ^ cookies = httpObj.CookieManager->GetCookies(uri);

    Platform::String ^ cookieName = ref new Platform::String(StringToWString(name).c_str());
    IIterator<HttpCookie ^> ^ it = cookies->First();
    while (it->HasCurrent)
    {
        HttpCookie ^ cookie = it->Current;
        if (cookie->Name == cookieName)
        {
            result = WStringToString(cookie->Value->Data());
            break;
        }
        it->MoveNext();
    }
    return result;
}

Map<String, String> WebViewControl::GetCookies(const String& url) const
{
    Map<String, String> result;

    Uri ^ uri = ref new Uri(ref new Platform::String(StringToWString(url).c_str()));
    HttpBaseProtocolFilter httpObj;
    HttpCookieCollection ^ cookies = httpObj.CookieManager->GetCookies(uri);

    IIterator<HttpCookie ^> ^ it = cookies->First();
    while (it->HasCurrent)
    {
        HttpCookie ^ cookie = it->Current;
        result.emplace(WStringToString(cookie->Name->Data()), WStringToString(cookie->Value->Data()));
        it->MoveNext();
    }
    return result;
}

} // namespace DAVA

#endif // (__DAVAENGINE_WIN_UAP__) && !(DISABLE_NATIVE_WEBVIEW)

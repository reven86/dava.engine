#pragma once

#if !defined(DISABLE_NATIVE_WEBVIEW)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseObject.h"
#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
class Rect;
class FilePath;
class UIWebView;
class IUIWebViewDelegate;

class WebViewControlImpl : public BaseObject
{
public:
    WebViewControlImpl(Window& w, UIWebView& uiWebView);
    ~WebViewControlImpl() override;

    // WebViewControl should invoke it in its destructor to tell this class instance
    // to fly away on its own (finish pending jobs if any, and delete when all references are lost)
    void OwnerAtPremortem();

    void Initialize(const Rect& rect);

    void OpenURL(const String& url);
    void LoadHtmlString(const WideString& htmlString);
    void OpenFromBuffer(const String& htmlString, const FilePath& basePath);
    void ExecuteJScript(const String& jsScript);

    void SetRect(const Rect& rect);
    void SetVisible(bool visible);
    void SetBackgroundTransparency(bool enabled);

    void SetDelegate(IUIWebViewDelegate* webViewDelegate);

    void SetRenderToTexture(bool enabled);
    bool IsRenderToTexture() const;

    void Update();

    static void DeleteCookies(const String& url);
    static String GetCookie(const String& url, const String& name);
    static Map<String, String> GetCookies(const String& url);

private:
    Window& window;
    UIWebView* uiWebView = nullptr;
    IUIWebViewDelegate* uiWebViewDelegate = nullptr;
};

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__
#endif // !DISABLE_NATIVE_WEBVIEW

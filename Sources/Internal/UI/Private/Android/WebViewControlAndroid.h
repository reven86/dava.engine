#ifndef __DAVAENGINE_WEBVIEWCONTROL_H__
#define __DAVAENGINE_WEBVIEWCONTROL_H__

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_ANDROID__) && !defined(DISABLE_NATIVE_WEBVIEW)

#include "UI/IWebViewControl.h"
#include "Platform/TemplateAndroid/JniHelpers.h"

namespace DAVA
{
class JniWebView;
class UIWebView;

// Web View Control - Android version.
class WebViewControl : public IWebViewControl
{
    friend class JniWebView;

public:
    WebViewControl(UIWebView& uiWebView);
    virtual ~WebViewControl();

    // Initialize the control.
    void Initialize(const Rect& rect) override;

    // Open the URL requested.
    void OpenURL(const String& urlToOpen) override;
    // Load html page from string
    void LoadHtmlString(const WideString& htmlString) override;
    // Delete all cookies associated with target URL
    void DeleteCookies(const String& targetUrl) override;
    // Get cookie for specific domain and name
    String GetCookie(const String& url, const String& name) const override;
    // Get the list of cookies for specific domain
    Map<String, String> GetCookies(const String& url) const override;
    // Perform Java script
    void ExecuteJScript(const String& scriptString) override;

    void OpenFromBuffer(const String& string, const FilePath& basePath) override;

    // Size/pos/visibility changes.
    void SetRect(const Rect& rect) override;
    void SetVisible(bool isVisible, bool hierarchic) override;

    void SetDelegate(IUIWebViewDelegate* delegate, UIWebView*) override;
    void SetBackgroundTransparency(bool enabled) override;

    void SetRenderToTexture(bool value) override;
    bool IsRenderToTexture() const override;

    UIWebView& GetUIWebView() const
    {
        return webView;
    }

    void WillDraw() override;

private:
    int32 webViewId;
    IUIWebViewDelegate* delegate;
    UIWebView& webView;

    static int32 webViewIdCount;
};

class JniWebView
{
public:
    JniWebView();
    void Initialize(WebViewControl* control, int id, const Rect& rect);
    void Deinitialize(int id);

    void OpenURL(int id, const String& urlToOpen);
    void LoadHtmlString(int id, const String& htmlString);
    void DeleteCookies(const String& targetUrl);
    String GetCookie(const String& targetUrl, const String& name);
    Map<String, String> GetCookies(const String& targetUrl);
    void ExecuteJScript(int id, const String& scriptString);
    void OpenFromBuffer(int id, const String& string, const String& basePath);

    void SetRect(int id, const Rect& rect);
    void SetVisible(int id, bool isVisible);

    void SetBackgroundTransparency(int id, bool isVisible);

    void SetRenderToTexture(int id, bool renderToTexture);
    bool IsRenderToTexture(int id);

    void WillDraw(int id);

    static IUIWebViewDelegate::eAction URLChanged(int id, const String& newURL, bool isRedirectedByMouseClick);
    static void PageLoaded(int id, int* rawPixels, int width, int height);
    static void OnExecuteJScript(int id, const String& result);

private:
    typedef std::map<int, WebViewControl*> CONTROLS_MAP;
    static CONTROLS_MAP controls;

    JNI::JavaClass jniWebView;

    Function<void(jint, jfloat, jfloat, jfloat, jfloat)> initialize;
    Function<void(jint)> deinitialize;
    Function<void(jint, jstring)> openURL;
    Function<void(jint, jstring)> loadHtmlString;
    Function<void(jint, jstring)> executeJScript;
    Function<jstring(jstring, jstring)> getCookie;
    Function<jobjectArray(jstring)> getCookies;
    Function<void(jstring)> deleteCookies;
    Function<void(jint, jstring, jstring)> openFromBuffer;
    Function<void(jint, jfloat, jfloat, jfloat, jfloat)> setRect;
    Function<void(jint, jboolean)> setVisible;
    Function<void(jint, jboolean)> setBackgroundTransparency;
    Function<void(jint, jboolean)> setRenderToTexture;
    Function<jboolean(jint)> isRenderToTexture;
    Function<void(jint)> willDraw;
};
};

#endif //#if defined(__DAVAENGINE_ANDROID__) && !defined(DISABLE_NATIVE_WEBVIEW)

#endif /* defined(__DAVAENGINE_WEBVIEWCONTROL_MACOS_H__) */

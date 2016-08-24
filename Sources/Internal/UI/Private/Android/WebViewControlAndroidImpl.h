#pragma once

#if !defined(DISABLE_NATIVE_WEBVIEW)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

#include "Functional/Function.h"

#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Android/JNIBridge.h"

#include "UI/IWebViewControl.h"

namespace DAVA
{
class Rect;
class FilePath;
class UIWebView;
class IUIWebViewDelegate;

class WebViewControlImpl
{
public:
    WebViewControlImpl(Window& w, UIWebView& uiWebView);
    ~WebViewControlImpl();

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

    void DeleteCookies(const String& url);
    String GetCookie(const String& url, const String& name);
    Map<String, String> GetCookies(const String& url);

    jint nativeOnUrlChanged(JNIEnv* env, jstring jurl, jboolean jisRedirectedByMouseClick);
    void nativeOnPageLoaded(JNIEnv* env);
    void nativeOnExecuteJavaScript(JNIEnv* env, jstring jresult);

private:
    IUIWebViewDelegate::eAction OnUrlChanged(const String& url, bool isRedirectedByMouseClick);
    void OnPageLoaded();
    void OnExecuteJavaScript(const String& result);

private:
    Window& window;
    UIWebView* uiWebView = nullptr;
    IUIWebViewDelegate* uiWebViewDelegate = nullptr;
    jobject javaWebView = nullptr;

    std::unique_ptr<JNI::JavaClass> webViewJavaClass;
    Function<void(jobject)> release;
    Function<void(jobject, jstring)> openURL;
    Function<void(jobject, jstring)> loadHtmlString;
    Function<void(jobject, jstring, jstring)> openFromBuffer;
    Function<void(jobject, jstring)> executeJScript;
    Function<void(jobject, jfloat, jfloat, jfloat, jfloat)> setRect;
    Function<void(jobject, jboolean)> setVisible;
    Function<void(jobject, jboolean)> setBackgroundTransparency;
    Function<void(jobject, jboolean)> setRenderToTexture;
    Function<jboolean(jobject)> isRenderToTexture;
    Function<void(jobject)> update;
    Function<void(jstring)> deleteCookies;
    Function<jstring(jstring, jstring)> getCookie;
    Function<jstringArray(jstring)> getCookies;
};

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__
#endif // !DISABLE_NATIVE_WEBVIEW

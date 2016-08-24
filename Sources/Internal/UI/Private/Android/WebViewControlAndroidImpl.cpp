#if !defined(DISABLE_NATIVE_WEBVIEW)

#include "UI/Private/Android/WebViewControlAndroidImpl.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

#include "Math/Rect.h"
#include "Utils/Utils.h"
#include "Logger/Logger.h"

#include "Engine/Engine.h"
#include "Engine/Window.h"
#include "Engine/Android/WindowNativeServiceAndroid.h"

extern "C"
{

JNIEXPORT jint JNICALL Java_com_dava_engine_DavaWebView_nativeOnUrlChanged(JNIEnv* env, jclass jclazz, jlong backendPointer, jstring url, jboolean isRedirectedByMouseClick)
{
    using DAVA::WebViewControlImpl;
    WebViewControlImpl* backend = reinterpret_cast<WebViewControlImpl*>(static_cast<uintptr_t>(backendPointer));
    return backend->nativeOnUrlChanged(env, url, isRedirectedByMouseClick);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaWebView_nativeOnPageLoaded(JNIEnv* env, jclass jclazz, jlong backendPointer)
{
    using DAVA::WebViewControlImpl;
    WebViewControlImpl* backend = reinterpret_cast<WebViewControlImpl*>(static_cast<uintptr_t>(backendPointer));
    backend->nativeOnPageLoaded(env);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaWebView_nativeOnExecuteJavaScript(JNIEnv* env, jclass jclazz, jlong backendPointer, jstring result)
{
    using DAVA::WebViewControlImpl;
    WebViewControlImpl* backend = reinterpret_cast<WebViewControlImpl*>(static_cast<uintptr_t>(backendPointer));
    backend->nativeOnExecuteJavaScript(env, result);
}
}

namespace DAVA
{
WebViewControlImpl::WebViewControlImpl(Window& w, UIWebView& uiWebView)
    : window(w)
    , uiWebView(&uiWebView)
{
}

WebViewControlImpl::~WebViewControlImpl()
{
    uiWebView = nullptr;
    uiWebViewDelegate = nullptr;
    if (javaWebView != nullptr)
    {
        release(javaWebView);
        JNI::GetEnv()->DeleteGlobalRef(javaWebView);
    }
}

void WebViewControlImpl::Initialize(const Rect& rect)
{
    try
    {
        webViewJavaClass.reset(new JNI::JavaClass("com/dava/engine/DavaWebView"));
        release = webViewJavaClass->GetMethod<void>("release");
        openURL = webViewJavaClass->GetMethod<void, jstring>("openURL");
        loadHtmlString = webViewJavaClass->GetMethod<void, jstring>("loadHtmlString");
        openFromBuffer = webViewJavaClass->GetMethod<void, jstring, jstring>("openFromBuffer");
        executeJScript = webViewJavaClass->GetMethod<void, jstring>("executeJScript");
        setRect = webViewJavaClass->GetMethod<void, jfloat, jfloat, jfloat, jfloat>("setRect");
        setVisible = webViewJavaClass->GetMethod<void, jboolean>("setVisible");
        setBackgroundTransparency = webViewJavaClass->GetMethod<void, jboolean>("setBackgroundTransparency");
        setRenderToTexture = webViewJavaClass->GetMethod<void, jboolean>("setRenderToTexture");
        isRenderToTexture = webViewJavaClass->GetMethod<jboolean>("isRenderToTexture");
        update = webViewJavaClass->GetMethod<void>("update");
        deleteCookies = webViewJavaClass->GetStaticMethod<void, jstring>("deleteCookies");
        getCookie = webViewJavaClass->GetStaticMethod<jstring, jstring, jstring>("getCookie");
        getCookies = webViewJavaClass->GetStaticMethod<jstringArray, jstring>("getCookies");
    }
    catch (const JNI::Exception& e)
    {
        Logger::Error("[WebViewControl] failed to init java bridge: %s", e.what());
        DVASSERT_MSG(false, e.what());
        return;
    }

    jobject obj = window.GetNativeService()->CreateNativeControl("com.dava.engine.DavaWebView", this);
    if (obj != nullptr)
    {
        JNIEnv* env = JNI::GetEnv();
        javaWebView = env->NewGlobalRef(obj);
        env->DeleteLocalRef(obj);
        SetRect(rect);
    }
    else
    {
        Logger::Error("[WebViewControl] failed to create java webview");
    }
}

void WebViewControlImpl::OpenURL(const String& url)
{
    if (javaWebView != nullptr)
    {
        JNIEnv* env = JNI::GetEnv();
        jstring jstr = JNI::StringToJavaString(url, env);

        openURL(javaWebView, jstr);
        env->DeleteLocalRef(jstr);
    }
}

void WebViewControlImpl::LoadHtmlString(const WideString& htmlString)
{
    if (javaWebView != nullptr)
    {
        JNIEnv* env = JNI::GetEnv();
        jstring jstr = JNI::WideStringToJavaString(htmlString, env);

        loadHtmlString(javaWebView, jstr);
        env->DeleteLocalRef(jstr);
    }
}

void WebViewControlImpl::OpenFromBuffer(const String& htmlString, const FilePath& basePath)
{
    if (javaWebView != nullptr)
    {
        JNIEnv* env = JNI::GetEnv();
        jstring jstr = JNI::StringToJavaString(htmlString, env);
        jstring jbase = JNI::StringToJavaString(basePath.AsURL(), env);

        openFromBuffer(javaWebView, jstr, jbase);
        env->DeleteLocalRef(jstr);
        env->DeleteLocalRef(jbase);
    }
}

void WebViewControlImpl::ExecuteJScript(const String& jsScript)
{
    if (javaWebView != nullptr)
    {
        JNIEnv* env = JNI::GetEnv();
        jstring jstr = JNI::StringToJavaString(jsScript, env);

        executeJScript(javaWebView, jstr);
        env->DeleteLocalRef(jstr);
    }
}

void WebViewControlImpl::SetRect(const Rect& rect)
{
    if (javaWebView != nullptr)
    {
        Rect rc = JNI::V2I(rect);
        rc.dx = std::max(0.0f, rc.dx);
        rc.dy = std::max(0.0f, rc.dy);

        setRect(javaWebView, rc.x, rc.y, rc.dx, rc.dy);
    }
}

void WebViewControlImpl::SetVisible(bool visible)
{
    if (javaWebView != nullptr)
    {
        setVisible(javaWebView, visible ? JNI_TRUE : JNI_FALSE);
    }
}

void WebViewControlImpl::SetBackgroundTransparency(bool enabled)
{
    if (javaWebView != nullptr)
    {
        setBackgroundTransparency(javaWebView, enabled ? JNI_TRUE : JNI_FALSE);
    }
}

void WebViewControlImpl::SetDelegate(IUIWebViewDelegate* webViewDelegate)
{
    uiWebViewDelegate = webViewDelegate;
}

void WebViewControlImpl::SetRenderToTexture(bool enabled)
{
    if (javaWebView != nullptr)
    {
        setRenderToTexture(javaWebView, enabled ? JNI_TRUE : JNI_FALSE);
    }
}

bool WebViewControlImpl::IsRenderToTexture() const
{
    return javaWebView != nullptr ? isRenderToTexture(javaWebView) == JNI_TRUE : false;
}

void WebViewControlImpl::Update()
{
    if (javaWebView != nullptr)
    {
        update(javaWebView);
    }
}

void WebViewControlImpl::DeleteCookies(const String& url)
{
    if (javaWebView != nullptr)
    {
        JNIEnv* env = JNI::GetEnv();
        jstring jurl = JNI::StringToJavaString(url, env);
        deleteCookies(jurl);
        env->DeleteLocalRef(jurl);
    }
}

String WebViewControlImpl::GetCookie(const String& url, const String& name)
{
    String cookie;
    if (javaWebView != nullptr)
    {
        JNIEnv* env = JNI::GetEnv();
        jstring jurl = JNI::StringToJavaString(url, env);
        jstring jname = JNI::StringToJavaString(name, env);
        jstring jcookie = getCookie(jurl, jname);
        cookie = JNI::JavaStringToString(jcookie);
        env->DeleteLocalRef(jurl);
        env->DeleteLocalRef(jname);
        env->DeleteLocalRef(jcookie);
    }
    return cookie;
}

Map<String, String> WebViewControlImpl::GetCookies(const String& url)
{
    Map<String, String> result;
    if (javaWebView != nullptr)
    {
        JNIEnv* env = JNI::GetEnv();
        jstring jurl = JNI::StringToJavaString(url, env);
        jstringArray jcookies = getCookies(jurl);

        jsize n = env->GetArrayLength(jcookies);
        for (jsize i = 0; i < n; ++i)
        {
            jstring jitem = static_cast<jstring>(env->GetObjectArrayElement(jcookies, i));
            String item = JNI::JavaStringToString(jitem);
            env->DeleteLocalRef(jitem);

            Vector<String> cookie;
            Split(item, "=", cookie);

            result[cookie[0]] = cookie[1];
        }

        env->DeleteLocalRef(jurl);
        env->DeleteLocalRef(jcookies);
    }
    return result;
}

jint WebViewControlImpl::nativeOnUrlChanged(JNIEnv* env, jstring jurl, jboolean jisRedirectedByMouseClick)
{
    String url = JNI::JavaStringToString(jurl, env);

    bool isRedirectedByMouseClick = jisRedirectedByMouseClick == JNI_TRUE;
    IUIWebViewDelegate::eAction action = IUIWebViewDelegate::PROCESS_IN_WEBVIEW;
    window.GetEngine()->RunAndWaitOnMainThread([this, url, isRedirectedByMouseClick, &action]() {
        action = OnUrlChanged(url, isRedirectedByMouseClick);
    });

    return static_cast<jint>(action);
}

void WebViewControlImpl::nativeOnPageLoaded(JNIEnv* env)
{
    window.GetEngine()->RunAsyncOnMainThread([this]() {
        OnPageLoaded();
    });
}

void WebViewControlImpl::nativeOnExecuteJavaScript(JNIEnv* env, jstring jresult)
{
    String result = JNI::JavaStringToString(jresult, env);
    window.GetEngine()->RunAsyncOnMainThread([this, result]() {
        OnExecuteJavaScript(result);
    });
}

IUIWebViewDelegate::eAction WebViewControlImpl::OnUrlChanged(const String& url, bool isRedirectedByMouseClick)
{
    IUIWebViewDelegate::eAction action = IUIWebViewDelegate::PROCESS_IN_WEBVIEW;
    if (uiWebViewDelegate != nullptr)
    {
        action = uiWebViewDelegate->URLChanged(uiWebView, url, isRedirectedByMouseClick);
    }
    return action;
}

void WebViewControlImpl::OnPageLoaded()
{
    if (uiWebViewDelegate != nullptr)
    {
        uiWebViewDelegate->PageLoaded(uiWebView);
    }
}

void WebViewControlImpl::OnExecuteJavaScript(const String& result)
{
    if (uiWebViewDelegate != nullptr)
    {
        uiWebViewDelegate->OnExecuteJScript(uiWebView, result);
    }
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__
#endif // !DISABLE_NATIVE_WEBVIEW

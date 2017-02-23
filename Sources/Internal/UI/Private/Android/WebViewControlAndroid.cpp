#if !defined(DISABLE_NATIVE_WEBVIEW)

#include "UI/Private/Android/WebViewControlAndroid.h"
#include "UI/UIControlSystem.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

#include "Math/Rect.h"
#include "Utils/Utils.h"
#include "Logger/Logger.h"

#include "Engine/Engine.h"
#include "Engine/Window.h"

extern "C"
{

JNIEXPORT void JNICALL Java_com_dava_engine_DavaWebView_nativeReleaseWeakPtr(JNIEnv* env, jclass jclazz, jlong backendPointer)
{
    using DAVA::WebViewControl;

    // Postpone deleting in case some other jobs are posted to main thread
    DAVA::RunOnMainThreadAsync([backendPointer]() {
        std::weak_ptr<WebViewControl>* weak = reinterpret_cast<std::weak_ptr<WebViewControl>*>(static_cast<uintptr_t>(backendPointer));
        delete weak;
    });
}

JNIEXPORT jint JNICALL Java_com_dava_engine_DavaWebView_nativeOnUrlChanged(JNIEnv* env, jclass jclazz, jlong backendPointer, jstring url, jboolean isRedirectedByMouseClick)
{
    using DAVA::WebViewControl;
    std::weak_ptr<WebViewControl>* weak = reinterpret_cast<std::weak_ptr<WebViewControl>*>(static_cast<uintptr_t>(backendPointer));
    if (auto backend = weak->lock())
        return backend->nativeOnUrlChanged(env, url, isRedirectedByMouseClick);
    return static_cast<jint>(DAVA::IUIWebViewDelegate::NO_PROCESS);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaWebView_nativeOnPageLoaded(JNIEnv* env, jclass jclazz, jlong backendPointer)
{
    using DAVA::WebViewControl;
    std::weak_ptr<WebViewControl>* weak = reinterpret_cast<std::weak_ptr<WebViewControl>*>(static_cast<uintptr_t>(backendPointer));
    if (auto backend = weak->lock())
        backend->nativeOnPageLoaded(env);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaWebView_nativeOnExecuteJavaScript(JNIEnv* env, jclass jclazz, jlong backendPointer, jstring result)
{
    using DAVA::WebViewControl;
    std::weak_ptr<WebViewControl>* weak = reinterpret_cast<std::weak_ptr<WebViewControl>*>(static_cast<uintptr_t>(backendPointer));
    if (auto backend = weak->lock())
        backend->nativeOnExecuteJavaScript(env, result);
}

} // extern "C"

namespace DAVA
{
WebViewControl::WebViewControl(Window* w, UIWebView* uiWebView)
    : window(w)
    , uiWebView(uiWebView)
{
}

WebViewControl::~WebViewControl() = default;

void WebViewControl::Initialize(const Rect& rect)
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
        DVASSERT(false, e.what());
        return;
    }

    std::weak_ptr<WebViewControl>* selfWeakPtr = new std::weak_ptr<WebViewControl>(shared_from_this());
    jobject obj = PlatformApi::Android::CreateNativeControl(window, "com.dava.engine.DavaWebView", selfWeakPtr);
    if (obj != nullptr)
    {
        JNIEnv* env = JNI::GetEnv();
        javaWebView = env->NewGlobalRef(obj);
        env->DeleteLocalRef(obj);
        SetRect(rect);
    }
    else
    {
        delete selfWeakPtr;
        Logger::Error("[WebViewControl] failed to create java webview");
    }
}

void WebViewControl::OwnerIsDying()
{
    uiWebView = nullptr;
    uiWebViewDelegate = nullptr;
    if (javaWebView != nullptr)
    {
        release(javaWebView);
        JNI::GetEnv()->DeleteGlobalRef(javaWebView);
        javaWebView = nullptr;
    }
}

void WebViewControl::OpenURL(const String& url)
{
    if (javaWebView != nullptr)
    {
        JNIEnv* env = JNI::GetEnv();
        jstring jstr = JNI::StringToJavaString(url, env);

        openURL(javaWebView, jstr);
        env->DeleteLocalRef(jstr);
    }
}

void WebViewControl::LoadHtmlString(const WideString& htmlString)
{
    if (javaWebView != nullptr)
    {
        JNIEnv* env = JNI::GetEnv();
        jstring jstr = JNI::WideStringToJavaString(htmlString, env);

        loadHtmlString(javaWebView, jstr);
        env->DeleteLocalRef(jstr);
    }
}

void WebViewControl::OpenFromBuffer(const String& htmlString, const FilePath& basePath)
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

void WebViewControl::ExecuteJScript(const String& jsScript)
{
    if (javaWebView != nullptr)
    {
        JNIEnv* env = JNI::GetEnv();
        jstring jstr = JNI::StringToJavaString(jsScript, env);

        executeJScript(javaWebView, jstr);
        env->DeleteLocalRef(jstr);
    }
}

void WebViewControl::SetRect(const Rect& rect)
{
    if (javaWebView != nullptr)
    {
        Rect rc = UIControlSystem::Instance()->vcs->ConvertVirtualToInput(rect);
        rc.dx = std::max(0.0f, rc.dx);
        rc.dy = std::max(0.0f, rc.dy);

        setRect(javaWebView, rc.x, rc.y, rc.dx, rc.dy);
    }
}

void WebViewControl::SetVisible(bool visible, bool /*hierarchic*/)
{
    if (javaWebView != nullptr)
    {
        setVisible(javaWebView, visible ? JNI_TRUE : JNI_FALSE);
    }
}

void WebViewControl::SetBackgroundTransparency(bool enabled)
{
    if (javaWebView != nullptr)
    {
        setBackgroundTransparency(javaWebView, enabled ? JNI_TRUE : JNI_FALSE);
    }
}

void WebViewControl::SetDelegate(IUIWebViewDelegate* webViewDelegate, UIWebView* /*uiWebView*/)
{
    uiWebViewDelegate = webViewDelegate;
}

void WebViewControl::SetRenderToTexture(bool enabled)
{
    if (javaWebView != nullptr)
    {
        setRenderToTexture(javaWebView, enabled ? JNI_TRUE : JNI_FALSE);
    }
}

bool WebViewControl::IsRenderToTexture() const
{
    return javaWebView != nullptr ? isRenderToTexture(javaWebView) == JNI_TRUE : false;
}

void WebViewControl::Update()
{
    if (javaWebView != nullptr)
    {
        update(javaWebView);
    }
}

void WebViewControl::DeleteCookies(const String& url)
{
    if (javaWebView != nullptr)
    {
        JNIEnv* env = JNI::GetEnv();
        jstring jurl = JNI::StringToJavaString(url, env);
        deleteCookies(jurl);
        env->DeleteLocalRef(jurl);
    }
}

String WebViewControl::GetCookie(const String& url, const String& name) const
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

Map<String, String> WebViewControl::GetCookies(const String& url) const
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

jint WebViewControl::nativeOnUrlChanged(JNIEnv* env, jstring jurl, jboolean jisRedirectedByMouseClick)
{
    String url = JNI::JavaStringToString(jurl, env);

    bool isRedirectedByMouseClick = jisRedirectedByMouseClick == JNI_TRUE;
    IUIWebViewDelegate::eAction action = IUIWebViewDelegate::PROCESS_IN_WEBVIEW;
    RunOnMainThread([this, url, isRedirectedByMouseClick, &action]() {
        action = OnUrlChanged(url, isRedirectedByMouseClick);
    });

    return static_cast<jint>(action);
}

void WebViewControl::nativeOnPageLoaded(JNIEnv* env)
{
    RunOnMainThreadAsync([this]() {
        OnPageLoaded();
    });
}

void WebViewControl::nativeOnExecuteJavaScript(JNIEnv* env, jstring jresult)
{
    String result = JNI::JavaStringToString(jresult, env);
    RunOnMainThreadAsync([this, result]() {
        OnExecuteJavaScript(result);
    });
}

IUIWebViewDelegate::eAction WebViewControl::OnUrlChanged(const String& url, bool isRedirectedByMouseClick)
{
    IUIWebViewDelegate::eAction action = IUIWebViewDelegate::PROCESS_IN_WEBVIEW;
    if (uiWebViewDelegate != nullptr)
    {
        action = uiWebViewDelegate->URLChanged(uiWebView, url, isRedirectedByMouseClick);
    }
    return action;
}

void WebViewControl::OnPageLoaded()
{
    if (uiWebViewDelegate != nullptr)
    {
        uiWebViewDelegate->PageLoaded(uiWebView);
    }
}

void WebViewControl::OnExecuteJavaScript(const String& result)
{
    if (uiWebViewDelegate != nullptr)
    {
        uiWebViewDelegate->OnExecuteJScript(uiWebView, result);
    }
}

} // namespace DAVA

#else // __DAVAENGINE_COREV2__

#include "Base/Platform.h"

#include "Logger/Logger.h"
#include "Utils/UTF8Utils.h"
#include "Utils/Utils.h"
#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"
#include "UI/Private/Android/WebViewControlAndroid.h"

#include "Render/Texture.h"
#include "Render/2D/Sprite.h"
#include "UI/UIWebView.h"

namespace DAVA
{
int32 WebViewControl::webViewIdCount = 0;
JniWebView::CONTROLS_MAP JniWebView::controls;

JniWebView::JniWebView()
    : jniWebView("com/dava/framework/JNIWebView")
{
    initialize = jniWebView.GetStaticMethod<void, jint, jfloat, jfloat, jfloat, jfloat>("Initialize");
    deinitialize = jniWebView.GetStaticMethod<void, jint>("Deinitialize");
    openURL = jniWebView.GetStaticMethod<void, jint, jstring>("OpenURL");
    loadHtmlString = jniWebView.GetStaticMethod<void, jint, jstring>("LoadHtmlString");
    executeJScript = jniWebView.GetStaticMethod<void, jint, jstring>("ExecuteJScript");
    getCookie = jniWebView.GetStaticMethod<jstring, jstring, jstring>("GetCookie");
    getCookies = jniWebView.GetStaticMethod<jobjectArray, jstring>("GetCookies");
    deleteCookies = jniWebView.GetStaticMethod<void, jstring>("DeleteCookies");
    openFromBuffer = jniWebView.GetStaticMethod<void, jint, jstring, jstring>("OpenFromBuffer");
    setRect = jniWebView.GetStaticMethod<void, jint, jfloat, jfloat, jfloat, jfloat>("SetRect");
    setVisible = jniWebView.GetStaticMethod<void, jint, jboolean>("SetVisible");
    setBackgroundTransparency = jniWebView.GetStaticMethod<void, jint, jboolean>("SetBackgroundTransparency");
    setRenderToTexture = jniWebView.GetStaticMethod<void, jint, jboolean>("setRenderToTexture");
    isRenderToTexture = jniWebView.GetStaticMethod<jboolean, jint>("isRenderToTexture");
    willDraw = jniWebView.GetStaticMethod<void, jint>("WillDraw");
}

void JniWebView::Initialize(WebViewControl* control, int id, const Rect& controlRect)
{
    controls[id] = control;
    Rect rect = UIControlSystem::Instance()->vcs->ConvertVirtualToInput(controlRect);

    rect.dx = std::max(0.0f, rect.dx);
    rect.dy = std::max(0.0f, rect.dy);

    initialize(id, rect.x, rect.y, rect.dx, rect.dy);
}

void JniWebView::Deinitialize(int id)
{
    controls.erase(id);
    deinitialize(id);
}

void JniWebView::OpenURL(int id, const String& urlToOpen)
{
    JNIEnv* env = JNI::GetEnv();
    jstring jUrlToOpen = env->NewStringUTF(urlToOpen.c_str());

    openURL(id, jUrlToOpen);

    env->DeleteLocalRef(jUrlToOpen);
}

void JniWebView::LoadHtmlString(int id, const String& htmlString)
{
    JNIEnv* env = JNI::GetEnv();

    jstring jHtmlStringToOpen = env->NewStringUTF(htmlString.c_str());

    loadHtmlString(id, jHtmlStringToOpen);

    env->DeleteLocalRef(jHtmlStringToOpen);
}

void JniWebView::ExecuteJScript(int id, const String& scriptString)
{
    JNIEnv* env = JNI::GetEnv();

    jstring jScriptToExecute = env->NewStringUTF(scriptString.c_str());

    executeJScript(id, jScriptToExecute);

    env->DeleteLocalRef(jScriptToExecute);
}

void JniWebView::DeleteCookies(const String& targetURL)
{
    JNIEnv* env = JNI::GetEnv();
    jstring jTargetURL = env->NewStringUTF(targetURL.c_str());

    deleteCookies(jTargetURL);

    env->DeleteLocalRef(jTargetURL);
}

String JniWebView::GetCookie(const String& targetUrl, const String& cookieName)
{
    JNIEnv* env = JNI::GetEnv();

    String returnStr;

    jstring jTargetURL = env->NewStringUTF(targetUrl.c_str());
    jstring jName = env->NewStringUTF(cookieName.c_str());
    jobject item = getCookie(jTargetURL, jName);

    returnStr = JNI::ToString(jstring(item));

    env->DeleteLocalRef(jTargetURL);
    env->DeleteLocalRef(jName);
    env->DeleteLocalRef(item);

    return returnStr;
}

Map<String, String> JniWebView::GetCookies(const String& targetUrl)
{
    JNIEnv* env = JNI::GetEnv();

    Map<String, String> cookiesMap;

    jstring jTargetURL = env->NewStringUTF(targetUrl.c_str());

    jobjectArray jArray = getCookies(jTargetURL);
    if (jArray)
    {
        jsize size = env->GetArrayLength(jArray);
        for (jsize i = 0; i < size; ++i)
        {
            jobject item = env->GetObjectArrayElement(jArray, i);
            String cookiesString = JNI::ToString(jstring(item));
            env->DeleteLocalRef(item);

            Vector<String> cookieEntry;
            Split(cookiesString, "=", cookieEntry);

            DVASSERT(1 < cookieEntry.size());
            cookiesMap[cookieEntry[0]] = cookieEntry[1];
        }

        env->DeleteLocalRef(jArray);
    }

    env->DeleteLocalRef(jTargetURL);

    return cookiesMap;
}

void JniWebView::OpenFromBuffer(int id, const String& string, const String& baseUrl)
{
    JNIEnv* env = JNI::GetEnv();

    jstring jString = env->NewStringUTF(string.c_str());
    jstring jBaseUrl = env->NewStringUTF(baseUrl.c_str());

    openFromBuffer(id, jString, jBaseUrl);

    env->DeleteLocalRef(jString);
    env->DeleteLocalRef(jBaseUrl);
}

void JniWebView::SetRect(int id, const Rect& controlRect)
{
    Rect rect = UIControlSystem::Instance()->vcs->ConvertVirtualToInput(controlRect);

    rect.dx = std::max(0.0f, rect.dx);
    rect.dy = std::max(0.0f, rect.dy);

    setRect(id, rect.x, rect.y, rect.dx, rect.dy);
}

void JniWebView::SetVisible(int id, bool isVisible)
{
    setVisible(id, isVisible);
}

void JniWebView::SetBackgroundTransparency(int id, bool enabled)
{
    setBackgroundTransparency(id, enabled);
}

void JniWebView::SetRenderToTexture(int id, bool renderToTexture)
{
    setRenderToTexture(id, renderToTexture);
}

bool JniWebView::IsRenderToTexture(int id)
{
    return isRenderToTexture(id) == 0 ? false : true;
}

void JniWebView::WillDraw(int id)
{
    willDraw(id);
}

IUIWebViewDelegate::eAction JniWebView::URLChanged(int id, const String& newURL, bool isRedirectedByMouseClick)
{
    CONTROLS_MAP::iterator iter = controls.find(id);
    if (iter == controls.end())
    {
        Logger::Error("Error web view id=%d", id);
        return IUIWebViewDelegate::PROCESS_IN_WEBVIEW;
    }

    WebViewControl* control = iter->second;
    IUIWebViewDelegate* delegate = control->delegate;
    if (!delegate)
        return IUIWebViewDelegate::PROCESS_IN_WEBVIEW;

    return delegate->URLChanged(&control->webView, newURL, isRedirectedByMouseClick);
}

void JniWebView::PageLoaded(int id, int* rawPixels, int width, int height)
{
    CONTROLS_MAP::iterator iter = controls.find(id);
    if (iter == controls.end())
    {
        Logger::Error("Error web view id=%d", id);
        return;
    }

    WebViewControl* control = iter->second;
    IUIWebViewDelegate* delegate = control->delegate;
    if (delegate)
    {
        delegate->PageLoaded(&control->webView);
    }

    UIWebView& webView = control->GetUIWebView();
    if (rawPixels)
    {
        {
            Texture* tex = Texture::CreateFromData(FORMAT_RGBA8888,
                                                   reinterpret_cast<uint8*>(rawPixels), width, height, false);
            Rect rect = webView.GetRect();
            {
                Sprite* spr = Sprite::CreateFromTexture(tex, 0, 0, width, height, rect.dx, rect.dy);
                UIControlBackground* bg = webView.GetOrCreateComponent<UIControlBackground>();
                bg->SetSprite(spr, 0);
                SafeRelease(spr);
            }
            SafeRelease(tex);
        }
    }
    else
    {
        // reset sprite to prevent render old sprite under native webveiw
        webView.RemoveComponent(UIComponent::BACKGROUND_COMPONENT);
    }
}

void JniWebView::OnExecuteJScript(int id, const String& result)
{
    CONTROLS_MAP::iterator iter = controls.find(id);
    if (iter == controls.end())
    {
        Logger::Error("Error web view id=%d", id);
        return;
    }

    WebViewControl* control = iter->second;
    IUIWebViewDelegate* delegate = control->delegate;
    if (delegate)
    {
        delegate->OnExecuteJScript(&control->webView, result);
    }
}

WebViewControl::WebViewControl(UIWebView* uiWebView)
    :
    webViewId(-1)
    ,
    delegate(nullptr)
    ,
    webView(*uiWebView)
{
    webViewId = webViewIdCount++;
}

WebViewControl::~WebViewControl()
{
    JniWebView jniWebView;
    jniWebView.Deinitialize(webViewId);
}

void WebViewControl::Initialize(const Rect& rect)
{
    JniWebView jniWebView;
    jniWebView.Initialize(this, webViewId, rect);
}

void WebViewControl::OpenURL(const String& urlToOpen)
{
    JniWebView jniWebView;
    jniWebView.OpenURL(webViewId, urlToOpen);
}

void WebViewControl::LoadHtmlString(const WideString& urlToOpen)
{
    JniWebView jniWebView;
    jniWebView.LoadHtmlString(webViewId, UTF8Utils::EncodeToUTF8(urlToOpen));
}

void WebViewControl::DeleteCookies(const String& targetUrl)
{
    JniWebView jniWebView;
    jniWebView.DeleteCookies(targetUrl);
}

String WebViewControl::GetCookie(const String& url, const String& name) const
{
    JniWebView jniWebView;
    return jniWebView.GetCookie(url, name);
}
// Get the list of cookies for specific domain
Map<String, String> WebViewControl::GetCookies(const String& url) const
{
    JniWebView jniWebView;
    return jniWebView.GetCookies(url);
}

void WebViewControl::ExecuteJScript(const String& scriptString)
{
    JniWebView jniWebView;
    jniWebView.ExecuteJScript(webViewId, scriptString);
}

void WebViewControl::OpenFromBuffer(const String& data, const FilePath& urlToOpen)
{
    JniWebView jniWebView;
    jniWebView.OpenFromBuffer(webViewId, data, urlToOpen.AsURL());
}

void WebViewControl::SetRect(const Rect& rect)
{
    JniWebView jniWebView;
    jniWebView.SetRect(webViewId, rect);
}

void WebViewControl::SetVisible(bool isVisible, bool hierarchic)
{
    JniWebView jniWebView;
    jniWebView.SetVisible(webViewId, isVisible);
}

void WebViewControl::SetDelegate(DAVA::IUIWebViewDelegate* delegate, DAVA::UIWebView*)
{
    this->delegate = delegate;
}

void WebViewControl::SetBackgroundTransparency(bool enabled)
{
    JniWebView jniWebView;
    jniWebView.SetBackgroundTransparency(webViewId, enabled);
}

void WebViewControl::SetRenderToTexture(bool value)
{
    JniWebView jniWebView;
    jniWebView.SetRenderToTexture(webViewId, value);
}
bool WebViewControl::IsRenderToTexture() const
{
    JniWebView jniWebView;
    return jniWebView.IsRenderToTexture(webViewId);
}

void WebViewControl::WillDraw()
{
    JniWebView jniWebView;
    jniWebView.WillDraw(webViewId);
}

} //namespace DAVA

#endif // !__DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__
#endif // !DISABLE_NATIVE_WEBVIEW

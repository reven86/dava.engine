/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "FileSystem/Logger.h"
#include "Utils/UTF8Utils.h"
#include "Utils/Utils.h"
#include "ExternC/AndroidLayer.h"
#include "Platform/TemplateAndroid/JniHelpers.h"
#include "Platform/TemplateAndroid/WebViewControlAndroid.h"

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
	Rect rect = JNI::V2P(controlRect);
	initialize(id, rect.x, rect.y, rect.dx, rect.dy);
}

void JniWebView::Deinitialize(int id)
{
	controls.erase(id);
	deinitialize(id);
}

void JniWebView::OpenURL(int id, const String& urlToOpen)
{
	JNIEnv *env = JNI::GetEnv();
	jstring jUrlToOpen = env->NewStringUTF(urlToOpen.c_str());

	openURL(id, jUrlToOpen);

	env->DeleteLocalRef(jUrlToOpen);
}

void JniWebView::LoadHtmlString(int id, const String& htmlString)
{
	JNIEnv *env = JNI::GetEnv();

	jstring jHtmlStringToOpen = env->NewStringUTF(htmlString.c_str());

	loadHtmlString(id, jHtmlStringToOpen);

	env->DeleteLocalRef(jHtmlStringToOpen);
}

void JniWebView::ExecuteJScript(int id, const String& scriptString)
{
	JNIEnv *env = JNI::GetEnv();

	jstring jScriptToExecute = env->NewStringUTF(scriptString.c_str());

	executeJScript(id, jScriptToExecute);

	env->DeleteLocalRef(jScriptToExecute);
}

void JniWebView::DeleteCookies(const String& targetURL)
{
	JNIEnv *env = JNI::GetEnv();
	jstring jTargetURL = env->NewStringUTF(targetURL.c_str());

	deleteCookies(jTargetURL);

	env->DeleteLocalRef(jTargetURL);
}

String JniWebView::GetCookie(const String& targetUrl, const String& cookieName)
{
	JNIEnv *env = JNI::GetEnv();

	String returnStr = "";

    jstring jTargetURL = env->NewStringUTF(targetUrl.c_str());
    jstring jName = env->NewStringUTF(cookieName.c_str());
    jobject item = getCookie(jTargetURL, jName);

    JNI::CreateStringFromJni(jstring(item), returnStr);

    env->DeleteLocalRef(jTargetURL);
    env->DeleteLocalRef(jName);

	return returnStr;
}

Map<String, String> JniWebView::GetCookies(const String& targetUrl)
{
	JNIEnv *env = JNI::GetEnv();

	Map<String, String> cookiesMap;

	jstring jTargetURL = env->NewStringUTF(targetUrl.c_str());

    jobjectArray jArray = getCookies(jTargetURL);
    if (jArray)
    {
        jsize size = env->GetArrayLength(jArray);
        for (jsize i = 0; i < size; ++i)
        {
            jobject item = env->GetObjectArrayElement(jArray, i);
            String cookiesString = "";
            JNI::CreateStringFromJni(jstring(item), cookiesString);

            Vector<String> cookieEntry;
            Split(cookiesString, "=", cookieEntry);

            DVASSERT(1 < cookieEntry.size());
            cookiesMap[cookieEntry[0]] = cookieEntry[1];
        }
    }

    env->DeleteLocalRef(jTargetURL);

	return cookiesMap;
}

void JniWebView::OpenFromBuffer(int id, const String& string, const String& baseUrl)
{
	JNIEnv *env = JNI::GetEnv();

	jstring jString = env->NewStringUTF(string.c_str());
	jstring jBaseUrl = env->NewStringUTF(baseUrl.c_str());

	openFromBuffer(id, jString, jBaseUrl);

	env->DeleteLocalRef(jString);
	env->DeleteLocalRef(jBaseUrl);
}

void JniWebView::SetRect(int id, const Rect& controlRect)
{
	Rect rect = JNI::V2P(controlRect);
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
	IUIWebViewDelegate *delegate = control->delegate;
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
	IUIWebViewDelegate *delegate = control->delegate;
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
                webView.GetBackground()->SetSprite(spr, 0);
                SafeRelease(spr);
            }
            SafeRelease(tex);
		}
	} else
	{
		// reset sprite to prevent render old sprite under native webveiw
		webView.SetSprite(nullptr, 0);
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
	IUIWebViewDelegate *delegate = control->delegate;
	if (delegate)
	{
		delegate->OnExecuteJScript(&control->webView, result);
	}
}

WebViewControl::WebViewControl(UIWebView& uiWebView):
	webViewId(-1),
	delegate(nullptr),
	webView(uiWebView)
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

void WebViewControl::SetDelegate(DAVA::IUIWebViewDelegate *delegate, DAVA::UIWebView*)
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

}//namespace DAVA

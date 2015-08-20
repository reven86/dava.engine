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


#ifndef __DAVAENGINE_WEBVIEWCONTROL_H__
#define __DAVAENGINE_WEBVIEWCONTROL_H__

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_ANDROID__)

#include "../../UI/IWebViewControl.h"
#include "Platform/TemplateAndroid/JniHelpers.h"

namespace DAVA {

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

	void SetDelegate(IUIWebViewDelegate *delegate, UIWebView*) override;
	void SetBackgroundTransparency(bool enabled) override;

	void SetRenderToTexture(bool value) override;
    bool IsRenderToTexture() const override;

    UIWebView& GetUIWebView() const {return webView;}

    void WillDraw() override;

private:
	int32 webViewId;
	IUIWebViewDelegate *delegate;
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

	Function<void (jint, jfloat, jfloat, jfloat, jfloat)> initialize;
	Function<void (jint)> deinitialize;
	Function<void (jint, jstring)> openURL;
	Function<void (jint, jstring)> loadHtmlString;
	Function<void (jint, jstring)> executeJScript;
	Function<jstring (jstring, jstring)> getCookie;
	Function<jobjectArray (jstring)> getCookies;
	Function<void (jstring)> deleteCookies;
	Function<void (jint, jstring, jstring)> openFromBuffer;
	Function<void (jint, jfloat, jfloat, jfloat, jfloat)> setRect;
	Function<void (jint, jboolean)> setVisible;
	Function<void (jint, jboolean)> setBackgroundTransparency;
	Function<void (jint, jboolean)> setRenderToTexture;
	Function<jboolean (jint)> isRenderToTexture;
	Function<void (jint)> willDraw;
};

};

#endif //#if defined(__DAVAENGINE_ANDROID__)

#endif /* defined(__DAVAENGINE_WEBVIEWCONTROL_MACOS_H__) */

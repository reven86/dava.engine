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
#include "JniExtensions.h"

namespace DAVA {

class JniWebView;

// Web View Control - Android version.
class WebViewControl : public IWebViewControl
{
	friend class JniWebView;
public:
	WebViewControl();
	virtual ~WebViewControl();
	
	// Initialize the control.
	virtual void Initialize(const Rect& rect);
	
	// Open the URL requested.
	virtual void OpenURL(const String& urlToOpen);
	// Load html page from string
	virtual void LoadHtmlString(const WideString& htmlString);
	// Delete all cookies associated with target URL
	virtual void DeleteCookies(const String& targetUrl);
	// Get cookie for specific domain and name
	virtual String GetCookie(const String& url, const String& name);
	// Get the list of cookies for specific domain
	virtual Map<String, String> GetCookies(const String& url);
	// Perform Java script
	virtual int32_t ExecuteJScript(const String& scriptString);

	void OpenFromBuffer(const String& string, const FilePath& basePath);

	// Size/pos/visibility changes.
	virtual void SetRect(const Rect& rect);
	virtual void SetVisible(bool isVisible, bool hierarchic);

	virtual void SetDelegate(IUIWebViewDelegate *delegate, UIWebView* webView);
	virtual void SetBackgroundTransparency(bool enabled);

private:
	static int32_t webViewIdCount;
	int32_t webViewId;
	IUIWebViewDelegate *delegate;
	UIWebView* webView;
	static int32_t requestId;
};

class JniWebView: public JniExtension
{
public:
	void Initialize(WebViewControl* control, int id, const Rect& rect);
	void Deinitialize(int id);

	void OpenURL(int id, const String& urlToOpen);
	void LoadHtmlString(int id, const String& htmlString);
	void DeleteCookies(int id, const String& targetUrl);
	String GetCookie(const String& targetUrl, const String& name);
	String GetCookies(const String& targetUrl);
	void ExecuteJScript(int id, int requestId, const String& scriptString);
	void OpenFromBuffer(int id, const String& string, const String& basePath);

	void SetRect(int id, const Rect& rect);
	void SetVisible(int id, bool isVisible);

	void SetBackgroundTransparency(int id, bool isVisible);

	static IUIWebViewDelegate::eAction URLChanged(int id, const String& newURL);
	static void PageLoaded(int id);
	static void OnExecuteJScript(int id, int requestId, const String& result);

protected:
	virtual jclass GetJavaClass() const;
	virtual const char* GetJavaClassName() const;

public:
	static jclass gJavaClass;
	static const char* gJavaClassName;
	static String returnStr;

private:
	typedef std::map<int, WebViewControl*> CONTROLS_MAP;
	static CONTROLS_MAP controls;
};

};

#endif //#if defined(__DAVAENGINE_ANDROID__)

#endif /* defined(__DAVAENGINE_WEBVIEWCONTROL_MACOS_H__) */

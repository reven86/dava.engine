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



#ifndef __DAVAENGINE_WEBVIEWCONTROL_MACOS_H__
#define __DAVAENGINE_WEBVIEWCONTROL_MACOS_H__

#include "../../UI/IWebViewControl.h"

namespace DAVA {

class UIWebView;
    
// Web View Control - MacOS version.
class WebViewControl : public IWebViewControl
{
public:
	explicit WebViewControl(UIWebView& uiWebView);
	virtual ~WebViewControl();
	
	// Initialize the control.
    void Initialize(const Rect& rect) override;
	
	// Open the URL requested.
    void OpenURL(const String& urlToOpen) override;
	// Load html page from string
    void LoadHtmlString(const WideString& htlmString) override;
	// Delete all cookies associated with target URL
    void DeleteCookies(const String& targetUrl) override;
    // Execute javascript command, return request ID
    void ExecuteJScript(const String& scriptString) override;
    
    void OpenFromBuffer(const String& string, const FilePath& basePath) override;

	// Size/pos/visibility changes.
    void SetRect(const Rect& rect) override;
    void SetVisible(bool isVisible, bool hierarchic) override;

	void SetDelegate(DAVA::IUIWebViewDelegate *delegate,
                     DAVA::UIWebView* webView) override;
	void SetBackgroundTransparency(bool enabled) override;
    
    void SetRenderToTexture(bool value) override;
    bool IsRenderToTexture() const override {return isRenderToTexture;}
    
    void SetImageCache(void* ptr);
    void* GetImageCache() const;

    void RenderToTextureAndSetAsBackgroundSpriteToControl(DAVA::UIWebView&
                                                          uiWebViewControl);
private:
    
	//A pointer to MacOS WebView.
	void* webViewPtr;
	
	// A pointer to the WebView delegate.
	void* webViewDelegatePtr;

	void* webViewPolicyDelegatePtr;
    // A pointer to NSBitmapImageRep cached image of web view to texture
    void* webImageCachePtr;
    
    UIWebView& uiWebViewControl;
    
    bool isRenderToTexture;
    bool isVisible;
};

};

#endif /* defined(__DAVAENGINE_WEBVIEWCONTROL_MACOS_H__) */

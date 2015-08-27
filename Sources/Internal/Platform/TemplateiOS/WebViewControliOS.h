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


#ifndef __DAVAENGINE_WEBVIEWCONTROL_IOS_H__
#define __DAVAENGINE_WEBVIEWCONTROL_IOS_H__

#include "../../UI/IWebViewControl.h"

namespace DAVA {

class UIControl;
class FilePath;
class UIWebView;
    
// Web View Control - iOS version.
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
	void LoadHtmlString(const WideString& htmlString) override;
	// Delete all cookies associated with target URL
	void DeleteCookies(const String& targetUrl) override;
	// Get cookie for specific domain and name
	String GetCookie(const String& targetUrl, const String& name) const override;
	// Get the list of cookies for specific domain
    Map<String, String> GetCookies(const String& targetUrl) const override;
	// Perfrom Java script
    void ExecuteJScript(const String& scriptString) override;

    void OpenFromBuffer(const String& string, const FilePath& basePath) override;
    
	// Size/pos/visibility changes.
	void SetRect(const Rect& rect) override;
    void SetVisible(bool isVisible, bool hierarchic) override;

    void SetScalesPageToFit(bool isScalesToFit) override;

	void SetDelegate(IUIWebViewDelegate *delegate,
                             UIWebView* webView) override;
	void SetBackgroundTransparency(bool enabled) override;

	// Bounces control.
	bool GetBounces() const override;
	void SetBounces(bool value) override;
    void SetGestures(bool value) override;

    // Data detector types.
    void SetDataDetectorTypes(int32 value) override;
    int32 GetDataDetectorTypes() const override;
    
    void SetRenderToTexture(bool value) override;
    bool IsRenderToTexture() const override {return pendingRenderToTexture;}
    
    void WillDraw() override;
    
    // common ios part to render any UIView* to UIImage*
    // workaroundKeyboardBug - if call during show/hide keyboard - false
    static void* RenderIOSUIViewToImage(void* uiviewPtr);
    // common ios part to copy from ios ::UIImage* to DAVA::Sprite*
    static void SetImageAsSpriteToControl(void* imagePtr, UIControl& control);
    
    void RenderToTextureAndSetAsBackgroundSpriteToControl(UIWebView& control);

private:

	//A pointer to iOS WebView.
	void* webViewPtr;
	
	// A pointer to the WebView delegate.

	void* webViewURLDelegatePtr;

    void* rightSwipeGesturePtr;
    void* leftSwipeGesturePtr;

    
	Map<void*, bool> subviewVisibilityMap;

	void HideSubviewImages(void* view);
	void RestoreSubviewImages();
    
    bool gesturesEnabled;
    bool isRenderToTexture;
    bool pendingRenderToTexture;
    bool isVisible;
    bool pendingVisible;
    
    UIWebView& uiWebView;
};

};

#endif /* defined(__DAVAENGINE_WEBVIEWCONTROL_IOS_H__) */

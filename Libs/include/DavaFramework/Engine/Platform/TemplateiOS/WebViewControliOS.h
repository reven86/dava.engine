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

class FilePath;
    
// Web View Control - MacOS version.
class WebViewControl : public IWebViewControl
{
public:
	WebViewControl();
	virtual ~WebViewControl();
	
	// Initialize the control.
	virtual void Initialize(const Rect& rect);
	
	// Open the URL requested.
	virtual void OpenURL(const String& urlToOpen);

    virtual void OpenFromBuffer(const String& string, const FilePath& basePath);
    
	// Size/pos/visibility changes.
	virtual void SetRect(const Rect& rect);
	virtual void SetVisible(bool isVisible, bool hierarchic);

	virtual void SetDelegate(DAVA::IUIWebViewDelegate *delegate, DAVA::UIWebView* webView);
	virtual void SetBackgroundTransparency(bool enabled);

	// Bounces control.
	virtual bool GetBounces() const;
	virtual void SetBounces(bool value);
    virtual void SetGestures(bool value);

    // Data detector types.
    virtual void SetDataDetectorTypes(int32 value);
    virtual int32 GetDataDetectorTypes() const;

protected:
	// Get the scale divider for Retina devices.
	float GetScaleDivider();

	//A pointer to iOS WebView.
	void* webViewPtr;
	
	// A pointer to the WebView delegate.
	void* webViewDelegatePtr;

	void* webViewURLDelegatePtr;

    void *rightSwipeGesturePtr;
    void *leftSwipeGesturePtr;

    
	Map<void*, bool> subviewVisibilityMap;

	void HideSubviewImages(void* view);
	void RestoreSubviewImages();
    bool gesturesEnabled;
};

};

#endif /* defined(__DAVAENGINE_WEBVIEWCONTROL_IOS_H__) */

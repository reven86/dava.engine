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



#ifndef __DAVAENGINE_UIWEBVIEW_H__
#define __DAVAENGINE_UIWEBVIEW_H__

#include "UIControl.h"
#include "IWebViewControl.h"

namespace DAVA {
	
// The purpose of UIWebView class is displaying embedded Web Page Controls.
class UIWebView : public UIControl
{
protected:
	virtual ~UIWebView();
public:
    // Data detector types. May be a combination of several flags.
    enum eDataDetectorType
    {
        DATA_DETECTOR_NONE              = 0x00,
        DATA_DETECTOR_PHONE_NUMBERS     = 0x01,
        DATA_DETECTOR_LINKS             = 0x02,
        DATA_DETECTOR_ADDRESSES         = 0x04,
        DATA_DETECTOR_CALENDAR_EVENTS   = 0x08,
        DATA_DETECTOR_ALL               = 0xFF
    };

	UIWebView(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
		
	// Open the URL.
	void OpenURL(const String& urlToOpen);
	// Load html page
	void LoadHtmlString(const WideString& htmlString);
	// Delete all cookies for target URL
	void DeleteCookies(const String& targetUrl);
	// Get cookie for specific domain and name
	String GetCookie(const String& targetUrl, const String& name) const;
	// Get the list of cookies for specific domain
	Map<String, String> GetCookies(const String& targetUrl) const;
	// Perfrom Java script, return script id
	int32_t ExecuteJScript(const String& scriptString);
	
    void OpenFromBuffer(const String& string, const FilePath& basePath);
    
	// Overloaded virtual methods.
	virtual void WillAppear();
	virtual void WillDisappear();

	virtual void SetPosition(const Vector2 &position, bool positionInAbsoluteCoordinates = false);
	virtual void SetSize(const Vector2 &newSize);
	virtual void SetVisible(bool isVisible, bool hierarchic = true);
	
	// Page scale property change
	void SetScalesPageToFit(bool isScalesToFit);

    virtual void LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader);
	virtual YamlNode * SaveToYamlNode(UIYamlLoader * loader);

    virtual UIControl* Clone();
    virtual void CopyDataFrom(UIControl *srcControl);

    void SetNativeControlVisible(bool isVisible);
    bool GetNativeControlVisible() const;

	void SetDelegate(IUIWebViewDelegate* delegate);
	void SetBackgroundTransparency(bool enabled);

	// Enable/disable bounces.
	void SetBounces(bool value);
	bool GetBounces() const;
	void SetGestures(bool value);
    
    // Set the data detector types.
    void SetDataDetectorTypes(int32 value);
    int32 GetDataDetectorTypes() const;

protected:

    // Set the visibility of native control.
    void UpdateNativeControlVisible(bool value, bool hierarchic);

    // Update the rect of the web view control.
    void UpdateControlRect();

	// Platform-specific implementation of the Web View Control.
	IWebViewControl* webViewControl;
    
private:
    bool isNativeControlVisible;
    int32 dataDetectorTypes;
};
};

#endif /* defined(__DAVAENGINE_UIWEBVIEW_H__) */

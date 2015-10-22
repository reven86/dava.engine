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

    UIWebView(const Rect& rect = Rect());

protected:
    virtual ~UIWebView();

public:
    // Open the URL.
    void OpenFile(const FilePath &path);
	void OpenURL(const String& urlToOpen);
	// Load html page
	void LoadHtmlString(const WideString& htmlString);
	// Delete all cookies for target URL
	void DeleteCookies(const String& targetUrl);
	// Get cookie for specific domain and name
	String GetCookie(const String& targetUrl, const String& name) const;
	// Get the list of cookies for specific domain
	Map<String, String> GetCookies(const String& targetUrl) const;
	// Perform Java script
	// if you need return data from javascript just
	// return JSON string you can parse it in c++
	// with yaml parser, call back with JSON will come to
	// IUIWebViewDelegate::OnExecuteJScript
	void ExecuteJScript(const String& scriptString);
    
    void OpenFromBuffer(const String& string, const FilePath& basePath);
    
	// Overloaded virtual methods.
	void SetPosition(const Vector2 &position) override;
	void SetSize(const Vector2 &newSize) override;

	// Page scale property change
	void SetScalesPageToFit(bool isScalesToFit);

    void LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader) override;
	YamlNode * SaveToYamlNode(UIYamlLoader * loader) override;

    UIWebView* Clone() override;
    void CopyDataFrom(UIControl *srcControl) override;
    
    void SystemDraw(const UIGeometricData &geometricData) override;

protected:
    void WillBecomeVisible() override;
    void WillBecomeInvisible() override;
    void DidAppear() override;

public:
    void SetRenderToTexture(bool value);
    bool IsRenderToTexture() const;

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
    void SetNativeControlVisible(bool isVisible);
    bool GetNativeControlVisible() const;
    // Set the visibility of native control.
    void UpdateNativeControlVisible(bool value);

    // Update the rect of the web view control.
    void UpdateControlRect();

	// Platform-specific implementation of the Web View Control.
	IWebViewControl* webViewControl;
    
private:
    bool isNativeControlVisible;
    int32 dataDetectorTypes;
public:
    INTROSPECTION_EXTEND(UIWebView, UIControl,
                         PROPERTY("dataDetectorTypes", InspDesc("Data detector types", GlobalEnumMap<eDataDetectorType>::Instance(), InspDesc::T_FLAGS), GetDataDetectorTypes, SetDataDetectorTypes, I_SAVE | I_VIEW | I_EDIT));
};
};

#endif /* defined(__DAVAENGINE_UIWEBVIEW_H__) */

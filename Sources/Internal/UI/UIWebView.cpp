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



#include "UIWebView.h"
#include "Render/RenderManager.h"

#if defined(__DAVAENGINE_MACOS__)
#include "../Platform/TemplateMacOS/WebViewControlMacOS.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "../Platform/TemplateIOS/WebViewControliOS.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "../Platform/TemplateWin32/WebViewControlWin32.h"
#elif defined(__DAVAENGINE_ANDROID__)
#include "../Platform/TemplateAndroid/WebViewControl.h"
#else
#pragma error UIWEbView control is not implemented for this platform yet!
#endif

namespace DAVA {

UIWebView::UIWebView(const Rect &rect, bool rectInAbsoluteCoordinates) :
    webViewControl(new WebViewControl()),
    UIControl(rect, rectInAbsoluteCoordinates),
    isNativeControlVisible(true)
{
    Rect newRect = GetRect(true);
    this->webViewControl->Initialize(newRect);
    UpdateControlRect();

    UpdateNativeControlVisible(false, true); // will be displayed in WillAppear.
    SetDataDetectorTypes(DATA_DETECTOR_LINKS);
}

UIWebView::~UIWebView()
{
	SafeDelete(webViewControl);
};

void UIWebView::SetDelegate(IUIWebViewDelegate* delegate)
{
	webViewControl->SetDelegate(delegate, this);
}

void UIWebView::OpenURL(const String& urlToOpen)
{
	webViewControl->OpenURL(urlToOpen);
}

void UIWebView::LoadHtmlString(const WideString& htmlString)
{
	webViewControl->LoadHtmlString(htmlString);
}

String UIWebView::GetCookie(const String& targetUrl, const String& name) const
{
	return webViewControl->GetCookie(targetUrl, name);
}

Map<String, String> UIWebView::GetCookies(const String& targetUrl) const
{
	return webViewControl->GetCookies(targetUrl);
}

void UIWebView::DeleteCookies(const String& targetUrl)
{
	webViewControl->DeleteCookies(targetUrl);
}

int32_t UIWebView::ExecuteJScript(const String& scriptString)
{
	return webViewControl->ExecuteJScript(scriptString);
}

void UIWebView::OpenFromBuffer(const String& string, const FilePath& basePath)
{
    this->webViewControl->OpenFromBuffer(string, basePath);
}

void UIWebView::WillAppear()
{
    UIControl::WillAppear();
    UpdateNativeControlVisible(GetVisible(), true);
}

void UIWebView::WillDisappear()
{
    UIControl::WillDisappear();
    UpdateNativeControlVisible(false, true);
}

void UIWebView::SetPosition(const Vector2 &position, bool positionInAbsoluteCoordinates)
{
	UIControl::SetPosition(position, positionInAbsoluteCoordinates);
    UpdateControlRect();
}

void UIWebView::SetSize(const Vector2 &newSize)
{
	UIControl::SetSize(newSize);
    UpdateControlRect();
}

void UIWebView::SetVisible(bool isVisible, bool hierarchic)
{
	UIControl::SetVisible(isVisible, hierarchic);
    if (IsOnScreen())
        UpdateNativeControlVisible(isVisible, hierarchic);
}

void UIWebView::SetScalesPageToFit(bool isScalesToFit)
{
	webViewControl->SetScalesPageToFit(isScalesToFit);
}

void UIWebView::SetBackgroundTransparency(bool enabled)
{
	webViewControl->SetBackgroundTransparency(enabled);
}

// Enable/disable bounces.
void UIWebView::SetBounces(bool value)
{
	webViewControl->SetBounces(value);
}

bool UIWebView::GetBounces() const
{
	return webViewControl->GetBounces();
}

void UIWebView::SetGestures(bool value)
{
	webViewControl->SetGestures(value);    
}

void UIWebView::UpdateControlRect()
{
    Rect rect = GetRect(true);

    rect.SetPosition(rect.GetPosition() * RenderManager::Instance()->GetDrawScale() + RenderManager::Instance()->GetDrawTranslate());
    rect.SetSize(rect.GetSize() * RenderManager::Instance()->GetDrawScale());

    webViewControl->SetRect(rect);
}

void UIWebView::SetNativeControlVisible(bool isVisible)
{
    isNativeControlVisible = isVisible;
    UpdateNativeControlVisible(GetVisible(), true);
}

bool UIWebView::GetNativeControlVisible() const
{
    return isNativeControlVisible;
}

void UIWebView::UpdateNativeControlVisible(bool value, bool hierarchic)
{
    // In case isDisplayNativeControl is set to false - always hide the native control.
    bool visibleValue = isNativeControlVisible ? value : false;
    webViewControl->SetVisible(visibleValue, hierarchic);
}

void UIWebView::SetDataDetectorTypes(int32 value)
{
    dataDetectorTypes = value;
	webViewControl->SetDataDetectorTypes(value);
}


int32 UIWebView::GetDataDetectorTypes() const
{
    return dataDetectorTypes;
}

void UIWebView::LoadFromYamlNode(const DAVA::YamlNode *node, DAVA::UIYamlLoader *loader)
{
    UIControl::LoadFromYamlNode(node, loader);
    
    const YamlNode * dataDetectorTypesNode = node->Get("dataDetectorTypes");
    if (dataDetectorTypesNode)
    {
        SetDataDetectorTypes(dataDetectorTypesNode->AsInt32());
    }
}

YamlNode* UIWebView::SaveToYamlNode(DAVA::UIYamlLoader *loader)
{
    UIWebView* baseControl = new UIWebView();
    YamlNode *node = UIControl::SaveToYamlNode(loader);
    
    // Data Detector Types.
    if (baseControl->GetDataDetectorTypes() != GetDataDetectorTypes())
    {
        node->Set("dataDetectorTypes", GetDataDetectorTypes());
    }
    
    SafeRelease(baseControl);
    return node;
}

UIControl* UIWebView::Clone()
{
    UIWebView* webView = new UIWebView(GetRect());
    webView->CopyDataFrom(this);
    return webView;
}

void UIWebView::CopyDataFrom(UIControl *srcControl)
{
    UIControl::CopyDataFrom(srcControl);

    UIWebView* webView = (UIWebView*) srcControl;
    SetNativeControlVisible(webView->GetNativeControlVisible());
    SetDataDetectorTypes(webView->GetDataDetectorTypes());
}


};

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

using namespace DAVA;

UIWebView::UIWebView(const Rect &rect, bool rectInAbsoluteCoordinates) :
    webViewControl(new WebViewControl()),
    UIControl(rect, rectInAbsoluteCoordinates)
{
    Rect newRect = GetRect(true);
    this->webViewControl->Initialize(newRect);
    this->webViewControl->SetVisible(false, true); // will be displayed in WillAppear.
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
	this->webViewControl->OpenURL(urlToOpen);
}

void UIWebView::OpenFromBuffer(const String& string, const FilePath& basePath)
{
    this->webViewControl->OpenFromBuffer(string, basePath);
}

void UIWebView::WillAppear()
{
    UIControl::WillAppear();
    webViewControl->SetVisible(GetVisible(), true);
}

void UIWebView::WillDisappear()
{
    UIControl::WillDisappear();
    webViewControl->SetVisible(false, true);
}

void UIWebView::SetPosition(const Vector2 &position, bool positionInAbsoluteCoordinates)
{
	UIControl::SetPosition(position, positionInAbsoluteCoordinates);

	// Update the inner control.
	Rect newRect = GetRect(true);
	this->webViewControl->SetRect(newRect);
}

void UIWebView::SetSize(const Vector2 &newSize)
{
	UIControl::SetSize(newSize);

	// Update the inner control.
	Rect newRect = GetRect(true);
	this->webViewControl->SetRect(newRect);
}

void UIWebView::SetVisible(bool isVisible, bool hierarchic)
{
	UIControl::SetVisible(isVisible, hierarchic);
    if (IsOnScreen())
        webViewControl->SetVisible(isVisible, hierarchic);
}

void UIWebView::SetBackgroundTransparency(bool enabled)
{
	this->webViewControl->SetBackgroundTransparency(enabled);
}

// Enable/disable bounces.
void UIWebView::SetBounces(bool value)
{
	this->webViewControl->SetBounces(value);
}

bool UIWebView::GetBounces() const
{
	return this->webViewControl->GetBounces();
}

void UIWebView::SetGestures(bool value)
{
	this->webViewControl->SetGestures(value);    
}

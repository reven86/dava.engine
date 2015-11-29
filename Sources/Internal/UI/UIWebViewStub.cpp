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

#include "Render/2D/TextBlockGraphicRender.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#ifdef __NO_NATIVE_WEBVIEW__

#include "Render/2D/GraphicFont.h"
#include "UIWebView.h"

namespace DAVA
{
UIWebView::UIWebView(const Rect& rect)
    : UIControl(rect)
    , webViewControl(nullptr)
    , isNativeControlVisible(false)
{
    background->SetDrawType(UIControlBackground::DRAW_FILL); //there is no good ways to indicate webview
}

UIWebView::~UIWebView()
{
}

void UIWebView::SetDelegate(IUIWebViewDelegate*)
{
}

void UIWebView::OpenFile(const FilePath&)
{
}

void UIWebView::OpenURL(const String&)
{
}

void UIWebView::LoadHtmlString(const WideString&)
{
}

String UIWebView::GetCookie(const String&, const String&) const
{
    return String();
}

Map<String, String> UIWebView::GetCookies(const String&) const
{
    return Map<String, String>();
}

void UIWebView::DeleteCookies(const String&)
{
}

void UIWebView::ExecuteJScript(const String&)
{
}

void UIWebView::OpenFromBuffer(const String&, const FilePath&)
{
}

void UIWebView::WillBecomeVisible()
{
}

void UIWebView::WillBecomeInvisible()
{
}

void UIWebView::DidAppear()
{
}

void UIWebView::SetPosition(const Vector2& position)
{
    UIControl::SetPosition(position);
}

void UIWebView::SetSize(const Vector2& newSize)
{
    UIControl::SetSize(newSize);
}

void UIWebView::SetScalesPageToFit(bool isScalesToFit)
{
}

void UIWebView::SetBackgroundTransparency(bool enabled)
{
}

// Enable/disable bounces.
void UIWebView::SetBounces(bool)
{
}

bool UIWebView::GetBounces() const
{
    return false; //like
}

void UIWebView::SetGestures(bool)
{
}

void UIWebView::UpdateControlRect()
{
}

void UIWebView::SetRenderToTexture(bool)
{
}

bool UIWebView::IsRenderToTexture() const
{
    return false;
}

void UIWebView::SetNativeControlVisible(bool)
{
}

bool UIWebView::GetNativeControlVisible() const
{
    return false;
}

void UIWebView::UpdateNativeControlVisible(bool)
{
}

void UIWebView::SetDataDetectorTypes(int32)
{
}

int32 UIWebView::GetDataDetectorTypes() const
{
    return 0;
}

void UIWebView::LoadFromYamlNode(const DAVA::YamlNode* node, DAVA::UIYamlLoader* loader)
{
    UIControl::LoadFromYamlNode(node, loader);
}

YamlNode* UIWebView::SaveToYamlNode(DAVA::UIYamlLoader* loader)
{
    ScopedPtr<UIWebView> baseControl(new UIWebView());
    YamlNode* node = UIControl::SaveToYamlNode(loader);
    return node;
}

UIWebView* UIWebView::Clone()
{
    UIWebView* webView = new UIWebView(GetRect());
    webView->CopyDataFrom(this);
    return webView;
}

void UIWebView::CopyDataFrom(UIControl* srcControl)
{
    UIControl::CopyDataFrom(srcControl);
}

void UIWebView::SystemDraw(const DAVA::UIGeometricData& geometricData)
{
    UIControl::SystemDraw(geometricData);
}

#if defined(__DAVAENGINE_WIN_UAP__)
void UIWebView::Update(float32 timeElapsed)
{
    webViewControl->Update();
    UIControl::Update(timeElapsed);
}
#endif //__DAVAENGINE_WIN_UAP__
};

#endif //__NO_NATIVE_WEBVIEW__

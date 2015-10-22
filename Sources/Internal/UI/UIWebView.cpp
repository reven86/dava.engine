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
#include "FileSystem/YamlNode.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#if defined(__DAVAENGINE_MACOS__)
#include "Platform/TemplateMacOS/WebViewControlMacOS.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "Platform/TemplateiOS/WebViewControliOS.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Platform/TemplateWin32/WebViewControlWin32.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Platform/TemplateWin32/WebViewControlWinUAP.h"
#elif defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/WebViewControlAndroid.h"
#else
#error UIWEbView control is not implemented for this platform yet!
#endif

namespace DAVA {
UIWebView::UIWebView(const Rect& rect)
    : UIControl(rect)
    , webViewControl(0)
    , isNativeControlVisible(false)
{
    webViewControl = new WebViewControl(*this);
    Rect newRect = GetAbsoluteRect();
    webViewControl->Initialize(newRect);
    UpdateControlRect();

    UpdateNativeControlVisible(false); // will be displayed in WillAppear.
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

void UIWebView::OpenFile(const FilePath &path)
{
    // Open files in a browser via a buffer necessary because
    // the reference type file:// is not supported in Windows 10
    // for security reasons
    ScopedPtr<File> file(File::Create(path, File::OPEN | File::READ));
    DVASSERT_MSG(file, "[UIWebView] Failed to open file");
    String data;
    if (file && file->ReadString(data) > 0)
    {
        webViewControl->OpenFromBuffer(data, path.GetDirectory());
    }
    else
    {
        Logger::Error("[UIWebView] Failed to read content from %s", path.GetStringValue().c_str());
    }
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

void UIWebView::ExecuteJScript(const String& scriptString)
{
	webViewControl->ExecuteJScript(scriptString);
}

void UIWebView::OpenFromBuffer(const String& string, const FilePath& basePath)
{
    webViewControl->OpenFromBuffer(string, basePath);
}

void UIWebView::WillBecomeVisible()
{
    UIControl::WillBecomeVisible();
    UpdateNativeControlVisible(true);
}

void UIWebView::WillBecomeInvisible()
{
    UIControl::WillBecomeInvisible();
    UpdateNativeControlVisible(false);
}

void UIWebView::DidAppear()
{
    UIControl::DidAppear();
    UpdateControlRect();
}

void UIWebView::SetPosition(const Vector2 &position)
{
	UIControl::SetPosition(position);
    UpdateControlRect();
}

void UIWebView::SetSize(const Vector2 &newSize)
{
	UIControl::SetSize(newSize);
    UpdateControlRect();
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
    Rect rect = GetAbsoluteRect();

    webViewControl->SetRect(rect);
}

void UIWebView::SetRenderToTexture(bool value)
{
    // for now disable this functionality
    value = false;
    webViewControl->SetRenderToTexture(value);
}

bool UIWebView::IsRenderToTexture() const
{
    return webViewControl->IsRenderToTexture();
}

void UIWebView::SetNativeControlVisible(bool isVisible)
{
    UpdateNativeControlVisible(isVisible);
}

bool UIWebView::GetNativeControlVisible() const
{
    return isNativeControlVisible;
}

void UIWebView::UpdateNativeControlVisible(bool value)
{
    isNativeControlVisible = value;
    webViewControl->SetVisible(value, true);
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
        eDataDetectorType dataDetectorTypes = static_cast<eDataDetectorType>(
            dataDetectorTypesNode->AsInt32());
        SetDataDetectorTypes(dataDetectorTypes);
    }
}

YamlNode* UIWebView::SaveToYamlNode(DAVA::UIYamlLoader *loader)
{
    ScopedPtr<UIWebView> baseControl(new UIWebView());
    YamlNode *node = UIControl::SaveToYamlNode(loader);
    
    // Data Detector Types.
    if (baseControl->GetDataDetectorTypes() != GetDataDetectorTypes())
    {
        node->Set("dataDetectorTypes", GetDataDetectorTypes());
    }
    
    return node;
}

UIWebView* UIWebView::Clone()
{
    UIWebView* webView = new UIWebView(GetRect());
    webView->CopyDataFrom(this);
    return webView;
}

void UIWebView::CopyDataFrom(UIControl *srcControl)
{
    UIControl::CopyDataFrom(srcControl);

    UIWebView* webView = DynamicTypeCheck<UIWebView*>(srcControl);
    SetNativeControlVisible(webView->GetNativeControlVisible());
    SetDataDetectorTypes(webView->GetDataDetectorTypes());
}

void UIWebView::SystemDraw(const DAVA::UIGeometricData &geometricData)
{
    webViewControl->WillDraw();
    UIControl::SystemDraw(geometricData);
    webViewControl->DidDraw();
}


};

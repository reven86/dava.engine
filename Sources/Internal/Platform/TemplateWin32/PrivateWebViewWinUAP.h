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

#ifndef __DAVAENGINE_PRIVATEWEBVIEWWINUAP_H__
#define __DAVAENGINE_PRIVATEWEBVIEWWINUAP_H__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

namespace DAVA
{

class Sprite;
class CorePlatformWinUAP;

class PrivateWebViewWinUAP : public std::enable_shared_from_this<PrivateWebViewWinUAP>
{
public:
    PrivateWebViewWinUAP(UIWebView* UIWebView);
    ~PrivateWebViewWinUAP();

    // WebViewControl should invoke it in its destructor to tell this class instance
    // to fly away on its own (finish pending jobs if any, and delete when all references are lost)
    void OwnerAtPremortem();

    void Initialize(const Rect& rect);

    void OpenURL(const String& urlToOpen);
    void LoadHtmlString(const WideString& htmlString);
    void OpenFromBuffer(const String& string, const FilePath& basePath);
    void ExecuteJScript(const String& scriptString);

    void SetRect(const Rect& rect);
    void SetVisible(bool isVisible);
    void SetBackgroundTransparency(bool enabled);

    void SetDelegate(IUIWebViewDelegate* webViewDelegate);

    void SetRenderToTexture(bool value);
    bool IsRenderToTexture() const;

private:
    void InstallEventHandlers();
    void PositionWebView(const Rect& rectInVirtualCoordinates, bool offScreen);

    void RenderToTexture();
    Sprite* CreateSpriteFromPreviewData(const uint8* imageData, int32 width, int32 height) const;

private:    // WebView event handlers
    void OnNavigationStarting(Windows::UI::Xaml::Controls::WebView^ sender, Windows::UI::Xaml::Controls::WebViewNavigationStartingEventArgs^ args);
    void OnNavigationCompleted(Windows::UI::Xaml::Controls::WebView^ sender, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs^ args);

private:
    CorePlatformWinUAP* core;
    UIWebView* uiWebView = nullptr;
    IUIWebViewDelegate* webViewDelegate = nullptr;
    Windows::UI::Xaml::Controls::WebView^ nativeWebView = nullptr;
    Windows::UI::Color defaultBkgndColor;
    bool visible = true;
    bool renderToTexture = false;
    Rect originalRect;
};

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
#endif  // __DAVAENGINE_PRIVATEWEBVIEWWINUAP_H__

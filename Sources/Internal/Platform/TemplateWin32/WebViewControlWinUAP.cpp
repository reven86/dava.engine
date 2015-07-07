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

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include <ppltasks.h>

#include "Base/RefPtr.h"

#include "Platform/TemplateWin32/WinUAPXamlApp.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/Image/ImageConvert.h"

#include "Utils/UTF8Utils.h"
#include "Utils/Utils.h"

#include "UI/UIWebView.h"
#include "Platform/TemplateWin32/WebViewControlWinUAP.h"

#include "FileSystem/File.h"
#include "FileSystem/Logger.h"

using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace concurrency;

namespace DAVA
{

WebViewControl::WebViewControl(UIWebView& uiWebView_)
    : uiWebView(uiWebView_)
    , core(static_cast<CorePlatformWinUAP*>(Core::Instance()))
    , id(++n)
{
    Logger::Debug("****************************** WebViewControl::WebViewControl: %p", this);
}

WebViewControl::~WebViewControl()
{
    Logger::Debug("****************************** WebViewControl::~WebViewControl: %p", this);
    if (nativeWebView != nullptr)
    {
        core->RunOnUIThreadBlocked([this]() {
            core->XamlApplication()->RemoveUIElement(nativeWebView);
        });
        nativeWebView = nullptr;
    }
}

void WebViewControl::Initialize(const Rect& rect)
{
    originalRect = rect;
    core->RunOnUIThreadBlocked([this]() {
        nativeWebView = ref new WebView;//(WebViewExecutionMode::SeparateThread);
        //nativeWebView->Visibility = Visibility::Collapsed;
        //nativeWebView->CompositeMode = ElementCompositeMode::MinBlend;

        core->XamlApplication()->AddUIElement(nativeWebView);

        InstallEventHandlers();
        PositionWebView(originalRect);
    });
}

void WebViewControl::OpenURL(const String& urlToOpen)
{
    core->RunOnUIThread([this, urlToOpen]() {
        WideString wideUrl = UTF8Utils::EncodeToWideString(urlToOpen);
        Uri^ url = ref new Uri(ref new Platform::String(wideUrl.c_str()));
        nativeWebView->Navigate(url);
    });
}

void WebViewControl::LoadHtmlString(const WideString& htmlString)
{
    core->RunOnUIThread([this, htmlString]() {
        Platform::String^ html = ref new Platform::String(htmlString.c_str());
        nativeWebView->NavigateToString(html);
    });
}

void WebViewControl::ExecuteJScript(const String& scriptString)
{
    /*core->RunOnUIThread([this, scriptString]() {
        Platform::String^ script = ref new Platform::String(UTF8Utils::EncodeToWideString(scriptString).c_str());
        Platform::Array<Platform::String^>^ args = ref new Platform::Array<Platform::String^>(1);
        args[0] = script;
        Windows::Foundation::Collections::IVector<
        //auto args = ref new Platform::Collections::Vector<Platform::String^>();
        //args->Append(script);
        //nativeWebView->InvokeScriptAsync("eval", script);
        nativeWebView->InvokeScriptAsync("eval", args);
    });*/
}

void WebViewControl::SetRect(const Rect& rect)
{
    originalRect = rect;
    core->RunOnUIThread([this]() {
        PositionWebView(originalRect);
    });
}

void WebViewControl::SetVisible(bool isVisible, bool /*hierarchic*/)
{
    core->RunOnUIThread([this, isVisible]() {
        nativeWebView->Visibility = isVisible ? Visibility::Visible : Visibility::Collapsed;
    });
}

void WebViewControl::SetBackgroundTransparency(bool enabled)
{
    core->RunOnUIThread([this, enabled]() {
        //nativeWebView->Opacity = enabled ? 0.5 : 1.0;
        //nativeWebView->CompositeMode = enabled ? ElementCompositeMode::MinBlend : ElementCompositeMode::SourceOver;
        nativeWebView->CompositeMode = enabled ? (ElementCompositeMode)3 : ElementCompositeMode::SourceOver;
    });
}

void WebViewControl::SetDelegate(IUIWebViewDelegate* webViewDelegate_, UIWebView* webView)
{
    if (webViewDelegate_ != nullptr && webView != nullptr)
    {
        webViewDelegate = webViewDelegate_;
    }
}

void WebViewControl::SetRenderToTexture(bool value)
{
    if (renderToTexture != value)
    {
        renderToTexture = value;
        if (!renderToTexture)
        {
            uiWebView.SetSprite(nullptr, 0);
        }

        core->RunOnUIThread([this]() {
            if (renderToTexture)
            {
                PositionWebView(originalRect);
                RenderToTexture();
            }
            else
            {
                PositionWebView(originalRect);
            }
        });
    }
}

void WebViewControl::InstallEventHandlers()
{
    auto navigationStarting = ref new TypedEventHandler<WebView^, WebViewNavigationStartingEventArgs^>([this](WebView^ sender, WebViewNavigationStartingEventArgs^ args) {
        OnNavigationStarting(sender, args);
    });
    auto navigationCompleted = ref new TypedEventHandler<WebView^, WebViewNavigationCompletedEventArgs^>([this](WebView^ sender, WebViewNavigationCompletedEventArgs^ args) {
        OnNavigationCompleted(sender, args);
    });
    nativeWebView->NavigationStarting += navigationStarting;
    nativeWebView->NavigationCompleted += navigationCompleted;
}

void WebViewControl::PositionWebView(const Rect& rect)
{
    VirtualCoordinatesSystem* coordSys = VirtualCoordinatesSystem::Instance();

    Rect physRect = coordSys->ConvertVirtualToPhysical(rect);
    const Vector2 physOffset = coordSys->GetPhysicalDrawOffset();

    float32 width = physRect.dx + physOffset.x;
    float32 height = physRect.dy + physOffset.y;

    // When rendering to texture move WebView off the screen
    // as only visible WebView can produce its image
    if (renderToTexture)
    {
        physRect.x = -width;
        physRect.y = -height;
    }
    nativeWebView->Width = width;
    nativeWebView->Height = height;
    core->XamlApplication()->PositionUIElement(nativeWebView, physRect.x, physRect.y);
}

void WebViewControl::OnNavigationStarting(Windows::UI::Xaml::Controls::WebView^ sender, Windows::UI::Xaml::Controls::WebViewNavigationStartingEventArgs^ args)
{
    String url;
    if (args->Uri != nullptr)
    {
        Platform::String^ canonicalUrl = args->Uri->AbsoluteCanonicalUri;
        url = UTF8Utils::EncodeToUTF8(WideString(canonicalUrl->Data()));
    }
    Logger::Debug("********* OnNavigationStarting: url=%s", url.c_str());

    if (webViewDelegate != nullptr)
    {
        webViewDelegate->URLChanged(&uiWebView, url, true);
    }
}

void WebViewControl::OnNavigationCompleted(Windows::UI::Xaml::Controls::WebView^ sender, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs^ args)
{
    bool success = args->IsSuccess;
    int status = (int)args->WebErrorStatus;
    Uri^ uri = args->Uri;
    String x;
    if (uri != nullptr)
    {
        Platform::String^ s = uri->AbsoluteCanonicalUri;
        x = WStringToString(WideString(s->Data()));
    }

    Logger::Debug("*** OnNavigationCompleted: success=%s, status=%d, url=%s", success ? "ok" : "fail", status, x.c_str());

    if (renderToTexture)
    {
        RenderToTexture();
    }

    if (webViewDelegate != nullptr)
    {
        webViewDelegate->PageLoaded(&uiWebView);
    }
}

void WebViewControl::RenderToTexture()
{
    int32 width = static_cast<int32>(nativeWebView->Width);
    int32 height = static_cast<int32>(nativeWebView->Height);

    InMemoryRandomAccessStream^ inMemoryStream = ref new InMemoryRandomAccessStream();
    auto taskCapture = create_task(nativeWebView->CapturePreviewToStreamAsync(inMemoryStream));
    taskCapture.then([this, inMemoryStream, width, height]()
    {
        size_t streamSize = static_cast<size_t>(inMemoryStream->Size);
        DataReader^ reader = ref new DataReader(inMemoryStream->GetInputStreamAt(0));
        auto taskLoad = create_task(reader->LoadAsync(streamSize));
        taskLoad.then([this, reader, width, height, streamSize](task<unsigned int>)
        {
            std::vector<uint8> buf(streamSize, 0);
            size_t index = 0;
            while (reader->UnconsumedBufferLength > 0)
            {
                buf[index] = reader->ReadByte();
                index += 1;
            }
            SavePreviewToFile(buf.data(), buf.size());

            Sprite* sprite = CreateSpriteFromPreviewData(buf.data(), width, height);
            core->RunOnMainThread([this, sprite]() {
                uiWebView.SetSprite(sprite, 0);
            });
        });
    });
}

Sprite* WebViewControl::CreateSpriteFromPreviewData(const uint8* imageData, int32 width, int32 height) const
{
    /*struct BITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
    };*/
    DWORD bitsOffset = *OffsetPointer<DWORD>(imageData, 10);

    /*{ // manual swap B and R components
        int n = width * height * 4;
        uint8* p = const_cast<uint8*>(imageData + bitsOffset);
        for (int i = 0;i < n;i += 4)
        {
            std::swap(p[0], p[2]);
        }
    }*/

    RefPtr<Image> imgSrc(Image::CreateFromData(width, height, FORMAT_RGBA8888, imageData + bitsOffset));
    RefPtr<Image> imgDst(Image::Create(width, height, FORMAT_RGB888));
    if (imgSrc.Valid() && imgDst.Valid())
    {
        ImageConvert::ConvertImageDirect(imgSrc.Get(), imgDst.Get());
        return Sprite::CreateFromImage(imgDst.Get());
    }
    return nullptr;
}

int WebViewControl::n = 0;

void WebViewControl::SavePreviewToFile(const uint8* imageData, size_t size) const
{
    StorageFolder^ folder = ApplicationData::Current->LocalFolder;
    String path = WStringToString(WideString(folder->Path->Data()));

    path += Format("\\image_%d_%d.bmp", id, ++inc);

    RefPtr<File> file(File::Create(FilePath(path), File::CREATE | File::WRITE));
    if (file.Valid())
    {
        file->Write(imageData, size);
    }
}

}   // namespace DAVA

#endif // defined(__DAVAENGINE_WIN_UAP__)

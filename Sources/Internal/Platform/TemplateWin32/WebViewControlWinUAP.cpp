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

#include "Platform/TemplateWin32/WinUAPXamlApp.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/Image/ImageConvert.h"

#include "Utils/UTF8Utils.h"
#include "Utils/Utils.h"

#include "Platform/TemplateWin32/WebViewControlWinUAP.h"

#include "FileSystem/File.h"
#include "FileSystem/Logger.h"

using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

using namespace Windows::Storage::Streams;

namespace DAVA
{
/*
ref class MyStream : public IRandomAccessStream
{
public:
    IRandomAccessStream^ CloneStream() override;
    IInputStream^ GetInputStreamAt(unsigned long long position) override;
    IOutputStream^ GetOutputStreamAt(unsigned long long position) override;
    void Seek(unsigned long long position) override;

    property bool CanRead;
};
*/
WebViewControl::WebViewControl(UIWebView& uiWebView_)
    : uiWebView(uiWebView_)
    , core(static_cast<CorePlatformWinUAP*>(Core::Instance()))
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
    core->RunOnUIThreadBlocked([this, &rect]() {
        nativeWebView = ref new WebView(WebViewExecutionMode::SeparateThread);
        //nativeWebView->Visibility = Visibility::Collapsed;

        auto x1 = ref new TypedEventHandler<WebView^, WebViewNavigationStartingEventArgs^>([this](WebView^ sender, WebViewNavigationStartingEventArgs^ args) {
            OnNavigationStarting(sender, args);
        });
        auto x2 = ref new TypedEventHandler<WebView^, WebViewNavigationCompletedEventArgs^>([this](WebView^ sender, WebViewNavigationCompletedEventArgs^ args) {
            OnNavigationCompleted(sender, args);
        });
        nativeWebView->NavigationStarting += x1;
        nativeWebView->NavigationCompleted += x2;

        core->XamlApplication()->AddUIElement(nativeWebView);
        PositionWebView(rect);
    });
}

void WebViewControl::OpenURL(const String& urlToOpen)
{
    WideString wideUrl = UTF8Utils::EncodeToWideString(urlToOpen);
    curUrl = ref new Uri(ref new Platform::String(wideUrl.c_str()));

    core->RunOnUIThread([this]() {
        nativeWebView->Navigate(curUrl);
    });
}

void WebViewControl::LoadHtmlString(const WideString& htmlString)
{
    curUrl = nullptr;
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
    core->RunOnUIThread([this, rect]() {
        PositionWebView(rect);
    });
}

void WebViewControl::SetVisible(bool isVisible, bool /*hierarchic*/)
{
    core->RunOnUIThread([this, isVisible]() {
        nativeWebView->Visibility = isVisible ? Visibility::Visible : Visibility::Collapsed;
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
    renderToTexture = value;
}

void WebViewControl::PositionWebView(const Rect& rect)
{
    VirtualCoordinatesSystem* coordSys = VirtualCoordinatesSystem::Instance();

    Rect physRect = coordSys->ConvertVirtualToPhysical(rect);
    const Vector2 physOffset = coordSys->GetPhysicalDrawOffset();

    nativeWebView->Width = physRect.dx + physOffset.x;
    nativeWebView->Height = physRect.dy + physOffset.y;
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

    RenderToTexture();

    //if (webViewDelegate != nullptr)
    //{
    //    webViewDelegate->PageLoaded(&uiWebView);
    //}
}

void WebViewControl::RenderToTexture()
{
    using namespace concurrency;
    using namespace Windows::UI::Xaml::Media::Imaging;
    using namespace Windows::Storage::Streams;
    using namespace Windows::Storage;

    int w = (int)nativeWebView->Width;
    int h = (int)nativeWebView->Height;

    InMemoryRandomAccessStream^ ms = ref new InMemoryRandomAccessStream();
    auto asyncCapture = nativeWebView->CapturePreviewToStreamAsync(ms);
    auto taskCapture = create_task(asyncCapture);
    taskCapture.then([this, ms, w, h]()
    {
        unsigned int streamSize = (unsigned int)ms->Size;
        DataReader^ reader = ref new DataReader(ms->GetInputStreamAt(0));

        auto asyncLoad = reader->LoadAsync(streamSize);
        auto taskLoad = create_task(asyncLoad);
        taskLoad.then([this, reader, streamSize](task<unsigned int> x)
        {
            std::vector<uint8> v(streamSize, 0);
            size_t index = 0;
            unsigned int unread = (unsigned int)reader->UnconsumedBufferLength;
            while (unread > 0)
            {
                v[index] = reader->ReadByte();
                index += 1;
                unread = (unsigned int)reader->UnconsumedBufferLength;
            }

            StorageFolder^ folder = ApplicationData::Current->LocalFolder;
            Platform::String^ wname = folder->Name;
            Platform::String^ wpath = folder->Path;
            String name = WStringToString(WideString(wname->Data()));
            String path = WStringToString(WideString(wpath->Data()));

            path += Format("\\image_%p_%d.bmp", this, ++n);

            //wchar_t curdir[500];
            //GetCurrentDirectoryW(500, curdir);
            //wcscat(curdir, L"\\image.bmp");
            //DWORD nw = 0;
            ////HANDLE hfile = CreateFile2(curdir, GENERIC_WRITE, 0, CREATE_ALWAYS, nullptr);
            //HANDLE hfile = CreateFile2(L"image", GENERIC_WRITE, 0, CREATE_ALWAYS, nullptr);
            //if (hfile != INVALID_HANDLE_VALUE)
            //{
            //    WriteFile(hfile, v.data(), v.size(), &nw, nullptr);
            //    CloseHandle(hfile);
            //}
            FILE* file = fopen(path.c_str(), "wb");
            //FILE* file = fopen(curdir, "rb");
            //File* file = File::Create(FilePath("d:\\image.bmp"), File::CREATE | File::WRITE);
            //File* file = File::PureCreate(FilePath("image.bmp"), File::CREATE | File::WRITE);
            size_t nwritten = 0;
            if (file)
            {
                nwritten = fwrite(v.data(), 1, v.size(), file);
                fclose(file);
                //file->Write(v.data(), v.size());
                //file->Release();
            }
            int h = 0;
        });
    });
}

}   // namespace DAVA

#endif // defined(__DAVAENGINE_WIN_UAP__)

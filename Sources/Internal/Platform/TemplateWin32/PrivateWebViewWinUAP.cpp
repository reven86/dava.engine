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
#include <collection.h>

#include "Base/RefPtr.h"
#include "Debug/DVAssert.h"

#include "Platform/TemplateWin32/WinUAPXamlApp.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/Image/ImageConvert.h"

#include "Utils/UTF8Utils.h"

#include "UI/UIWebView.h"
#include "Platform/TemplateWin32/PrivateWebViewWinUAP.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "FileSystem/Logger.h"

using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Web;
using namespace concurrency;

namespace DAVA
{
// clang-format off
/*
    UriResolver is intended for mapping resource paths in html source to local resource files

    UriResolver is used in PrivateWebViewWinUAP's OpenFromBuffer method which allows to load
    HTML string specifying location of resource files (css, images, etc).
*/
private ref class UriResolver sealed : public IUriToStreamResolver
{
internal:
    UriResolver(const String& htmlData, const FilePath& basePath);

public:
    virtual IAsyncOperation<IInputStream^>^ UriToStreamAsync(Uri^ uri);

private:
    IAsyncOperation<IInputStream ^> ^ GetStreamFromFilePathAsync(const FilePath& filePath);
    IAsyncOperation<IInputStream^>^ GetStreamFromStringAsync(Platform::String^ s);

    Platform::String^ htmlData;
    FilePath basePath;
};

UriResolver::UriResolver(const String& htmlData_, const FilePath& basePath_)
    : htmlData(ref new Platform::String(UTF8Utils::EncodeToWideString(htmlData_).c_str()))
    , basePath(basePath_)
{
}

IAsyncOperation<IInputStream^>^ UriResolver::UriToStreamAsync(Uri^ uri)
{
    DVASSERT(uri != nullptr);

    Platform::String^ dummy = uri->Path;
    if (0 == Platform::String::CompareOrdinal(uri->Path, L"/johny23"))
    {   // Create stream from HTML data
        return GetStreamFromStringAsync(htmlData);
    }

    FilePath path = basePath + RTStringToString(uri->Path);
    return GetStreamFromFilePathAsync(path);
}

IAsyncOperation<IInputStream ^> ^ UriResolver::GetStreamFromFilePathAsync(const FilePath& filePath)
{
    String fileNameStr = filePath.GetAbsolutePathname();
    std::replace(fileNameStr.begin(), fileNameStr.end(), '/', '\\');
    Platform::String^ fileName = StringToRTString(fileNameStr);

    return create_async([ this, fileName ]() -> IInputStream^
                        {
                            try
                            {
                                StorageFile^ storageFile = WaitAsync(StorageFile::GetFileFromPathAsync(fileName));
                                IRandomAccessStream^ stream = WaitAsync(storageFile->OpenAsync(FileAccessMode::Read));
                                return static_cast<IInputStream^>(stream);
                            }
                            catch (Platform::COMException^ e)
                            {
                                Logger::Error("[MovieView] failed to load file %s: %s (0x%08x)",
                                              RTStringToString(fileName).c_str(),
                                              RTStringToString(e->Message).c_str(),
                                              e->HResult);
                                return ref new InMemoryRandomAccessStream();
                            }
                        });
}

IAsyncOperation<IInputStream^>^ UriResolver::GetStreamFromStringAsync(Platform::String^ s)
{
    InMemoryRandomAccessStream^ stream = ref new InMemoryRandomAccessStream();
    DataWriter^ writer = ref new DataWriter(stream->GetOutputStreamAt(0));

    writer->WriteString(s);
    WaitAsync(writer->StoreAsync());

    return create_async([stream]() -> IInputStream^ { return stream; });
}
// clang-format on
//////////////////////////////////////////////////////////////////////////

PrivateWebViewWinUAP::WebViewProperties::WebViewProperties()
    : createNew(false)
    , anyPropertyChanged(false)
    , rectChanged(false)
    , visibleChanged(false)
    , renderToTextureChanged(false)
    , backgroundTransparencyChanged(false)
    , execJavaScript(false)
    , navigateTo(WebViewProperties::NAVIGATE_NONE)
{
}

void PrivateWebViewWinUAP::WebViewProperties::ClearChangedFlags()
{
    createNew = false;
    anyPropertyChanged = false;
    rectChanged = false;
    visibleChanged = false;
    renderToTextureChanged = false;
    backgroundTransparencyChanged = false;
    execJavaScript = false;
    navigateTo = WebViewProperties::NAVIGATE_NONE;
}

PrivateWebViewWinUAP::PrivateWebViewWinUAP(UIWebView* uiWebView_)
    : uiWebView(uiWebView_)
    , core(static_cast<CorePlatformWinUAP*>(Core::Instance()))
{
}

PrivateWebViewWinUAP::~PrivateWebViewWinUAP()
{
    if (nativeWebView != nullptr)
    {
        // Compiler complains of capturing nativeWebView data member in lambda
        WebView ^ p = nativeWebView;
        core->RunOnUIThread([p]() { // We don't need blocking call here
            static_cast<CorePlatformWinUAP*>(Core::Instance())->XamlApplication()->RemoveUIElement(p);
        });
        nativeWebView = nullptr;
    }
}

void PrivateWebViewWinUAP::OwnerAtPremortem()
{
    uiWebView = nullptr;
    webViewDelegate = nullptr;
}

void PrivateWebViewWinUAP::Initialize(const Rect& rect)
{
    properties.createNew = true;

    properties.rect = rect;
    properties.rectInWindowSpace = VirtualToWindow(rect);
    properties.rectChanged = true;
    properties.anyPropertyChanged = true;
}

void PrivateWebViewWinUAP::OpenURL(const String& urlToOpen)
{
    properties.urlOrHtml = urlToOpen;
    properties.navigateTo = WebViewProperties::NAVIGATE_OPEN_URL;
    properties.anyPropertyChanged = true;
}

void PrivateWebViewWinUAP::LoadHtmlString(const WideString& htmlString)
{
    properties.htmlString = htmlString;
    properties.navigateTo = WebViewProperties::NAVIGATE_LOAD_HTML;
    properties.anyPropertyChanged = true;
}

void PrivateWebViewWinUAP::OpenFromBuffer(const String& htmlString, const FilePath& basePath)
{
    properties.urlOrHtml = htmlString;
    properties.basePath = basePath;
    properties.navigateTo = WebViewProperties::NAVIGATE_OPEN_BUFFER;
    properties.anyPropertyChanged = true;
}

void PrivateWebViewWinUAP::ExecuteJScript(const String& scriptString)
{
    properties.jsScript = scriptString;
    properties.execJavaScript = true;
    properties.anyPropertyChanged = true;
}

void PrivateWebViewWinUAP::SetRect(const Rect& rect)
{
    if (properties.rect != rect)
    {
        properties.rect = rect;
        properties.rectInWindowSpace = VirtualToWindow(rect);
        properties.rectChanged = true;
        properties.anyPropertyChanged = true;
    }
}

void PrivateWebViewWinUAP::SetVisible(bool isVisible)
{
    if (properties.visible != isVisible)
    {
        properties.visible = isVisible;
        properties.visibleChanged = true;
        properties.anyPropertyChanged = true;
        if (!isVisible)
        { // Immediately hide native control if it has been already created
            core->RunOnUIThread([this]() {
                if (nativeWebView != nullptr)
                {
                    SetNativePositionAndSize(rectInWindowSpace, true);
                }
            });
        }
    }
}

void PrivateWebViewWinUAP::SetBackgroundTransparency(bool enabled)
{
    if (properties.backgroundTransparency != enabled)
    {
        properties.backgroundTransparency = enabled;
        properties.backgroundTransparencyChanged = true;
        properties.anyPropertyChanged = true;
    }
}

void PrivateWebViewWinUAP::SetDelegate(IUIWebViewDelegate* webViewDelegate_)
{
    webViewDelegate = webViewDelegate_;
}

void PrivateWebViewWinUAP::SetRenderToTexture(bool value)
{
    if (properties.renderToTexture != value)
    {
        properties.renderToTexture = value;
        properties.renderToTextureChanged = true;
        properties.anyPropertyChanged = true;
        if (!value)
        { // Immediately hide native control if it has been already created
            core->RunOnUIThread([this]() {
                if (nativeWebView != nullptr)
                {
                    SetNativePositionAndSize(rectInWindowSpace, true);
                }
            });
        }
    }
}

bool PrivateWebViewWinUAP::IsRenderToTexture() const
{
    return properties.renderToTexture;
}

void PrivateWebViewWinUAP::Update()
{
    if (properties.createNew || properties.anyPropertyChanged)
    {
        auto self{ shared_from_this() };
        WebViewProperties props(properties);
        core->RunOnUIThread([this, self, props] {
            ProcessProperties(props);
        });

        properties.createNew = false;
        properties.ClearChangedFlags();
    }
}

void PrivateWebViewWinUAP::CreateNativeControl()
{
    nativeWebView = ref new WebView();
    defaultBkgndColor = nativeWebView->DefaultBackgroundColor;
    InstallEventHandlers();

    nativeWebView->MinWidth = 0.0;
    nativeWebView->MinHeight = 0.0;
    nativeWebView->Visibility = Visibility::Visible;

    core->XamlApplication()->AddUIElement(nativeWebView);
    SetNativePositionAndSize(rectInWindowSpace, true); // After creation move native control offscreen
}

void PrivateWebViewWinUAP::InstallEventHandlers()
{
    // clang-format off
    std::weak_ptr<PrivateWebViewWinUAP> self_weak(shared_from_this());
    // Install event handlers through lambdas as it seems only ref class's member functions can be event handlers directly
    auto navigationStarting = ref new TypedEventHandler<WebView^, WebViewNavigationStartingEventArgs^>([this, self_weak](WebView^ sender, WebViewNavigationStartingEventArgs^ args) {
        if (auto self = self_weak.lock())
            OnNavigationStarting(sender, args);
    });
    auto navigationCompleted = ref new TypedEventHandler<WebView^, WebViewNavigationCompletedEventArgs^>([this, self_weak](WebView^ sender, WebViewNavigationCompletedEventArgs^ args) {
        if (auto self = self_weak.lock())
            OnNavigationCompleted(sender, args);
    });
    nativeWebView->NavigationStarting += navigationStarting;
    nativeWebView->NavigationCompleted += navigationCompleted;
    // clang-format on
}

void PrivateWebViewWinUAP::OnNavigationStarting(WebView ^ sender, WebViewNavigationStartingEventArgs ^ args)
{
    String url;
    if (args->Uri != nullptr)
    {
        url = WStringToString(args->Uri->AbsoluteCanonicalUri->Data());
    }
    Logger::FrameworkDebug("[WebView] OnNavigationStarting: url=%s", url.c_str());

    bool redirectedByMouse = false; // For now I don't know how to get redirection method
    IUIWebViewDelegate::eAction whatToDo = IUIWebViewDelegate::PROCESS_IN_WEBVIEW;
    core->RunOnMainThreadBlocked([this, &whatToDo, &url, redirectedByMouse]()
                                 {
                                     if (uiWebView != nullptr && webViewDelegate != nullptr)
                                     {
                                         whatToDo = webViewDelegate->URLChanged(uiWebView, url, redirectedByMouse);
                                     }
                                 });

    if (IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER == whatToDo && args->Uri != nullptr)
    {
        Launcher::LaunchUriAsync(args->Uri);
    }
    args->Cancel = whatToDo != IUIWebViewDelegate::PROCESS_IN_WEBVIEW;
}

void PrivateWebViewWinUAP::OnNavigationCompleted(WebView ^ sender, WebViewNavigationCompletedEventArgs ^ args)
{
    String url;
    if (args->Uri != nullptr)
    {
        url = WStringToString(args->Uri->AbsoluteCanonicalUri->Data());
    }

    if (args->IsSuccess)
    {
        Logger::FrameworkDebug("[WebView] OnNavigationCompleted: url=%s", url.c_str());
    }
    else
    {
        Logger::Error("[WebView] OnNavigationCompleted failed: err_status=%d, url=%s", static_cast<int>(args->WebErrorStatus), url.c_str());
    }

    if (renderToTexture)
    {
        RenderToTexture();
    }

    auto self{ shared_from_this() };
    core->RunOnMainThread([this, self]() {
        if (uiWebView != nullptr && webViewDelegate != nullptr)
        {
            webViewDelegate->PageLoaded(uiWebView);
        }
    });
}

void PrivateWebViewWinUAP::ProcessProperties(const WebViewProperties& props)
{
    rectInWindowSpace = props.rectInWindowSpace;
    if (props.createNew)
    {
        CreateNativeControl();
    }
    if (props.anyPropertyChanged)
    {
        ApplyChangedProperties(props);
        if (renderToTexture && (props.rectChanged || props.backgroundTransparencyChanged || props.renderToTextureChanged))
        {
            RenderToTexture();
        }
    }
}

void PrivateWebViewWinUAP::ApplyChangedProperties(const WebViewProperties& props)
{
    if (props.renderToTextureChanged)
        renderToTexture = props.renderToTexture;
    if (props.visibleChanged)
        visible = props.visible;

    if (props.rectChanged || props.visibleChanged || props.renderToTextureChanged)
        SetNativePositionAndSize(props.rectInWindowSpace, renderToTexture || !visible);
    if (props.backgroundTransparencyChanged)
        SetNativeBackgroundTransparency(props.backgroundTransparency);
    if (props.navigateTo != WebViewProperties::NAVIGATE_NONE)
        NativeNavigateTo(props);
    if (props.execJavaScript)
        NativeExecuteJavaScript(props.jsScript);
}

void PrivateWebViewWinUAP::SetNativePositionAndSize(const Rect& rect, bool offScreen)
{
    float32 xOffset = 0.0f;
    float32 yOffset = 0.0f;
    if (offScreen)
    {
        // Move control very far offscreen as on phone control with disabled input remains visible
        xOffset = rect.x + rect.dx + 1000.0f;
        yOffset = rect.y + rect.dy + 1000.0f;
    }
    nativeWebView->Width = rect.dx;
    nativeWebView->Height = rect.dy;
    core->XamlApplication()->PositionUIElement(nativeWebView, rect.x - xOffset, rect.y - yOffset);
}

void PrivateWebViewWinUAP::SetNativeBackgroundTransparency(bool enabled)
{
    nativeWebView->DefaultBackgroundColor = enabled ? Colors::Transparent : defaultBkgndColor;
}

void PrivateWebViewWinUAP::NativeNavigateTo(const WebViewProperties& props)
{
    // clang-format off
    if (WebViewProperties::NAVIGATE_OPEN_URL == props.navigateTo)
    {
        Uri^ uri = ref new Uri(ref new Platform::String(StringToWString(props.urlOrHtml).c_str()));
        nativeWebView->Navigate(uri);
    }
    else if (WebViewProperties::NAVIGATE_LOAD_HTML == props.navigateTo)
    {
        Platform::String^ html = ref new Platform::String(props.htmlString.c_str());
        nativeWebView->NavigateToString(html);
    }
    else if (WebViewProperties::NAVIGATE_OPEN_BUFFER == props.navigateTo)
    {
        UriResolver^ resolver = ref new UriResolver(props.urlOrHtml, props.basePath);
        Uri^ uri = nativeWebView->BuildLocalStreamUri("DAVA", "/johny23");
        nativeWebView->NavigateToLocalStreamUri(uri, resolver);
    }
    // clang-format on
}

void PrivateWebViewWinUAP::NativeExecuteJavaScript(const String& jsScript)
{
    // clang-format off
    Platform::String^ script = ref new Platform::String(StringToWString(jsScript).c_str());

    auto args = ref new Platform::Collections::Vector<Platform::String^>();
    args->Append(script);

    auto js = nativeWebView->InvokeScriptAsync(L"eval", args);
    auto self{shared_from_this()};
    create_task(js).then([this, self](Platform::String^ result) {
        core->RunOnMainThread([this, result]() {
            if (webViewDelegate != nullptr && uiWebView != nullptr)
            {
                String jsResult = WStringToString(result->Data());
                webViewDelegate->OnExecuteJScript(uiWebView, jsResult);
            }
        });
    }).then([self](task<void> t) {
        try {
            t.get();
        } catch (Platform::Exception^ e) {
            // Exception can be thrown if a webpage has not been loaded into the WebView
            HRESULT hr = e->HResult;
            Logger::Error("[WebView] failed to execute JS: hresult=0x%08X, message=%s", hr, WStringToString(e->Message->Data()).c_str());
        }
    });
    // clang-format on
}

Rect PrivateWebViewWinUAP::VirtualToWindow(const Rect& srcRect) const
{
    VirtualCoordinatesSystem* coordSystem = VirtualCoordinatesSystem::Instance();

    // 1. map virtual to physical
    Rect rect = coordSystem->ConvertVirtualToPhysical(srcRect);
    rect += coordSystem->GetPhysicalDrawOffset();

    // 2. map physical to window
    const float32 scaleFactor = core->GetScreenScaleFactor();
    rect.x /= scaleFactor;
    rect.y /= scaleFactor;
    rect.dx /= scaleFactor;
    rect.dy /= scaleFactor;
    return rect;
}

void PrivateWebViewWinUAP::RenderToTexture()
{
    // clang-format off
    int32 width = static_cast<int32>(nativeWebView->Width);
    int32 height = static_cast<int32>(nativeWebView->Height);

    auto self{shared_from_this()};
    InMemoryRandomAccessStream^ inMemoryStream = ref new InMemoryRandomAccessStream();
    auto taskCapture = create_task(nativeWebView->CapturePreviewToStreamAsync(inMemoryStream));
    taskCapture.then([this, self, inMemoryStream, width, height]() {
        unsigned int streamSize = static_cast<unsigned int>(inMemoryStream->Size);
        DataReader^ reader = ref new DataReader(inMemoryStream->GetInputStreamAt(0));
        auto taskLoad = create_task(reader->LoadAsync(streamSize));
        taskLoad.then([this, self, reader, width, height, streamSize](task<unsigned int>) {
            size_t index = 0;
            std::vector<uint8> buf(streamSize, 0);
            while (reader->UnconsumedBufferLength > 0)
            {
                buf[index] = reader->ReadByte();
                index += 1;
            }

            RefPtr<Sprite> sprite(CreateSpriteFromPreviewData(&buf[0], width, height));
            if (sprite.Valid())
            {
                core->RunOnMainThread([this, self, sprite]()
                {
                    if (uiWebView != nullptr)
                    {
                        uiWebView->SetSprite(sprite.Get(), 0);
                    }
                });
            }
        });
    }).then([this, self](task<void> t) {
        try {
            t.get();
        } catch (Platform::COMException^ e) {
            HRESULT hr = e->HResult;
            Logger::Error("[WebView] RenderToTexture failed: 0x%08X", hr);
        }
    });
    // clang-format on
}

Sprite* PrivateWebViewWinUAP::CreateSpriteFromPreviewData(uint8* imageData, int32 width, int32 height) const
{
    /*
        imageData is in-memory BMP file and starts with BITMAPFILEHEADER struct
        In WinRT application this struct is invisible for compiler

        struct BITMAPFILEHEADER {
            WORD    bfType;
            DWORD   bfSize;
            WORD    bfReserved1;
            WORD    bfReserved2;
            DWORD   bfOffBits;      // offset +10
        };
    */
    DWORD bitsOffset = *OffsetPointer<DWORD>(imageData, 10);
    uint8* dataPtr = imageData + bitsOffset;

    ScopedPtr<Image> imgSrc(Image::CreateFromData(width, height, FORMAT_RGBA8888, dataPtr));
    ImageConvert::SwapRedBlueChannels(imgSrc);
    return Sprite::CreateFromImage(imgSrc, true, false);
}

} // namespace DAVA

#endif // defined(__DAVAENGINE_WIN_UAP__)

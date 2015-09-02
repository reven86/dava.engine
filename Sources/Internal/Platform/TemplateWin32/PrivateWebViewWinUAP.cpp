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

#include "Utils/Utils.h"

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

/*
    UriResolver is intended for mapping resource paths in html source to local resource files

    UriResolver is used in PrivateWebViewWinUAP's OpenFromBuffer method which allows to load
    HTML string specifying location of resource files (css, images, etc).
*/
private ref class UriResolver sealed : public IUriToStreamResolver
{
    enum eResLocation
    {
        PATH_IN_BIN,        // Resources are relative to app install dir
        PATH_IN_DOC,        // Resources are relative to user documents dir
        PATH_UNKNOWN        // Resource location is unknown
    };

internal:
    UriResolver(const String& htmlData, const FilePath& basePath);

public:
    virtual IAsyncOperation<IInputStream^>^ UriToStreamAsync(Uri^ uri);

private:
    IAsyncOperation<IInputStream^>^ GetStreamFromUriAsync(Uri^ uri);
    IAsyncOperation<IInputStream^>^ GetStreamFromStringAsync(Platform::String^ s);

    void DetermineResourcesLocation(const FilePath& basePath);
    bool CheckIfPathReachableFrom(const String& pathToCheck, const String& pathToReach, String& pathTail) const;

private:
    Platform::String^ htmlData;
    Platform::String^ relativeResPath;
    eResLocation location;
};

UriResolver::UriResolver(const String& htmlData_, const FilePath& basePath)
    : htmlData(ref new Platform::String(StringToWString(htmlData_).c_str()))
{
    // WinUAP app have access only to isolated storage which includes:
    //  - executable directory
    //  - local app data store
    //  - roaming app data store
    // Absolute paths are not allowed, so DetermineResourcesLocation method determines
    // to which type of storage basePath belongs to
    DetermineResourcesLocation(basePath);
}

IAsyncOperation<IInputStream^>^ UriResolver::UriToStreamAsync(Uri^ uri)
{
    DVASSERT(uri != nullptr);

    Platform::String^ dummy = uri->Path;
    if (0 == Platform::String::CompareOrdinal(uri->Path, L"/johny23"))
    {   // Create stream from HTML data
        return GetStreamFromStringAsync(htmlData);
    }
    else
    {   // Create stream for resource files
        Platform::String^ resPath = relativeResPath + uri->Path;
        Uri^ appDataUri = nullptr;
        switch (location)
        {
        case PATH_IN_BIN:
            appDataUri = ref new Uri(L"ms-appx:///" + resPath);
            break;
        case PATH_IN_DOC:
            appDataUri = ref new Uri(L"ms-appdata:///local/" + resPath);
            break;
        default:
            return nullptr;
        }
        return GetStreamFromUriAsync(appDataUri);
    }
}

IAsyncOperation<IInputStream^>^ UriResolver::GetStreamFromUriAsync(Uri^ uri)
{
    return create_async([this, uri]() -> IInputStream^
    {
        task<StorageFile^> getFileTask(StorageFile::GetFileFromApplicationUriAsync(uri));
        task<IInputStream^> getInputStreamTask = getFileTask.then([](StorageFile^ storageFile)
        {
            return storageFile->OpenAsync(FileAccessMode::Read);
        }).then([](IRandomAccessStream^ stream)
        {
            return static_cast<IInputStream^>(stream);
        });

        try {
            return getInputStreamTask.get();
        } catch (Platform::COMException^ e) {
            // Ignore errors when resource file is not found or access is denied
            HRESULT hr = e->HResult;
            if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr || HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hr)
            {
                Logger::Error("[WebView] failed to load URI='%s': %s", WStringToString(uri->Path->Data()).c_str(),
                              HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr ? "file not found" : "access denied");
                return ref new InMemoryRandomAccessStream();
            }
            throw;  // Rethrow other exceptions
        }
    });
}

IAsyncOperation<IInputStream^>^ UriResolver::GetStreamFromStringAsync(Platform::String^ s)
{
    InMemoryRandomAccessStream^ stream = ref new InMemoryRandomAccessStream();
    DataWriter^ writer = ref new DataWriter(stream->GetOutputStreamAt(0));

    writer->WriteString(s);
    create_task(writer->StoreAsync()).get();

    return create_async([stream]() -> IInputStream^ { return stream; });
}

void UriResolver::DetermineResourcesLocation(const FilePath& basePath)
{
    FileSystem* fs = FileSystem::Instance();

    String pathTail;
    String absPath = basePath.GetAbsolutePathname();
    if (CheckIfPathReachableFrom(absPath, fs->GetCurrentExecutableDirectory().GetAbsolutePathname(), pathTail))
    {
        location = PATH_IN_BIN;
    }
    else if (CheckIfPathReachableFrom(absPath, fs->GetUserDocumentsPath().GetAbsolutePathname(), pathTail))
    {
        location = PATH_IN_DOC;
    }
    else
    {
        location = PATH_UNKNOWN;
    }
    relativeResPath = ref new Platform::String(StringToWString(pathTail).c_str());
}

bool UriResolver::CheckIfPathReachableFrom(const String& pathToCheck, const String& pathToReach, String& pathTail) const
{
    if (pathToCheck.length() >= pathToReach.length())
    {
        if (0 == pathToCheck.compare(0, pathToReach.length(), pathToReach))
        {
            pathTail = pathToCheck.substr(pathToReach.length());
            if (!pathTail.empty() && '/' == pathTail.back())
            {
                pathTail.pop_back();
            }
            return true;
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

PrivateWebViewWinUAP::PrivateWebViewWinUAP(UIWebView* uiWebView_)
    : uiWebView(uiWebView_)
    , core(static_cast<CorePlatformWinUAP*>(Core::Instance()))
{}

PrivateWebViewWinUAP::~PrivateWebViewWinUAP()
{
    uiWebView = nullptr;
    webViewDelegate = nullptr;
    if (nativeWebView != nullptr)
    {
        // Compiler complains of capturing nativeWebView data member in lambda
        WebView^ p = nativeWebView;
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
    originalRect = rect;
    core->RunOnUIThreadBlocked([this]() {
        nativeWebView = ref new WebView();
        defaultBkgndColor = nativeWebView->DefaultBackgroundColor;

        core->XamlApplication()->AddUIElement(nativeWebView);

        InstallEventHandlers();
        PositionWebView(originalRect, false);
    });
}

void PrivateWebViewWinUAP::OpenURL(const String& urlToOpen)
{
    Uri^ url = ref new Uri(ref new Platform::String(StringToWString(urlToOpen).c_str()));
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, url]() {
        nativeWebView->Navigate(url);
    });
}

void PrivateWebViewWinUAP::LoadHtmlString(const WideString& htmlString)
{
    Platform::String^ html = ref new Platform::String(htmlString.c_str());
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, html]() {
        nativeWebView->NavigateToString(html);
    });
}

void PrivateWebViewWinUAP::OpenFromBuffer(const String& htmlString, const FilePath& basePath)
{
    UriResolver^ resolver = ref new UriResolver(htmlString, basePath);
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, resolver]() {
        Uri^ uri = nativeWebView->BuildLocalStreamUri("DAVA", "/johny23");
        nativeWebView->NavigateToLocalStreamUri(uri, resolver);
    });
}

void PrivateWebViewWinUAP::ExecuteJScript(const String& scriptString)
{
    Platform::String^ script = ref new Platform::String(StringToWString(scriptString).c_str());
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, script]() {
        auto args = ref new Platform::Collections::Vector<Platform::String^>();
        args->Append(script);

        auto js = nativeWebView->InvokeScriptAsync("eval", args);
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
    });
}

void PrivateWebViewWinUAP::SetRect(const Rect& rect)
{
    originalRect = rect;
    bool offScreen = renderToTexture || !visible;
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, offScreen]() {
        PositionWebView(originalRect, offScreen);
    });
}

void PrivateWebViewWinUAP::SetVisible(bool isVisible)
{
    if (visible != isVisible)
    {
        // If control should get invisible it is simply moved off the screen
        visible = isVisible;
        bool offScreen = renderToTexture || !visible;
        auto self{shared_from_this()};
        core->RunOnUIThread([this, self, offScreen]() {
            PositionWebView(originalRect, offScreen);
        });
    }
}

void PrivateWebViewWinUAP::SetBackgroundTransparency(bool enabled)
{
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, enabled]() {
        nativeWebView->DefaultBackgroundColor = enabled ? Colors::Transparent : defaultBkgndColor;
    });
}

void PrivateWebViewWinUAP::SetDelegate(IUIWebViewDelegate* webViewDelegate_)
{
    webViewDelegate = webViewDelegate_;
}

void PrivateWebViewWinUAP::SetRenderToTexture(bool value)
{
    if (renderToTexture != value)
    {
        renderToTexture = value;

        if (uiWebView != nullptr)
        {
            if (!renderToTexture)
            {
                uiWebView->SetSprite(nullptr, 0);
            }

            bool offScreen = renderToTexture || !visible;
            auto self{shared_from_this()};
            core->RunOnUIThread([this, self, offScreen]() {
                PositionWebView(originalRect, offScreen);
                if (renderToTexture)
                {
                    RenderToTexture();
                }
            });
        }
    }
}

bool PrivateWebViewWinUAP::IsRenderToTexture() const
{
    return renderToTexture;
}

void PrivateWebViewWinUAP::InstallEventHandlers()
{
    std::weak_ptr<PrivateWebViewWinUAP> self_weak(shared_from_this());
    // Install event handlers through lambdas as it seems only ref class's member functions can be event handlers directly
    auto navigationStarting = ref new TypedEventHandler<WebView^, WebViewNavigationStartingEventArgs^>([this, self_weak](WebView^ sender, WebViewNavigationStartingEventArgs^ args) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnNavigationStarting(sender, args);
        }
    });
    auto navigationCompleted = ref new TypedEventHandler<WebView^, WebViewNavigationCompletedEventArgs^>([this, self_weak](WebView^ sender, WebViewNavigationCompletedEventArgs^ args) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnNavigationCompleted(sender, args);
        }
    });
    nativeWebView->NavigationStarting += navigationStarting;
    nativeWebView->NavigationCompleted += navigationCompleted;
}

void PrivateWebViewWinUAP::PositionWebView(const Rect& rectInVirtualCoordinates, bool offScreen)
{
    VirtualCoordinatesSystem* coordSystem = VirtualCoordinatesSystem::Instance();

    // 1. map virtual to physical
    Rect controlRect = coordSystem->ConvertVirtualToPhysical(rectInVirtualCoordinates);
    controlRect += coordSystem->GetPhysicalDrawOffset();

    // 2. map physical to window
    const float32 scaleFactor = core->GetScreenScaleFactor();
    controlRect.x /= scaleFactor;
    controlRect.y /= scaleFactor;
    controlRect.dx /= scaleFactor;
    controlRect.dy /= scaleFactor;

    if (offScreen)
    {
        controlRect.x = -controlRect.dx;
        controlRect.y = -controlRect.dy;
    }

    // 3. set control's position and size
    nativeWebView->MinHeight = 0.0;     // Force minimum control sizes to zero to
    nativeWebView->MinWidth = 0.0;      // allow setting any control sizes
    nativeWebView->Width = controlRect.dx;
    nativeWebView->Height = controlRect.dy;
    core->XamlApplication()->PositionUIElement(nativeWebView, controlRect.x, controlRect.y);
}

void PrivateWebViewWinUAP::OnNavigationStarting(WebView^ sender, WebViewNavigationStartingEventArgs^ args)
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

void PrivateWebViewWinUAP::OnNavigationCompleted(WebView^ sender, WebViewNavigationCompletedEventArgs^ args)
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

    auto self{shared_from_this()};
    core->RunOnMainThread([this, self]() {
        if (uiWebView != nullptr && webViewDelegate != nullptr)
        {
            webViewDelegate->PageLoaded(uiWebView);
        }
    });
}

void PrivateWebViewWinUAP::RenderToTexture()
{
    int32 width = static_cast<int32>(nativeWebView->Width);
    int32 height = static_cast<int32>(nativeWebView->Height);

    auto self{shared_from_this()};
    InMemoryRandomAccessStream^ inMemoryStream = ref new InMemoryRandomAccessStream();
    auto taskCapture = create_task(nativeWebView->CapturePreviewToStreamAsync(inMemoryStream));
    taskCapture.then([this, self, inMemoryStream, width, height]()
    {
        size_t streamSize = static_cast<size_t>(inMemoryStream->Size);
        DataReader^ reader = ref new DataReader(inMemoryStream->GetInputStreamAt(0));
        auto taskLoad = create_task(reader->LoadAsync(streamSize));
        taskLoad.then([this, self, reader, width, height, streamSize](task<unsigned int>)
        {
            size_t index = 0;
            std::vector<uint8> buf(streamSize, 0);
            while (reader->UnconsumedBufferLength > 0)
            {
                buf[index] = reader->ReadByte();
                index += 1;
            }

            RefPtr<Sprite> sprite(CreateSpriteFromPreviewData(buf.data(), width, height));
            if (sprite.Valid())
            {
                core->RunOnMainThread([this, self, sprite]() {
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
}

Sprite* PrivateWebViewWinUAP::CreateSpriteFromPreviewData(const uint8* imageData, int32 width, int32 height) const
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

    RefPtr<Image> imgSrc(Image::CreateFromData(width, height, FORMAT_RGBA8888, imageData + bitsOffset));
    return Sprite::CreateFromImage(imgSrc.Get(), true, false);
}

}   // namespace DAVA

#endif // defined(__DAVAENGINE_WIN_UAP__)

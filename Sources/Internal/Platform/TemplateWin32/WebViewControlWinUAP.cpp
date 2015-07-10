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
#include "Platform/TemplateWin32/WebViewControlWinUAP.h"

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
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Filters;
using namespace concurrency;

namespace DAVA
{

/*
    UriResolver used in WebViewControl's OpenFromBuffer method to
    map resource paths in html source to local resource files
*/
private ref class UriResolver sealed : public IUriToStreamResolver
{
    enum eResLocation
    {
        PATH_IN_BIN,        // Resources are relative to app install dir
        PATH_IN_DOC,        // Resources are relative to user documents dir
        PATH_IN_PUB,        // Resources are relative to public documents dir
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
            appDataUri = ref new Uri(L"ms-appdata:///roaming/" + resPath);
            break;
        case PATH_IN_PUB:
            appDataUri = ref new Uri(L"ms-appdata:///local/" + resPath);
            break;
        default:
            return nullptr;
        }
        Platform::String^ foobar = appDataUri->ToString();
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

    return create_async([stream]() -> IInputStream^ {
        return stream;
    });
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
    else if (CheckIfPathReachableFrom(absPath, fs->GetPublicDocumentsPath().GetAbsolutePathname(), pathTail))
    {
        location = PATH_IN_PUB;
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

WebViewControl::WebViewControl(UIWebView& uiWebView_)
    : uiWebView(&uiWebView_)
    , core(static_cast<CorePlatformWinUAP*>(Core::Instance()))
{
    Logger::Debug("****************************** WebViewControl::WebViewControl: %p", this);
}

WebViewControl::~WebViewControl()
{
    Logger::Debug("****************************** WebViewControl::~WebViewControl: %p", this);
    uiWebView = nullptr;
    webViewDelegate = nullptr;
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
        nativeWebView = ref new WebView();
        defaultBkgndColor = nativeWebView->DefaultBackgroundColor;

        core->XamlApplication()->AddUIElement(nativeWebView);

        InstallEventHandlers();
        PositionWebView(originalRect, false);
    });
}

void WebViewControl::OpenURL(const String& urlToOpen)
{
    Uri^ url = ref new Uri(ref new Platform::String(StringToWString(urlToOpen).c_str()));
    core->RunOnUIThread([this, url]() {
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

void WebViewControl::OpenFromBuffer(const String& htmlString, const FilePath& basePath)
{
    UriResolver^ resolver = ref new UriResolver(htmlString, basePath);
    core->RunOnUIThread([this, resolver]() {
        Uri^ uri = nativeWebView->BuildLocalStreamUri("DAVA", "/johny23");
        nativeWebView->NavigateToLocalStreamUri(uri, resolver);
    });
}

void WebViewControl::ExecuteJScript(const String& scriptString)
{
    Platform::String^ script = ref new Platform::String(StringToWString(scriptString).c_str());
    core->RunOnUIThread([this, script]() {
        auto args = ref new Platform::Collections::Vector<Platform::String^>();
        args->Append(script);

        auto js = nativeWebView->InvokeScriptAsync("eval", args);
        create_task(js).then([this](Platform::String^ result) {
            core->RunOnMainThread([this, result]() {
                IUIWebViewDelegate* legate = webViewDelegate.Get();
                UIWebView* view = uiWebView.Get();
                if (legate != nullptr && view != nullptr)
                {
                    String jsResult = WStringToString(result->Data());
                    legate->OnExecuteJScript(view, jsResult);
                }
            });
        }).then([](task<void> t) {
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

void WebViewControl::DeleteCookies(const String& url)
{
    Uri^ uri = ref new Uri(ref new Platform::String(StringToWString(url).c_str()));
    HttpBaseProtocolFilter httpObj;
    HttpCookieManager^ cookieManager = httpObj.CookieManager;
    HttpCookieCollection^ cookies = cookieManager->GetCookies(uri);

    IIterator<HttpCookie^>^ it = cookies->First();
    while (it->HasCurrent)
    {
        HttpCookie^ cookie = it->Current;
        cookieManager->DeleteCookie(cookie);
        it->MoveNext();
    }
}

String WebViewControl::GetCookie(const String& url, const String& name) const
{
    String result;

    Uri^ uri = ref new Uri(ref new Platform::String(StringToWString(url).c_str()));
    HttpBaseProtocolFilter httpObj;
    HttpCookieCollection^ cookies = httpObj.CookieManager->GetCookies(uri);

    Platform::String^ cookieName = ref new Platform::String(StringToWString(name).c_str());
    IIterator<HttpCookie^>^ it = cookies->First();
    while (it->HasCurrent)
    {
        HttpCookie^ cookie = it->Current;
        if (cookie->Name == cookieName)
        {
            result = WStringToString(cookie->Value->Data());
            break;
        }
        it->MoveNext();
    }
    return result;
}

Map<String, String> WebViewControl::GetCookies(const String& url) const
{
    Map<String, String> result;

    Uri^ uri = ref new Uri(ref new Platform::String(StringToWString(url).c_str()));
    HttpBaseProtocolFilter httpObj;
    HttpCookieCollection^ cookies = httpObj.CookieManager->GetCookies(uri);

    IIterator<HttpCookie^>^ it = cookies->First();
    while (it->HasCurrent)
    {
        HttpCookie^ cookie = it->Current;
        result.emplace(WStringToString(cookie->Name->Data()), WStringToString(cookie->Value->Data()));
        it->MoveNext();
    }
    return result;
}

void WebViewControl::SetRect(const Rect& rect)
{
    originalRect = rect;
    bool offScreen = renderToTexture || !visible;
    core->RunOnUIThread([this, offScreen]() {
        PositionWebView(originalRect, offScreen);
    });
}

void WebViewControl::SetVisible(bool isVisible, bool /*hierarchic*/)
{
    if (visible != isVisible)
    {
        // If control should get invisible it is simply moved off the screen
        visible = isVisible;
        bool offScreen = renderToTexture || !visible;
        core->RunOnUIThread([this, offScreen]() {
            PositionWebView(originalRect, offScreen);
        });
    }
}

void WebViewControl::SetBackgroundTransparency(bool enabled)
{
    core->RunOnUIThread([this, enabled]() {
        nativeWebView->DefaultBackgroundColor = enabled ? Colors::Transparent : defaultBkgndColor;
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
            UIWebView* view = uiWebView.Get();
            if (view != nullptr)
            {
                view->SetSprite(nullptr, 0);
            }
        }

        bool offScreen = renderToTexture || !visible;
        core->RunOnUIThread([this, offScreen]() {
            PositionWebView(originalRect, offScreen);
            if (renderToTexture)
            {
                RenderToTexture();
            }
        });
    }
}

void WebViewControl::InstallEventHandlers()
{
    // Install event handlers through lambdas as it seems only ref class's member functions can be event handlers directly
    auto navigationStarting = ref new TypedEventHandler<WebView^, WebViewNavigationStartingEventArgs^>([this](WebView^ sender, WebViewNavigationStartingEventArgs^ args) {
        OnNavigationStarting(sender, args);
    });
    auto navigationCompleted = ref new TypedEventHandler<WebView^, WebViewNavigationCompletedEventArgs^>([this](WebView^ sender, WebViewNavigationCompletedEventArgs^ args) {
        OnNavigationCompleted(sender, args);
    });
    nativeWebView->NavigationStarting += navigationStarting;
    nativeWebView->NavigationCompleted += navigationCompleted;
}

void WebViewControl::PositionWebView(const Rect& rect, bool offScreen)
{
    VirtualCoordinatesSystem* coordSys = VirtualCoordinatesSystem::Instance();

    Rect physRect = coordSys->ConvertVirtualToPhysical(rect);
    const Vector2 physOffset = coordSys->GetPhysicalDrawOffset();

    float32 width = physRect.dx + physOffset.x;
    float32 height = physRect.dy + physOffset.y;

    // When rendering to texture move WebView off the screen
    // as only visible WebView can produce its image
    if (offScreen)
    {
        physRect.x = -width;
        physRect.y = -height;
    }
    nativeWebView->Width = width;
    nativeWebView->Height = height;
    core->XamlApplication()->PositionUIElement(nativeWebView, physRect.x, physRect.y);
}

void WebViewControl::OnNavigationStarting(WebView^ sender, WebViewNavigationStartingEventArgs^ args)
{
    String url;
    if (args->Uri != nullptr)
    {
        url = WStringToString(args->Uri->AbsoluteCanonicalUri->Data());
    }
    Logger::FrameworkDebug("[WebView] OnNavigationStarting: url=%s", url.c_str());

    IUIWebViewDelegate* legate = webViewDelegate.Get();
    UIWebView* view = uiWebView.Get();
    if (legate != nullptr && view != nullptr)
    {
        // For now delegate is running on UI thread not main, as RunOnMainThreadBlocked leads to deadlocks in some case
        // Problem needs further investigation

        bool redirectedByMouse = true;  // For now I don't know how to get redirection method
        IUIWebViewDelegate::eAction whatToDo = legate->URLChanged(view, url, redirectedByMouse);
        if (IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER == whatToDo && args->Uri != nullptr)
        {
            Launcher::LaunchUriAsync(args->Uri);
        }
        args->Cancel = whatToDo != IUIWebViewDelegate::PROCESS_IN_WEBVIEW;
    }
}

void WebViewControl::OnNavigationCompleted(WebView^ sender, WebViewNavigationCompletedEventArgs^ args)
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

    core->RunOnMainThread([this]() {
        IUIWebViewDelegate* legate = webViewDelegate.Get();
        UIWebView* view = uiWebView.Get();
        if (legate != nullptr && view != nullptr)
        {
            legate->PageLoaded(view);
        }
    });
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
            size_t index = 0;
            std::vector<uint8> buf(streamSize, 0);
            while (reader->UnconsumedBufferLength > 0)
            {
                buf[index] = reader->ReadByte();
                index += 1;
            }

            Sprite* sprite = CreateSpriteFromPreviewData(buf.data(), width, height);
            if (sprite != nullptr)
            {
                core->RunOnMainThread([this, sprite]() {
                    UIWebView* view = uiWebView.Get();
                    if (view != nullptr)
                    {
                        view->SetSprite(sprite, 0);
                    }
                    else
                    {
                        sprite->Release();
                    }
                });
            }
        });
    });
}

Sprite* WebViewControl::CreateSpriteFromPreviewData(const uint8* imageData, int32 width, int32 height) const
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
    RefPtr<Image> imgDst(Image::Create(width, height, FORMAT_RGB888));
    if (imgSrc.Valid() && imgDst.Valid())
    {
        ImageConvert::ConvertImageDirect(imgSrc.Get(), imgDst.Get());
        return Sprite::CreateFromImage(imgDst.Get());
    }
    return nullptr;
}

}   // namespace DAVA

#endif // defined(__DAVAENGINE_WIN_UAP__)

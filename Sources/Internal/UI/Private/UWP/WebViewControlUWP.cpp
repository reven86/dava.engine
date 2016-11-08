#include "UI/Private/UWP/WebViewControlUWP.h"

#if defined(__DAVAENGINE_WIN_UAP__) && !defined(DISABLE_NATIVE_WEBVIEW)

#include <ppltasks.h>
#include <collection.h>

#include "Base/RefPtr.h"
#include "Debug/DVAssert.h"

#if defined(__DAVAENGINE_COREV2__)
#include "Engine/EngineModule.h"
#include "Engine/WindowNativeService.h"
#else
#include "Platform/TemplateWin32/WinUAPXamlApp.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#endif
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageConvert.h"
#include "Render/Image/Image.h"

#include "Utils/UTF8Utils.h"
#include "Utils/Utils.h"
#include "Utils/Random.h"
#include "Utils/Utils.h"

#include "UI/UIWebView.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "Logger/Logger.h"
#include "Utils/Utils.h"

namespace DAVA
{
// clang-format off
/*
    UriResolver is intended for mapping resource paths in html source to local resource files

    UriResolver is used in WebViewControl's OpenFromBuffer method which allows to load
    HTML string specifying location of resource files (css, images, etc).
*/
private ref class UriResolver sealed : public ::Windows::Web::IUriToStreamResolver
{
internal:
    UriResolver(const String& htmlData, const FilePath& basePath);

public:
    virtual ::Windows::Foundation::IAsyncOperation<::Windows::Storage::Streams::IInputStream^>^ UriToStreamAsync(::Windows::Foundation::Uri^ uri);

private:
    ::Windows::Foundation::IAsyncOperation<::Windows::Storage::Streams::IInputStream ^> ^ GetStreamFromFilePathAsync(const FilePath& filePath);
    ::Windows::Foundation::IAsyncOperation<::Windows::Storage::Streams::IInputStream^>^ GetStreamFromStringAsync(Platform::String^ s);

    Platform::String^ htmlData;
    FilePath basePath;
};

UriResolver::UriResolver(const String& htmlData_, const FilePath& basePath_)
    : htmlData(ref new Platform::String(UTF8Utils::EncodeToWideString(htmlData_).c_str()))
    , basePath(basePath_)
{
}

::Windows::Foundation::IAsyncOperation<::Windows::Storage::Streams::IInputStream^>^ UriResolver::UriToStreamAsync(::Windows::Foundation::Uri^ uri)
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

::Windows::Foundation::IAsyncOperation<::Windows::Storage::Streams::IInputStream ^> ^ UriResolver::GetStreamFromFilePathAsync(const FilePath& filePath)
{
    using ::concurrency::create_async;
    using ::Windows::Storage::StorageFile;
    using ::Windows::Storage::FileAccessMode;
    using ::Windows::Storage::Streams::IInputStream;
    using ::Windows::Storage::Streams::IRandomAccessStream;
    using ::Windows::Storage::Streams::InMemoryRandomAccessStream;

    String fileNameStr = filePath.GetAbsolutePathname();
    std::replace(fileNameStr.begin(), fileNameStr.end(), '/', '\\');
    Platform::String^ fileName = StringToRTString(fileNameStr);

    return create_async([ this, fileName ]() -> IInputStream^ {
        try
        {
            StorageFile^ storageFile = WaitAsync(StorageFile::GetFileFromPathAsync(fileName));
            IRandomAccessStream^ stream = WaitAsync(storageFile->OpenAsync(FileAccessMode::Read));
            return static_cast<IInputStream^>(stream);
        }
        catch (Platform::COMException^ e)
        {
            Logger::Error("[WebView] failed to load file %s: %s (0x%08x)",
                          RTStringToString(fileName).c_str(),
                          RTStringToString(e->Message).c_str(),
                          e->HResult);
            return ref new InMemoryRandomAccessStream();
        }
    });
}

::Windows::Foundation::IAsyncOperation<::Windows::Storage::Streams::IInputStream^>^ UriResolver::GetStreamFromStringAsync(Platform::String^ s)
{
    using ::concurrency::create_async;
    using ::Windows::Storage::Streams::DataWriter;
    using ::Windows::Storage::Streams::IInputStream;
    using ::Windows::Storage::Streams::InMemoryRandomAccessStream;

    InMemoryRandomAccessStream^ stream = ref new InMemoryRandomAccessStream();
    DataWriter^ writer = ref new DataWriter(stream->GetOutputStreamAt(0));

    writer->WriteString(s);
    WaitAsync(writer->StoreAsync());

    return create_async([stream]() -> IInputStream^ { return stream; });
}
// clang-format on
//////////////////////////////////////////////////////////////////////////

void WebViewControl::WebViewProperties::ClearChangedFlags()
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

#if defined(__DAVAENGINE_COREV2__)
WebViewControl::WebViewControl(Window* w, UIWebView* uiWebView)
    : window(w)
    , uiWebView(uiWebView)
    , properties()
{
}
#else
WebViewControl::WebViewControl(UIWebView* uiWebView)
    : core(static_cast<CorePlatformWinUAP*>(Core::Instance()))
    , uiWebView(uiWebView)
    , properties()
{
}
#endif

WebViewControl::~WebViewControl()
{
    using ::Windows::UI::Xaml::Controls::WebView;
    if (nativeWebView != nullptr)
    {
        // Compiler complains of capturing nativeWebView data member in lambda
        WebView ^ p = nativeWebView;

#if defined(__DAVAENGINE_COREV2__)
        WindowNativeService* nservice = window->GetNativeService();
        window->RunAsyncOnUIThread([p, nservice]() {
            nservice->RemoveXamlControl(p);
        });
#else
        core->RunOnUIThread([p]() {
            // We don't need blocking call here
            static_cast<CorePlatformWinUAP*>(Core::Instance())->XamlApplication()->RemoveUIElement(p);
        });
#endif
        nativeWebView = nullptr;
    }
}

void WebViewControl::OwnerIsDying()
{
    using ::Windows::UI::Xaml::Controls::WebView;

    uiWebView = nullptr;
    webViewDelegate = nullptr;

    // Compiler complains of capturing nativeWebView data member in lambda
    WebView ^ p = nativeWebView;
    Windows::Foundation::EventRegistrationToken tokenNS = tokenNavigationStarting;
    Windows::Foundation::EventRegistrationToken tokenNC = tokenNavigationCompleted;

#if defined(__DAVAENGINE_COREV2__)
    WindowNativeService* nservice = window->GetNativeService();
    window->RunAsyncOnUIThread([p, nservice, tokenNS, tokenNC]() {
        p->NavigationStarting -= tokenNS;
        p->NavigationCompleted -= tokenNC;
    });
#else
    core->RunOnUIThread([p, tokenNS, tokenNC]() {
        // We don't need blocking call here
        p->NavigationStarting -= tokenNS;
        p->NavigationCompleted -= tokenNC;
    });
#endif
}

void WebViewControl::Initialize(const Rect& rect)
{
    properties.createNew = true;

    properties.rect = rect;
    properties.rectInWindowSpace = VirtualToWindow(rect);
    properties.rectChanged = true;
    properties.anyPropertyChanged = true;
}

void WebViewControl::OpenURL(const String& urlToOpen)
{
    properties.urlOrHtml = urlToOpen;
    properties.navigateTo = WebViewProperties::NAVIGATE_OPEN_URL;
    properties.anyPropertyChanged = true;
}

void WebViewControl::LoadHtmlString(const WideString& htmlString)
{
    properties.htmlString = htmlString;
    properties.navigateTo = WebViewProperties::NAVIGATE_LOAD_HTML;
    properties.anyPropertyChanged = true;
}

void WebViewControl::OpenFromBuffer(const String& htmlString, const FilePath& basePath)
{
    properties.urlOrHtml = htmlString;
    properties.basePath = basePath;
    properties.navigateTo = WebViewProperties::NAVIGATE_OPEN_BUFFER;
    properties.anyPropertyChanged = true;
}

void WebViewControl::ExecuteJScript(const String& scriptString)
{
    properties.jsScript = scriptString;
    properties.execJavaScript = true;
    properties.anyPropertyChanged = true;
}

void WebViewControl::SetRect(const Rect& rect)
{
    if (properties.rect != rect)
    {
        properties.rect = rect;
        properties.rectInWindowSpace = VirtualToWindow(rect);
        properties.rectChanged = true;
        properties.anyPropertyChanged = true;
    }
}

void WebViewControl::SetVisible(bool isVisible, bool /*hierarchic*/)
{
    if (properties.visible != isVisible)
    {
        properties.visible = isVisible;
        properties.visibleChanged = true;
        properties.anyPropertyChanged = true;
        if (!isVisible)
        { // Immediately hide native control if it has been already created
            auto self{ shared_from_this() };
#if defined(__DAVAENGINE_COREV2__)
            window->RunAsyncOnUIThread([this, self]() {
                if (nativeWebView != nullptr)
                {
                    SetNativePositionAndSize(rectInWindowSpace, true);
                }
            });
#else
            core->RunOnUIThread([this, self]() {
                if (nativeWebView != nullptr)
                {
                    SetNativePositionAndSize(rectInWindowSpace, true);
                }
            });
#endif
        }
    }
}

void WebViewControl::SetBackgroundTransparency(bool enabled)
{
    if (properties.backgroundTransparency != enabled)
    {
        properties.backgroundTransparency = enabled;
        properties.backgroundTransparencyChanged = true;
        properties.anyPropertyChanged = true;
    }
}

void WebViewControl::SetDelegate(IUIWebViewDelegate* webViewDelegate_, UIWebView* /*uiWebView*/)
{
    webViewDelegate = webViewDelegate_;
}

void WebViewControl::SetRenderToTexture(bool value)
{
    if (properties.renderToTexture != value)
    {
        properties.renderToTexture = value;
        properties.renderToTextureChanged = true;
        properties.anyPropertyChanged = true;
        if (!value)
        { // Immediately hide native control if it has been already created
            auto self{ shared_from_this() };
#if defined(__DAVAENGINE_COREV2__)
            window->RunAsyncOnUIThread([this, self]() {
                if (nativeWebView != nullptr)
                {
                    SetNativePositionAndSize(rectInWindowSpace, true);
                }
            });
#else
            core->RunOnUIThread([this, self]() {
                if (nativeWebView != nullptr)
                {
                    SetNativePositionAndSize(rectInWindowSpace, true);
                }
            });
#endif
        }
    }
}

bool WebViewControl::IsRenderToTexture() const
{
    return properties.renderToTexture;
}

void WebViewControl::Update()
{
    if (properties.createNew || properties.anyPropertyChanged)
    {
        auto self{ shared_from_this() };
        WebViewProperties props(properties);
#if defined(__DAVAENGINE_COREV2__)
        window->RunAsyncOnUIThread([this, self, props]() {
            ProcessProperties(props);
        });
#else
        core->RunOnUIThread([this, self, props] {
            ProcessProperties(props);
        });
#endif

        properties.createNew = false;
        properties.ClearChangedFlags();
    }
}

void WebViewControl::CreateNativeControl()
{
    using ::Windows::UI::Xaml::Visibility;
    using ::Windows::UI::Xaml::Controls::WebView;

    nativeWebView = ref new WebView();
    defaultBkgndColor = nativeWebView->DefaultBackgroundColor;
    InstallEventHandlers();

    nativeWebView->MinWidth = 0.0;
    nativeWebView->MinHeight = 0.0;
    nativeWebView->Visibility = Visibility::Visible;

#if defined(__DAVAENGINE_COREV2__)
    window->GetNativeService()->AddXamlControl(nativeWebView);
#else
    core->XamlApplication()->AddUIElement(nativeWebView);
#endif
    SetNativePositionAndSize(rectInWindowSpace, true); // After creation move native control offscreen
}

void WebViewControl::InstallEventHandlers()
{
    using ::Windows::Foundation::TypedEventHandler;
    using ::Windows::UI::Xaml::Controls::WebView;
    using ::Windows::UI::Xaml::Controls::WebViewNavigationStartingEventArgs;
    using ::Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs;

    // clang-format off
    std::weak_ptr<WebViewControl> self_weak(shared_from_this());
    // Install event handlers through lambdas as it seems only ref class's member functions can be event handlers directly
    auto navigationStarting = ref new TypedEventHandler<WebView^, WebViewNavigationStartingEventArgs^>([this, self_weak](WebView^ sender, WebViewNavigationStartingEventArgs^ args) {
        if (auto self = self_weak.lock())
            OnNavigationStarting(sender, args);
    });
    auto navigationCompleted = ref new TypedEventHandler<WebView^, WebViewNavigationCompletedEventArgs^>([this, self_weak](WebView^ sender, WebViewNavigationCompletedEventArgs^ args) {
        if (auto self = self_weak.lock())
            OnNavigationCompleted(sender, args);
    });
    tokenNavigationStarting = nativeWebView->NavigationStarting += navigationStarting;
    tokenNavigationCompleted = nativeWebView->NavigationCompleted += navigationCompleted;
    // clang-format on
}

void WebViewControl::OnNavigationStarting(::Windows::UI::Xaml::Controls::WebView ^ sender, ::Windows::UI::Xaml::Controls::WebViewNavigationStartingEventArgs ^ args)
{
    String url;
    if (args->Uri != nullptr)
    {
        url = WStringToString(args->Uri->AbsoluteCanonicalUri->Data());
    }
    Logger::FrameworkDebug("[WebView] OnNavigationStarting: url=%s", url.c_str());

    bool redirectedByMouse = false; // For now I don't know how to get redirection method
    IUIWebViewDelegate::eAction whatToDo = IUIWebViewDelegate::PROCESS_IN_WEBVIEW;
#if defined(__DAVAENGINE_COREV2__)
    window->GetEngine()->RunAndWaitOnMainThread([this, &whatToDo, &url, redirectedByMouse]() {
        if (uiWebView != nullptr && webViewDelegate != nullptr)
        {
            whatToDo = webViewDelegate->URLChanged(uiWebView, url, redirectedByMouse);
        }
    });
#else
    core->RunOnMainThreadBlocked([this, &whatToDo, &url, redirectedByMouse]() {
        if (uiWebView != nullptr && webViewDelegate != nullptr)
        {
            whatToDo = webViewDelegate->URLChanged(uiWebView, url, redirectedByMouse);
        }
    });
#endif

    if (IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER == whatToDo && args->Uri != nullptr)
    {
        ::Windows::System::Launcher::LaunchUriAsync(args->Uri);
    }
    args->Cancel = whatToDo != IUIWebViewDelegate::PROCESS_IN_WEBVIEW;
}

void WebViewControl::OnNavigationCompleted(::Windows::UI::Xaml::Controls::WebView ^ sender, ::Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs ^ args)
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
#if defined(__DAVAENGINE_COREV2__)
    window->GetEngine()->RunAsyncOnMainThread([this, self]() {
        if (uiWebView != nullptr && webViewDelegate != nullptr)
        {
            webViewDelegate->PageLoaded(uiWebView);
        }
    });
#else
    core->RunOnMainThread([this, self]() {
        if (uiWebView != nullptr && webViewDelegate != nullptr)
        {
            webViewDelegate->PageLoaded(uiWebView);
        }
    });
#endif
}

void WebViewControl::ProcessProperties(const WebViewProperties& props)
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

void WebViewControl::ApplyChangedProperties(const WebViewProperties& props)
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

void WebViewControl::SetNativePositionAndSize(const Rect& rect, bool offScreen)
{
    float32 xOffset = 0.0f;
    float32 yOffset = 0.0f;
    if (offScreen)
    {
        // Move control very far offscreen as on phone control with disabled input remains visible
        xOffset = rect.x + rect.dx + 1000.0f;
        yOffset = rect.y + rect.dy + 1000.0f;
    }
    nativeWebView->Width = std::max(0.0f, rect.dx);
    nativeWebView->Height = std::max(0.0f, rect.dy);
#if defined(__DAVAENGINE_COREV2__)
    window->GetNativeService()->PositionXamlControl(nativeWebView, rect.x - xOffset, rect.y - yOffset);
#else
    core->XamlApplication()->PositionUIElement(nativeWebView, rect.x - xOffset, rect.y - yOffset);
#endif
}

void WebViewControl::SetNativeBackgroundTransparency(bool enabled)
{
    using ::Windows::UI::Colors;
    nativeWebView->DefaultBackgroundColor = enabled ? Colors::Transparent : defaultBkgndColor;
}

void WebViewControl::NativeNavigateTo(const WebViewProperties& props)
{
    using ::Windows::Foundation::Uri;

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
        // Generate some unique content identifier for each request
        // as WebViews' backend can remember content id and reuse UriResolver instance
        // for another WebView control
        uint32 generatedContentId = Random::Instance()->Rand();
        Platform::String^ contentId = ref new Platform::String(StringToWString(Format("%u", generatedContentId)).c_str());

        UriResolver^ resolver = ref new UriResolver(props.urlOrHtml, props.basePath);
        Uri^ uri = nativeWebView->BuildLocalStreamUri(contentId, "/johny23");
        nativeWebView->NavigateToLocalStreamUri(uri, resolver);
    }
    // clang-format on
}

void WebViewControl::NativeExecuteJavaScript(const String& jsScript)
{
    using ::concurrency::create_task;
    using ::concurrency::task;

    // clang-format off
    Platform::String^ script = ref new Platform::String(StringToWString(jsScript).c_str());

    auto args = ref new Platform::Collections::Vector<Platform::String^>();
    args->Append(script);

    auto js = nativeWebView->InvokeScriptAsync(L"eval", args);
    auto self{shared_from_this()};
    create_task(js).then([this, self](Platform::String^ result) {
#if defined(__DAVAENGINE_COREV2__)
        window->GetEngine()->RunAsyncOnMainThread([this, self, result]() {
            if (webViewDelegate != nullptr && uiWebView != nullptr)
            {
                String jsResult = WStringToString(result->Data());
                webViewDelegate->OnExecuteJScript(uiWebView, jsResult);
            }
        });
#else
        core->RunOnMainThread([this, self, result]() {
            if (webViewDelegate != nullptr && uiWebView != nullptr)
            {
                String jsResult = WStringToString(result->Data());
                webViewDelegate->OnExecuteJScript(uiWebView, jsResult);
            }
        });
#endif
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

Rect WebViewControl::VirtualToWindow(const Rect& srcRect) const
{
    VirtualCoordinatesSystem* coordSystem = VirtualCoordinatesSystem::Instance();

    // 1. map virtual to physical
    Rect rect = coordSystem->ConvertVirtualToPhysical(srcRect);
    rect += coordSystem->GetPhysicalDrawOffset();

#if defined(__DAVAENGINE_COREV2__)
    // 2. map physical to window
    const float32 scaleFactor = window->GetRenderSurfaceScaleX();
#else
    // 2. map physical to window
    const float32 scaleFactor = core->GetScreenScaleFactor();
#endif
    rect.x /= scaleFactor;
    rect.y /= scaleFactor;
    rect.dx /= scaleFactor;
    rect.dy /= scaleFactor;
    return rect;
}

void WebViewControl::RenderToTexture()
{
    using ::concurrency::create_task;
    using ::concurrency::task;
    using ::Windows::Storage::Streams::DataReader;
    using ::Windows::Storage::Streams::InMemoryRandomAccessStream;

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
            Vector<uint8> buf(streamSize, 0);
            while (reader->UnconsumedBufferLength > 0)
            {
                buf[index] = reader->ReadByte();
                index += 1;
            }

            RefPtr<Sprite> sprite(CreateSpriteFromPreviewData(&buf[0], width, height));
            if (sprite.Valid())
            {
#if defined(__DAVAENGINE_COREV2__)
                window->GetEngine()->RunAsyncOnMainThread([this, self, sprite]()
                {
                    if (uiWebView != nullptr)
                    {
                        uiWebView->SetSprite(sprite.Get(), 0);
                    }
                });
#else
                core->RunOnMainThread([this, self, sprite]()
                {
                    if (uiWebView != nullptr)
                    {
                        uiWebView->SetSprite(sprite.Get(), 0);
                    }
                });
#endif
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

Sprite* WebViewControl::CreateSpriteFromPreviewData(uint8* imageData, int32 width, int32 height) const
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

void WebViewControl::DeleteCookies(const String& url)
{
    using ::Windows::Foundation::Uri;
    using ::Windows::Foundation::Collections::IIterator;
    using namespace ::Windows::Web::Http;
    using namespace ::Windows::Web::Http::Filters;

    Uri ^ uri = ref new Uri(ref new Platform::String(StringToWString(url).c_str()));
    HttpBaseProtocolFilter httpObj;
    HttpCookieManager ^ cookieManager = httpObj.CookieManager;
    HttpCookieCollection ^ cookies = cookieManager->GetCookies(uri);

    IIterator<HttpCookie ^> ^ it = cookies->First();
    while (it->HasCurrent)
    {
        HttpCookie ^ cookie = it->Current;
        cookieManager->DeleteCookie(cookie);
        it->MoveNext();
    }
}

String WebViewControl::GetCookie(const String& url, const String& name) const
{
    using ::Windows::Foundation::Uri;
    using ::Windows::Foundation::Collections::IIterator;
    using namespace ::Windows::Web::Http;
    using namespace ::Windows::Web::Http::Filters;

    String result;

    Uri ^ uri = ref new Uri(ref new Platform::String(StringToWString(url).c_str()));
    HttpBaseProtocolFilter httpObj;
    HttpCookieCollection ^ cookies = httpObj.CookieManager->GetCookies(uri);

    Platform::String ^ cookieName = ref new Platform::String(StringToWString(name).c_str());
    IIterator<HttpCookie ^> ^ it = cookies->First();
    while (it->HasCurrent)
    {
        HttpCookie ^ cookie = it->Current;
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
    using ::Windows::Foundation::Uri;
    using ::Windows::Foundation::Collections::IIterator;
    using namespace ::Windows::Web::Http;
    using namespace ::Windows::Web::Http::Filters;

    Map<String, String> result;

    Uri ^ uri = ref new Uri(ref new Platform::String(StringToWString(url).c_str()));
    HttpBaseProtocolFilter httpObj;
    HttpCookieCollection ^ cookies = httpObj.CookieManager->GetCookies(uri);

    IIterator<HttpCookie ^> ^ it = cookies->First();
    while (it->HasCurrent)
    {
        HttpCookie ^ cookie = it->Current;
        result.emplace(WStringToString(cookie->Name->Data()), WStringToString(cookie->Value->Data()));
        it->MoveNext();
    }
    return result;
}

} // namespace DAVA

#endif // (__DAVAENGINE_WIN_UAP__) && !(DISABLE_NATIVE_WEBVIEW)

#include "Platform/TemplateWin32/PrivateMovieViewWinUAP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Debug/DVAssert.h"
#include "FileSystem/FileSystem.h"
#include "Logger/Logger.h"
#include "Utils/Utils.h"

#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#if defined(__DAVAENGINE_COREV2__)
#include "Engine/Engine.h"
#include "Engine/Private/NativeWindow.h"
#include "Render/RHI/rhi_Public.h"
#else
#include "Platform/TemplateWin32/WinUAPXamlApp.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#endif

using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace concurrency;

namespace DAVA
{
void PrivateMovieViewWinUAP::MovieViewProperties::ClearChangedFlags()
{
    anyPropertyChanged = false;
    rectChanged = false;
    visibleChanged = false;
    streamChanged = false;
    actionChanged = false;
}

PrivateMovieViewWinUAP::PrivateMovieViewWinUAP()
#if defined(__DAVAENGINE_COREV2__)
    : window(Engine::Instance()->PrimaryWindow())
#else
    : core(static_cast<CorePlatformWinUAP*>(Core::Instance()))
#endif
    , properties()
{
}

PrivateMovieViewWinUAP::~PrivateMovieViewWinUAP()
{
    if (nativeControl != nullptr)
    {
        MediaElement ^ p = nativeControl;
#if defined(__DAVAENGINE_COREV2__)
        Private::WindowWinUWP* nativeWindow = window->GetNativeWindow();
        window->RunAsyncOnUIThread([p, nativeWindow]() {
            nativeWindow->RemoveXamlControl(p);
        });
#else
        core->RunOnUIThread([p]() { // We don't need blocking call here
            static_cast<CorePlatformWinUAP*>(Core::Instance())->XamlApplication()->RemoveUIElement(p);
        });
#endif
        nativeControl = nullptr;
    }
}

void PrivateMovieViewWinUAP::OwnerAtPremortem()
{
    // For now do nothing here
}

void PrivateMovieViewWinUAP::Initialize(const Rect& rect)
{
    properties.createNew = true;
}

void PrivateMovieViewWinUAP::SetRect(const Rect& rect)
{
    if (properties.rect != rect)
    {
        properties.rect = rect;
        properties.rectInWindowSpace = VirtualToWindow(rect);
        properties.rectChanged = true;
        properties.anyPropertyChanged = true;
    }
}

void PrivateMovieViewWinUAP::SetVisible(bool isVisible)
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
                if (nativeControl != nullptr)
                {
                    SetNativeVisible(false);
                }
            });
#else
            core->RunOnUIThread([this, self]() {
                if (nativeControl != nullptr)
                {
                    SetNativeVisible(false);
                }
            });
#endif
        }
    }
}

void PrivateMovieViewWinUAP::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    using ::Windows::UI::Xaml::Media::Stretch;
    using ::Windows::Storage::Streams::IRandomAccessStream;

    properties.stream = nullptr;
    properties.playing = false;
    properties.canPlay = false;

    IRandomAccessStream ^ stream = CreateStreamFromFilePath(moviePath);
    if (stream != nullptr)
    {
        properties.canPlay = true;

        Stretch scaling = Stretch::None;
        switch (params.scalingMode)
        {
        case scalingModeNone:
            scaling = Stretch::None;
            break;
        case scalingModeAspectFit:
            scaling = Stretch::Uniform;
            break;
        case scalingModeAspectFill:
            scaling = Stretch::UniformToFill;
            break;
        case scalingModeFill:
            scaling = Stretch::Fill;
            break;
        default:
            scaling = Stretch::None;
            break;
        }

        properties.stream = stream;
        properties.scaling = scaling;
        properties.streamChanged = true;
        properties.anyPropertyChanged = true;
    }
}

void PrivateMovieViewWinUAP::Play()
{
    if (properties.canPlay)
    {
        // It seems that dava.engine is client of game but not vice versa
        // Game does not take into account that video playback can take some time after Play() has been called
        // So assume movie is playing under following conditions:
        //  - movie is really playing
        //  - game has called Play() method
        properties.playing = true;

        properties.action = MovieViewProperties::ACTION_PLAY;
        properties.actionChanged = true;
        properties.anyPropertyChanged = true;
    }
}

void PrivateMovieViewWinUAP::Stop()
{
    // Game plays intro movie in the following sequence:
    //  1. movie->Play();
    //  2. while (movie->IsPlaying()) {}
    //  3. movie->Stop();
    // After Stop() method has been called native control shows first movie frame
    // so UIMovieView emulates Stop() through Pause()
    Pause();

    // DO NOT DELETE COMMENTED CODE
    // if (properties.canPlay)
    // {
    //     properties.playing = false;
    //
    //     properties.action = MovieViewProperties::ACTION_STOP;
    //     properties.actionChanged = true;
    //     properties.anyPropertyChanged = true;
    // }
}

void PrivateMovieViewWinUAP::Pause()
{
    if (properties.canPlay)
    {
        properties.playing = false;

        properties.action = MovieViewProperties::ACTION_PAUSE;
        properties.actionChanged = true;
        properties.anyPropertyChanged = true;
    }
}

void PrivateMovieViewWinUAP::Resume()
{
    Play();
}

void PrivateMovieViewWinUAP::Update()
{
    if (properties.anyPropertyChanged || properties.createNew)
    {
        auto self{ shared_from_this() };
        MovieViewProperties props(properties);
#if defined(__DAVAENGINE_COREV2__)
        window->RunAsyncOnUIThread([this, self, props]() {
            ProcessProperties(props);
        });
#else
        core->RunOnUIThread([this, self, props]() {
            ProcessProperties(props);
        });
#endif

        properties.createNew = false;
        properties.stream = nullptr;
        properties.ClearChangedFlags();
    }
}

void PrivateMovieViewWinUAP::ProcessProperties(const MovieViewProperties& props)
{
    if (props.createNew)
    {
        nativeControl = ref new MediaElement();
        nativeControl->AllowDrop = false;
        nativeControl->CanDrag = false;
        nativeControl->AutoPlay = false;
        nativeControl->MinHeight = 0.0; // Force minimum control sizes to zero to
        nativeControl->MinWidth = 0.0; // allow setting any control sizes
        nativeControl->Volume = 1.0;

#if defined(__DAVAENGINE_COREV2__)
        window->GetNativeWindow()->AddXamlControl(nativeControl);
#else
        core->XamlApplication()->AddUIElement(nativeControl);
#endif
        InstallEventHandlers();
    }

    if (props.anyPropertyChanged)
    {
        ApplyChangedProperties(props);
    }
}

void PrivateMovieViewWinUAP::ApplyChangedProperties(const MovieViewProperties& props)
{
    if (props.visibleChanged)
        SetNativeVisible(props.visible);
    if (props.rectChanged)
        SetNativePositionAndSize(props.rectInWindowSpace);
    if (props.streamChanged)
    {
        nativeControl->Stretch = props.scaling;
        nativeControl->SetSource(props.stream, L"");
        movieLoaded = false;
        playAfterLoaded = false;
    }
    if (props.actionChanged)
    {
        if (movieLoaded)
        {
            switch (props.action)
            {
            case MovieViewProperties::ACTION_PLAY:
                nativeControl->Play();
                break;
            case MovieViewProperties::ACTION_PAUSE:
                nativeControl->Pause();
                break;
            case MovieViewProperties::ACTION_RESUME:
                nativeControl->Play();
                break;
            case MovieViewProperties::ACTION_STOP:
                nativeControl->Stop();
                break;
            }
        }
        playAfterLoaded = !movieLoaded && props.action == MovieViewProperties::ACTION_PLAY;
    }
}

void PrivateMovieViewWinUAP::InstallEventHandlers()
{
    std::weak_ptr<PrivateMovieViewWinUAP> self_weak(shared_from_this());
    // Install event handlers through lambdas as it seems only ref class's member functions can be event handlers directly
    auto mediaOpened = ref new RoutedEventHandler([this, self_weak](Platform::Object ^, RoutedEventArgs ^ ) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnMediaOpened();
        }
    });
    auto mediaEnded = ref new RoutedEventHandler([this, self_weak](Platform::Object ^, RoutedEventArgs ^ ) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnMediaEnded();
        }
    });
    auto mediaFailed = ref new ExceptionRoutedEventHandler([this, self_weak](Platform::Object ^, ExceptionRoutedEventArgs ^ args) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnMediaFailed(args);
        }
    });
    nativeControl->MediaOpened += mediaOpened;
    nativeControl->MediaEnded += mediaEnded;
    nativeControl->MediaFailed += mediaFailed;
}

Windows::Storage::Streams::IRandomAccessStream ^ PrivateMovieViewWinUAP::CreateStreamFromFilePath(const FilePath& path) const
{
    String pathName = path.GetAbsolutePathname();
    std::replace(pathName.begin(), pathName.end(), '/', '\\');
    Platform::String ^ filePath = StringToRTString(pathName);

    try
    {
        StorageFile ^ file = WaitAsync(StorageFile::GetFileFromPathAsync(filePath));
        if (file != nullptr)
        {
            return WaitAsync(file->OpenAsync(FileAccessMode::Read));
        }
        return nullptr;
    }
    catch (Platform::COMException ^ e)
    {
        Logger::Error("[MovieView] failed to load file %s: %s (0x%08x)",
                      RTStringToString(filePath).c_str(),
                      RTStringToString(e->Message).c_str(),
                      e->HResult);
        return nullptr;
    }
}

void PrivateMovieViewWinUAP::SetNativeVisible(bool visible)
{
    nativeControl->Visibility = visible ? Visibility::Visible : Visibility::Collapsed;
}

void PrivateMovieViewWinUAP::SetNativePositionAndSize(const Rect& rect)
{
    nativeControl->Width = std::max(0.0f, rect.dx);
    nativeControl->Height = std::max(0.0f, rect.dy);
#if defined(__DAVAENGINE_COREV2__)
    window->GetNativeWindow()->PositionXamlControl(nativeControl, rect.x, rect.y);
#else
    core->XamlApplication()->PositionUIElement(nativeControl, rect.x, rect.y);
#endif

    { //'workaround' for ATI HD ****G adapters
        const char* gpuDesc = rhi::DeviceCaps().deviceDescription;
        if (strstr(gpuDesc, "AMD Radeon HD") && gpuDesc[strlen(gpuDesc) - 1] == 'G')
        {
            nativeControl->Height += 1.0;
        }
    }
}

Rect PrivateMovieViewWinUAP::VirtualToWindow(const Rect& srcRect) const
{
    VirtualCoordinatesSystem* coordSystem = VirtualCoordinatesSystem::Instance();

    // 1. map virtual to physical
    Rect rect = coordSystem->ConvertVirtualToPhysical(srcRect);
    rect += coordSystem->GetPhysicalDrawOffset();

// 2. map physical to window
#if defined(__DAVAENGINE_COREV2__)
    const float32 scaleFactor = window->GetRenderSurfaceScaleX();
#else
    const float32 scaleFactor = core->GetScreenScaleFactor();
#endif
    rect.x /= scaleFactor;
    rect.y /= scaleFactor;
    rect.dx /= scaleFactor;
    rect.dy /= scaleFactor;
    return rect;
}

void PrivateMovieViewWinUAP::TellPlayingStatus(bool playing)
{
#if defined(__DAVAENGINE_COREV2__)
    window->RunAsyncOnUIThread([this, playing]() {
        properties.playing = playing;
    });
#else
    core->RunOnMainThread([this, playing]() {
        properties.playing = playing;
    });
#endif
}

void PrivateMovieViewWinUAP::OnMediaOpened()
{
    movieLoaded = true;
    if (playAfterLoaded)
    {
        playAfterLoaded = false;
        nativeControl->Play();
    }
}

void PrivateMovieViewWinUAP::OnMediaEnded()
{
    TellPlayingStatus(false);
}

void PrivateMovieViewWinUAP::OnMediaFailed(ExceptionRoutedEventArgs ^ args)
{
    TellPlayingStatus(false);
    String errMessage = WStringToString(args->ErrorMessage->Data());
    Logger::Error("[MovieView] failed to decode media file: %s", errMessage.c_str());
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__

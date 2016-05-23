#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Debug/DVAssert.h"
#include "FileSystem/FileSystem.h"
#include "Logger/Logger.h"
#include "Utils/Utils.h"

#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#include "Platform/TemplateWin32/WinUAPXamlApp.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#include "Platform/TemplateWin32/PrivateMovieViewWinUAP.h"

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
PrivateMovieViewWinUAP::PrivateMovieViewWinUAP()
    : core(static_cast<CorePlatformWinUAP*>(Core::Instance()))
{
}

PrivateMovieViewWinUAP::~PrivateMovieViewWinUAP()
{
    if (nativeMovieView != nullptr)
    {
        // Compiler complains of capturing nativeWebView data member in lambda
        MediaElement ^ p = nativeMovieView;
        core->RunOnUIThread([p]() { // We don't need blocking call here
            static_cast<CorePlatformWinUAP*>(Core::Instance())->XamlApplication()->RemoveUIElement(p);
        });
        nativeMovieView = nullptr;
    }
}

void PrivateMovieViewWinUAP::OwnerAtPremortem()
{
    // For now do nothing here
}

void PrivateMovieViewWinUAP::Initialize(const Rect& rect)
{
    core->RunOnUIThreadBlocked([this, &rect]() {
        nativeMovieView = ref new MediaElement();
        nativeMovieView->AllowDrop = false;
        nativeMovieView->CanDrag = false;
        nativeMovieView->AutoPlay = false;
        nativeMovieView->Volume = 1.0;

        core->XamlApplication()->AddUIElement(nativeMovieView);

        InstallEventHandlers();
        PositionMovieView(rect);
    });
}

void PrivateMovieViewWinUAP::SetRect(const Rect& rect)
{
    auto self{ shared_from_this() };
    core->RunOnUIThread([this, self, rect]() {
        PositionMovieView(rect);
    });
}

void PrivateMovieViewWinUAP::SetVisible(bool isVisible)
{
    if (visible != isVisible)
    {
        visible = isVisible;
        auto self{ shared_from_this() };
        core->RunOnUIThread([this, self]() {
            nativeMovieView->Visibility = visible ? Visibility::Visible : Visibility::Collapsed;
        });
    }
}

void PrivateMovieViewWinUAP::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    IRandomAccessStream ^ stream = CreateStreamFromFilePath(moviePath);
    if (stream)
    {
        OpenMovieFromStream(stream, params);
    }
}

void PrivateMovieViewWinUAP::OpenMovieFromStream(IRandomAccessStream ^ stream, const OpenMovieParams& params)
{
    movieLoaded = false;
    playRequest = false;
    moviePlaying = false;

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

    auto self{ shared_from_this() };
    core->RunOnUIThread([this, self, stream, scaling]() {
        nativeMovieView->Stretch = scaling;
        nativeMovieView->SetSource(stream, L"");
    });
}

void PrivateMovieViewWinUAP::Play()
{
    if (movieLoaded)
    {
        if (!moviePlaying)
        {
            moviePlaying = true;
            auto self{ shared_from_this() };
            core->RunOnUIThread([this, self]()
                                {
                                    nativeMovieView->Play();
                                });
        }
    }
    else
    {
        playRequest = true;
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
    //playRequest = false;
    //moviePlaying = false;
    //if (movieLoaded)
    //{
    //    auto self{shared_from_this()};
    //    core->RunOnUIThread([this, self]() {
    //        nativeMovieView->Stop();
    //    });
    //}
}

void PrivateMovieViewWinUAP::Pause()
{
    playRequest = false;
    moviePlaying = false;
    if (movieLoaded)
    {
        auto self{ shared_from_this() };
        core->RunOnUIThread([this, self]() {
            nativeMovieView->Pause();
        });
    }
}

void PrivateMovieViewWinUAP::Resume()
{
    Play();
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
    nativeMovieView->MediaOpened += mediaOpened;
    nativeMovieView->MediaEnded += mediaEnded;
    nativeMovieView->MediaFailed += mediaFailed;
}

void PrivateMovieViewWinUAP::PositionMovieView(const Rect& rectInVirtualCoordinates)
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

    // 3. set control's position and size
    nativeMovieView->MinHeight = 0.0; // Force minimum control sizes to zero to
    nativeMovieView->MinWidth = 0.0; // allow setting any control sizes
    nativeMovieView->Width = std::max(0.0f, controlRect.dx);
    nativeMovieView->Height = std::max(0.0f, controlRect.dy);
    core->XamlApplication()->PositionUIElement(nativeMovieView, controlRect.x, controlRect.y);

    { //'workaround' for ATI HD ****G adapters
        const char* gpuDesc = rhi::DeviceCaps().deviceDescription;
        if (strstr(gpuDesc, "AMD Radeon HD") && gpuDesc[strlen(gpuDesc) - 1] == 'G')
        {
            nativeMovieView->Height += 1.0;
        }
    }
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

void PrivateMovieViewWinUAP::OnMediaOpened()
{
    movieLoaded = true;
    if (playRequest)
    {
        playRequest = false;
        moviePlaying = true;
        nativeMovieView->Play();
    }
}

void PrivateMovieViewWinUAP::OnMediaEnded()
{
    playRequest = false;
    moviePlaying = false;
}

void PrivateMovieViewWinUAP::OnMediaFailed(ExceptionRoutedEventArgs ^ args)
{
    playRequest = false;
    moviePlaying = false;
    String errMessage = WStringToString(args->ErrorMessage->Data());
    Logger::Error("[MovieView] failed to decode media file: %s", errMessage.c_str());
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__

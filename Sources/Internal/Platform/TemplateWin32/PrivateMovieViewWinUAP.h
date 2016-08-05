#pragma once

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "UI/IMovieViewControl.h"

#include "Engine/EngineFwd.h"

namespace DAVA
{
#if !defined(__DAVAENGINE_COREV2__)
class CorePlatformWinUAP;
#endif
class PrivateMovieViewWinUAP : public std::enable_shared_from_this<PrivateMovieViewWinUAP>
{
    struct MovieViewProperties
    {
        enum
        {
            ACTION_PLAY,
            ACTION_PAUSE,
            ACTION_RESUME,
            ACTION_STOP,
        };

        void ClearChangedFlags();

        Rect rect;
        Rect rectInWindowSpace;
        bool visible = false;
        bool playing = false;
        bool canPlay = false;
        int32 action = ACTION_STOP;
        ::Windows::Storage::Streams::IRandomAccessStream ^ stream = nullptr;
        ::Windows::UI::Xaml::Media::Stretch scaling = ::Windows::UI::Xaml::Media::Stretch::None;

        bool createNew : 1;

        bool anyPropertyChanged : 1;
        bool rectChanged : 1;
        bool visibleChanged : 1;
        bool streamChanged : 1;
        bool actionChanged : 1;
    };

public:
    PrivateMovieViewWinUAP();
    ~PrivateMovieViewWinUAP();

    // MovieViewControl should invoke it in its destructor to tell this class instance
    // to fly away on its own (finish pending jobs if any, and delete when all references are lost)
    void OwnerAtPremortem();

    void Initialize(const Rect& rect);

    void SetRect(const Rect& rect);
    void SetVisible(bool isVisible);

    void OpenMovie(const FilePath& moviePath, const OpenMovieParams& params);

    void Play();
    void Stop();

    void Pause();
    void Resume();

    bool IsPlaying();

    void Update();

private:
    void ProcessProperties(const MovieViewProperties& props);
    void ApplyChangedProperties(const MovieViewProperties& props);

    void InstallEventHandlers();

    Windows::Storage::Streams::IRandomAccessStream ^ CreateStreamFromFilePath(const FilePath& path) const;

    void SetNativeVisible(bool visible);
    void SetNativePositionAndSize(const Rect& rect);

    Rect VirtualToWindow(const Rect& srcRect) const;
    void TellPlayingStatus(bool playing);

private: // MediaElement event handlers
    void OnMediaOpened();
    void OnMediaEnded();
    void OnMediaFailed(Windows::UI::Xaml::ExceptionRoutedEventArgs ^ args);

private:
#if defined(__DAVAENGINE_COREV2__)
    Window* window = nullptr;
#else
    CorePlatformWinUAP* core;
#endif
    Windows::UI::Xaml::Controls::MediaElement ^ nativeControl = nullptr;
    bool playAfterLoaded = false; // Movie should play after loading as Play() can be invoked earlier than movie has been loaded
    bool movieLoaded = false; // Movie has been successfully loaded and decoded

    MovieViewProperties properties;
};

//////////////////////////////////////////////////////////////////////////
inline bool PrivateMovieViewWinUAP::IsPlaying()
{
    return properties.playing;
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__

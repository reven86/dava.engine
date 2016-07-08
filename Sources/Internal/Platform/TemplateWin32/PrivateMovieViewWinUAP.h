#ifndef __DAVAENGINE_PRIVATEMOVIEVIEWWINUAP_H__
#define __DAVAENGINE_PRIVATEMOVIEVIEWWINUAP_H__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "UI/IMovieViewControl.h"

namespace DAVA
{
class CorePlatformWinUAP;
class PrivateMovieViewWinUAP : public std::enable_shared_from_this<PrivateMovieViewWinUAP>
{
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

private:
    void InstallEventHandlers();
    void PositionMovieView(const Rect& rectInVirtualCoordinates);

    Windows::Storage::Streams::IRandomAccessStream ^ CreateStreamFromFilePath(const FilePath& path) const;
    void OpenMovieFromStream(Windows::Storage::Streams::IRandomAccessStream ^ stream, const OpenMovieParams& params);

private: // MediaElement event handlers
    void OnMediaOpened();
    void OnMediaEnded();
    void OnMediaFailed(Windows::UI::Xaml::ExceptionRoutedEventArgs ^ args);

private:
    CorePlatformWinUAP* core;
    Windows::UI::Xaml::Controls::MediaElement ^ nativeMovieView = nullptr;
    bool visible = true;
    bool movieLoaded = false; // Movie has been successfully loaded and decoded
    bool playRequest = false; // Movie should play after loading as Play() can be invoked earlier than movie has been loaded
    bool moviePlaying = false; // Movie is playing now
};

//////////////////////////////////////////////////////////////////////////
inline bool PrivateMovieViewWinUAP::IsPlaying()
{
    // It seems that framework is client of game but not vice versa
    // Game does not take into account that video playback can take some time after Play() has been called
    // So assume movie is playing under following conditions:
    //  - movie is really playing
    //  - game has called Play() method
    return moviePlaying || playRequest;
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_PRIVATEMOVIEVIEWWINUAP_H__

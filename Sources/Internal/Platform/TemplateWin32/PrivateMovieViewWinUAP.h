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
    void OpenMovie(const WideString& uri, const OpenMovieParams& params);

    void Play();
    void Stop();

    void Pause();
    void Resume();

    bool IsPlaying();

private:
    void InstallEventHandlers();
    void PositionMovieView(const Rect& rectInVirtualCoordinates);

    Windows::Storage::Streams::IRandomAccessStream^ CreateStreamFromUri(const WideString& uriString) const;
    Windows::Storage::Streams::IRandomAccessStream^ CreateStreamFromFilePath(const FilePath& path) const;
    void OpenMovieFromStream(Windows::Storage::Streams::IRandomAccessStream^ stream, const OpenMovieParams& params);

private:    // MediaElement event handlers
    void OnMediaOpened();
    void OnMediaEnded();
    void OnMediaFailed(Windows::UI::Xaml::ExceptionRoutedEventArgs^ args);

private:
    CorePlatformWinUAP* core;
    Windows::UI::Xaml::Controls::MediaElement^ nativeMovieView = nullptr;
    bool visible = true;
    bool movieLoaded = false;   // Movie has been successfully loaded and decoded
    bool playRequest = false;   // Movie should play after loading as Play() can be invoked earlier than movie has been loaded
    bool moviePlaying = false;  // Movie is playing now
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

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
#endif  // __DAVAENGINE_PRIVATEMOVIEVIEWWINUAP_H__

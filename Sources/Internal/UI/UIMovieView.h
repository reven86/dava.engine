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

#ifndef __DAVAENGINE_UIMOVIEVIEW__H__
#define __DAVAENGINE_UIMOVIEVIEW__H__

#include "Base/BaseTypes.h"

#include "UI/UIControl.h"
#include "UI/IMovieViewControl.h"

namespace DAVA
{

// The purpose of UIMovieView class is to display movies.
class UIMovieView : public UIControl
{
public:
    UIMovieView(const Rect &rect = Rect());
protected:
    virtual ~UIMovieView();

public:
    // Open the Movie.
    void OpenMovie(const FilePath& moviePath, const OpenMovieParams& params);

    // Overloaded virtual methods.
    void SetPosition(const Vector2 &position) override;
    void SetSize(const Vector2 &newSize) override;

    void SystemDraw(const UIGeometricData &geometricData) override;

    void WillBecomeVisible() override;
    void WillBecomeInvisible() override;

    UIMovieView *Clone() override;

    // Start/stop the video playback.
    void Play();
    void Stop();

    // Pause/resume the playback.
    void Pause();
    void Resume();

    // Whether the movie is being played?
    bool IsPlaying();

protected:
    // Platform-specific implementation of the Movie Control.
    IMovieViewControl* movieViewControl;
public:
    INTROSPECTION_EXTEND(UIMovieView, UIControl,
        nullptr
        );
};

}   // namespace DAVA

#endif  // __DAVAENGINE_UIMOVIEVIEW__H__

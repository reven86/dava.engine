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

#ifndef __DAVAENGINE_IMOVIEVIEWCONTROL__H__
#define __DAVAENGINE_IMOVIEVIEWCONTROL__H__

#include "FileSystem/FilePath.h"
#include "Math/Rect.h"

namespace DAVA
{

enum eMovieScalingMode
{
    scalingModeNone = 0,        // No scaling
    scalingModeAspectFit,       // Uniform scale until one dimension fits
    scalingModeAspectFill,      // Uniform scale until the movie fills the visible bounds. One dimension may have clipped contents
    scalingModeFill             // Non-uniform scale. Both render dimensions will exactly match the visible bounds
};

struct OpenMovieParams
{
    OpenMovieParams(eMovieScalingMode mode = scalingModeNone)
        : scalingMode(mode)
    {}

    eMovieScalingMode scalingMode;
};

// Common interface for Movie View Controls for different platforms.
class IMovieViewControl
{
public:
    virtual ~IMovieViewControl() {};

    // Initialize the control.
    virtual void Initialize(const Rect& rect) = 0;

    // Position/visibility.
    virtual void SetRect(const Rect& rect) = 0;
    virtual void SetVisible(bool isVisible) = 0;

    // Open the Movie.
    virtual void OpenMovie(const FilePath& moviePath, const OpenMovieParams& params) = 0;

    // Start/stop the video playback.
    virtual void Play() = 0;
    virtual void Stop() = 0;

    // Pause/resume the playback.
    virtual void Pause() = 0;
    virtual void Resume() = 0;

    // Whether the movie is being played?
    virtual bool IsPlaying() = 0;
};

}   // namespace DAVA

#endif //__DAVAENGINE_IMOVIEVIEWCONTROL__H__

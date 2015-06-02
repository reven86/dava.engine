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


#ifndef __DAVAENGINE_MOVIEVIEWCONTROL_IOS_H__
#define __DAVAENGINE_MOVIEVIEWCONTROL_IOS_H__

#include "DAVAEngine.h"
#include "UI/IMovieViewControl.h"

namespace DAVA
{
	
// Movie View Control - iOS implementation.
class MovieViewControl : public IMovieViewControl
{
public:
	MovieViewControl();
	virtual ~MovieViewControl();

	// Initialize the control.
	virtual void Initialize(const Rect& rect);

	// Open the Movie.
	virtual void OpenMovie(const FilePath& moviePath, const OpenMovieParams& params);

	// Position/visibility.
	virtual void SetRect(const Rect& rect);
	virtual void SetVisible(bool isVisible);

	// Start/stop the video playback.
	virtual void Play();
	virtual void Stop();
	
	// Pause/resume the playback.
	virtual void Pause();
	virtual void Resume();
	
	// Whether the movie is being played?
	virtual bool IsPlaying();
	
protected:
	
	// Convert the DAVA Scaling Mode to platform-specific (iOS) one.
	int ConvertScalingModeToPlatform(eMovieScalingMode scalingMode);

private:
	// Pointer to iOS movie player.
	void* moviePlayerController;
};
	
	
};

#endif /* defined(__DAVAENGINE_MOVIEVIEWCONTROL_IOS_H__) */

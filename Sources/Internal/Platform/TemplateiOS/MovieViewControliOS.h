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

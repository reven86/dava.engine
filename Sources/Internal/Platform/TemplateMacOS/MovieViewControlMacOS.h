#ifndef __DAVAENGINE_MOVIEVIEWCONTROL_MACOS_H__
#define __DAVAENGINE_MOVIEVIEWCONTROL_MACOS_H__

#include "DAVAEngine.h"
#include "UI/IMovieViewControl.h"
#include "Functional/SignalBase.h"

namespace DAVA
{
// Movie View Control - MacOS implementation.
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

private:
    void OnAppMinimizedRestored(bool minimized);
    SigConnectionID appMinimizedRestoredConnectionId;

    // Pointer to MacOS video player helper.
    void* moviePlayerHelper;
};
};

#endif /* defined(__DAVAENGINE_MOVIEVIEWCONTROL_MACOS_H__) */

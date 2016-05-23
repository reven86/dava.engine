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
    ~MovieViewControl() override;

    // Initialize the control.
    void Initialize(const Rect& rect) override;

    // Open the Movie.
    void OpenMovie(const FilePath& moviePath, const OpenMovieParams& params) override;

    // Position/visibility.
    void SetRect(const Rect& rect) override;
    void SetVisible(bool isVisible) override;

    // Start/stop the video playback.
    void Play() override;
    void Stop() override;

    // Pause/resume the playback.
    void Pause() override;
    void Resume() override;

    // Whether the movie is being played?
    bool IsPlaying() const override;

private:
    void OnAppMinimizedRestored(bool minimized);
    SigConnectionID appMinimizedRestoredConnectionId;

    // Pointer to MacOS video player helper.
    void* moviePlayerHelper;
};
};

#endif /* defined(__DAVAENGINE_MOVIEVIEWCONTROL_MACOS_H__) */

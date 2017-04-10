#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_MACOS__)
#if !defined(DISABLE_NATIVE_MOVIEVIEW)

#include "UI/IMovieViewControl.h"

namespace DAVA
{
class Window;
// Movie View Control - MacOS implementation.
class MovieViewControl : public IMovieViewControl
{
public:
#if defined(__DAVAENGINE_COREV2__)
    MovieViewControl(Window* w);
#else
    MovieViewControl();
#endif
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
#if defined(__DAVAENGINE_COREV2__)
    void OnWindowVisibilityChanged(Window* w, bool visible);
#else
    void OnAppMinimizedRestored(bool minimized);
#endif

#if defined(__DAVAENGINE_STEAM__)
    bool wasVisible = false;
    void OnSteamOverlayChanged(bool overlayActivated);
#endif

private:
#if defined(__DAVAENGINE_COREV2__)
    Window* window = nullptr;
#endif

    // Pointer to MacOS video player helper.
    void* moviePlayerHelper;
};
} // namespace DAVA

#endif // !DISABLE_NATIVE_MOVIEVIEW
#endif // __DAVAENGINE_MACOS__

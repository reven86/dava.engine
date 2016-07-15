#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "UI/IMovieViewControl.h"

#include "Engine/EngineFwd.h"

namespace DAVA
{
// Movie View Control - iOS implementation.
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
    struct MovieViewObjcBridge;
    std::unique_ptr<MovieViewObjcBridge> bridge;
    
#if defined(__DAVAENGINE_COREV2__)
    Window* window = nullptr;
#endif
};
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__

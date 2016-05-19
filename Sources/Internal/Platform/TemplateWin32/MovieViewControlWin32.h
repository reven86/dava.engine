#pragma once

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN32__)

#include "UI/IMovieViewControl.h"
#include "Base/ScopedPtr.h"

namespace DAVA
{
class FfmpegPlayer;
class Texture;
class UIControlBackground;
class MovieViewControl : public IMovieViewControl
{
public:
    MovieViewControl();
    ~MovieViewControl() override;

    // Initialize the control.
    void Initialize(const Rect& rect) override;

    // Position/visibility.
    void SetRect(const Rect& rect) override;
    void SetVisible(bool isVisible) override;

    // Open the Movie.
    void OpenMovie(const FilePath& moviePath, const OpenMovieParams& params) override;

    // Start/stop the video playback.
    void Play() override;
    void Stop() override;

    // Pause/resume the playback.
    void Pause() override;
    void Resume() override;

    // Whether the movie is being played?
    bool IsPlaying() const override;

    void Update() override;

    void Draw(const class UIGeometricData& parentGeometricData) override;

private:
    std::unique_ptr<FfmpegPlayer> ffmpegPlayer;

    Texture* videoTexture = nullptr;
    ScopedPtr<UIControlBackground> videoBackground;
    Vector<uint8> videoTextureBuffer;
    uint32 textureWidth = 0;
    uint32 textureHeight = 0;
    uint32 textureDataLen = 0;
};
}

#endif

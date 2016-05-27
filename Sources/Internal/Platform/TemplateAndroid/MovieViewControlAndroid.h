#ifndef __DAVAENGINE_MOVIEVIEWCONTROL_ANDROID_H__
#define __DAVAENGINE_MOVIEVIEWCONTROL_ANDROID_H__

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_ANDROID__)

#include "UI/IMovieViewControl.h"
#include "Platform/TemplateAndroid/JniHelpers.h"

namespace DAVA
{
class JniMovieViewControl
{
public:
    JniMovieViewControl(uint32 id);
    void Initialize(const Rect& rect);
    void Uninitialize();

    void SetRect(const Rect& rect);
    void SetVisible(bool isVisible);

    void OpenMovie(const FilePath& moviePath, const OpenMovieParams& params);

    void Play();
    void Stop();
    void Pause();
    void Resume();
    bool IsPlaying() const;

private:
    uint32 id;

    JNI::JavaClass jniMovieViewControl;
    Function<void(jint, jfloat, jfloat, jfloat, jfloat)> initialize;
    Function<void(jint)> uninitialize;
    Function<void(jint, jfloat, jfloat, jfloat, jfloat)> setRect;
    Function<void(jint, jboolean)> setVisible;
    Function<void(jint, jstring, jint)> openMovie;
    Function<void(jint)> play;
    Function<void(jint)> stop;
    Function<void(jint)> pause;
    Function<void(jint)> resume;
    Function<jboolean(jint)> isPlaying;
};

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

private:
    JniMovieViewControl jniMovieViewControl;
};
};

#endif //__DAVAENGINE_ANDROID__

#endif /* defined(__DAVAENGINE_MOVIEVIEWCONTROL_ANDROID_H__) */

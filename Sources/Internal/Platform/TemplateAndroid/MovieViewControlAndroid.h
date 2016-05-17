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
    bool IsPlaying();

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
    virtual ~MovieViewControl();

    // Initialize the control.
    virtual void Initialize(const Rect& rect);

    // Position/visibility.
    virtual void SetRect(const Rect& rect);
    virtual void SetVisible(bool isVisible);

    // Open the Movie.
    virtual void OpenMovie(const FilePath& moviePath, const OpenMovieParams& params);

    // Start/stop the video playback.
    virtual void Play();
    virtual void Stop();

    // Pause/resume the playback.
    virtual void Pause();
    virtual void Resume();

    // Whether the movie is being played?
    virtual bool IsPlaying();

private:
    JniMovieViewControl jniMovieViewControl;
};
};

#endif //__DAVAENGINE_ANDROID__

#endif /* defined(__DAVAENGINE_MOVIEVIEWCONTROL_ANDROID_H__) */

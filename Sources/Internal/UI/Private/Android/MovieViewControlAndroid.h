#pragma once

#if !defined(DISABLE_NATIVE_MOVIEVIEW)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Android/JNIBridge.h"

#include "UI/IMovieViewControl.h"

namespace DAVA
{
class Rect;
class FilePath;
class UIMovieView;

class MovieViewControl : public IMovieViewControl,
                         public std::enable_shared_from_this<MovieViewControl>
{
    enum eAction
    {
        ACTION_PLAY = 0,
        ACTION_PAUSE,
        ACTION_RESUME,
        ACTION_STOP
    };

public:
    MovieViewControl(Window* w);
    ~MovieViewControl() override;

    void Initialize(const Rect& rect) override;
    void OwnerIsDying() override;

    void SetRect(const Rect& rect) override;
    void SetVisible(bool isVisible) override;

    void OpenMovie(const FilePath& moviePath, const OpenMovieParams& params) override;

    void Play() override;
    void Stop() override;

    void Pause() override;
    void Resume() override;

    bool IsPlaying() const override;

    void Update() override;

private:
    Window* window = nullptr;
    jobject javaMovieView = nullptr;

    std::unique_ptr<JNI::JavaClass> movieViewJavaClass;
    Function<void(jobject)> release;
    Function<void(jobject, jfloat, jfloat, jfloat, jfloat)> setRect;
    Function<void(jobject, jboolean)> setVisible;
    Function<void(jobject, jstring, jint)> openMovie;
    Function<void(jobject, jint)> doAction;
    Function<void(jobject)> update;
};

} // namespace DAVA

#else // __DAVAENGINE_COREV2__

#include "UI/IMovieViewControl.h"
#include "Engine/Android/JNIBridge.h"

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
} // namespace DAVA

#endif // !__DAVAENGINE_COREV2__
#endif //__DAVAENGINE_ANDROID__
#endif // !DISABLE_NATIVE_MOVIEVIEW

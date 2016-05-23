#include "MovieViewControlAndroid.h"

namespace DAVA
{
JniMovieViewControl::JniMovieViewControl(uint32 id)
    : jniMovieViewControl("com/dava/framework/JNIMovieViewControl")
{
    this->id = id;

    initialize = jniMovieViewControl.GetStaticMethod<void, jint, jfloat, jfloat, jfloat, jfloat>("Initialize");
    uninitialize = jniMovieViewControl.GetStaticMethod<void, jint>("Uninitialize");
    setRect = jniMovieViewControl.GetStaticMethod<void, jint, jfloat, jfloat, jfloat, jfloat>("SetRect");
    setVisible = jniMovieViewControl.GetStaticMethod<void, jint, jboolean>("SetVisible");
    openMovie = jniMovieViewControl.GetStaticMethod<void, jint, jstring, jint>("OpenMovie");
    play = jniMovieViewControl.GetStaticMethod<void, jint>("Play");
    stop = jniMovieViewControl.GetStaticMethod<void, jint>("Stop");
    pause = jniMovieViewControl.GetStaticMethod<void, jint>("Pause");
    resume = jniMovieViewControl.GetStaticMethod<void, jint>("Resume");
    isPlaying = jniMovieViewControl.GetStaticMethod<jboolean, jint>("IsPlaying");
}

void JniMovieViewControl::Initialize(const Rect& _rect)
{
    Rect rect = JNI::V2I(_rect);

    rect.dx = std::max(0.0f, rect.dx);
    rect.dy = std::max(0.0f, rect.dy);

    initialize(id, rect.x, rect.y, rect.dx, rect.dy);
}

void JniMovieViewControl::Uninitialize()
{
    uninitialize(id);
}

void JniMovieViewControl::SetRect(const Rect& _rect)
{
    Rect rect = JNI::V2I(_rect);

    rect.dx = std::max(0.0f, rect.dx);
    rect.dy = std::max(0.0f, rect.dy);

    setRect(id, rect.x, rect.y, rect.dx, rect.dy);
}

void JniMovieViewControl::SetVisible(bool isVisible)
{
    setVisible(id, isVisible);
}

void JniMovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    JNIEnv* env = JNI::GetEnv();
    jstring jMoviePath = env->NewStringUTF(moviePath.GetAbsolutePathname().c_str());

    openMovie(id, jMoviePath, params.scalingMode);

    env->DeleteLocalRef(jMoviePath);
}

void JniMovieViewControl::Play()
{
    play(id);
}

void JniMovieViewControl::Stop()
{
    stop(id);
}

void JniMovieViewControl::Pause()
{
    pause(id);
}

void JniMovieViewControl::Resume()
{
    resume(id);
}

bool JniMovieViewControl::IsPlaying() const
{
    return isPlaying(id);
}

MovieViewControl::MovieViewControl()
    :
    jniMovieViewControl((uint32) this)
{
}

MovieViewControl::~MovieViewControl()
{
    jniMovieViewControl.Uninitialize();
}

void MovieViewControl::Initialize(const Rect& rect)
{
    jniMovieViewControl.Initialize(rect);
}

void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    jniMovieViewControl.OpenMovie(moviePath, params);
}

void MovieViewControl::SetRect(const Rect& rect)
{
    jniMovieViewControl.SetRect(rect);
}

void MovieViewControl::SetVisible(bool isVisible)
{
    jniMovieViewControl.SetVisible(isVisible);
}

void MovieViewControl::Play()
{
    jniMovieViewControl.Play();
}

void MovieViewControl::Stop()
{
    jniMovieViewControl.Stop();
}

void MovieViewControl::Pause()
{
    jniMovieViewControl.Pause();
}

void MovieViewControl::Resume()
{
    jniMovieViewControl.Resume();
}

bool MovieViewControl::IsPlaying() const
{
    return jniMovieViewControl.IsPlaying();
}
}

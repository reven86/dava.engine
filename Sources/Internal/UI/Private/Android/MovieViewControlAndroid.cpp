#if !defined(DISABLE_NATIVE_MOVIEVIEW)

#include "UI/Private/Android/MovieViewControlAndroid.h"
#include "UI/UIControlSystem.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

#include "Math/Rect.h"
#include "Utils/Utils.h"
#include "Logger/Logger.h"

#include "Engine/Engine.h"
#include "Engine/Window.h"

extern "C"
{

JNIEXPORT void JNICALL Java_com_dava_engine_DavaMovieView_nativeReleaseWeakPtr(JNIEnv* env, jclass jclazz, jlong backendPointer)
{
    using DAVA::MovieViewControl;

    // Postpone deleting in case some other jobs are posted to main thread
    DAVA::RunOnMainThreadAsync([backendPointer]() {
        std::weak_ptr<MovieViewControl>* weak = reinterpret_cast<std::weak_ptr<MovieViewControl>*>(static_cast<uintptr_t>(backendPointer));
        delete weak;
    });
}

} // extern "C"

namespace DAVA
{
MovieViewControl::MovieViewControl(Window* w)
    : window(w)
{
}

MovieViewControl::~MovieViewControl() = default;

void MovieViewControl::Initialize(const Rect& rect)
{
    try
    {
        movieViewJavaClass.reset(new JNI::JavaClass("com/dava/engine/DavaMovieView"));
        release = movieViewJavaClass->GetMethod<void>("release");
        setRect = movieViewJavaClass->GetMethod<void, jfloat, jfloat, jfloat, jfloat>("setRect");
        setVisible = movieViewJavaClass->GetMethod<void, jboolean>("setVisible");
        openMovie = movieViewJavaClass->GetMethod<void, jstring, jint>("openMovie");
        doAction = movieViewJavaClass->GetMethod<void, jint>("doAction");
        isPlaying = movieViewJavaClass->GetMethod<jboolean>("isPlaying");
        update = movieViewJavaClass->GetMethod<void>("update");
    }
    catch (const JNI::Exception& e)
    {
        Logger::Error("[MovieViewControl] failed to init java bridge: %s", e.what());
        DVASSERT(false, e.what());
        return;
    }

    std::weak_ptr<MovieViewControl>* selfWeakPtr = new std::weak_ptr<MovieViewControl>(shared_from_this());
    jobject obj = PlatformApi::Android::CreateNativeControl(window, "com.dava.engine.DavaMovieView", selfWeakPtr);
    if (obj != nullptr)
    {
        JNIEnv* env = JNI::GetEnv();
        javaMovieView = env->NewGlobalRef(obj);
        env->DeleteLocalRef(obj);
        SetRect(rect);
    }
    else
    {
        delete selfWeakPtr;
        Logger::Error("[MovieViewControl] failed to create java movieview");
    }
}

void MovieViewControl::OwnerIsDying()
{
    if (javaMovieView != nullptr)
    {
        release(javaMovieView);
        JNI::GetEnv()->DeleteGlobalRef(javaMovieView);
        javaMovieView = nullptr;
    }
}

void MovieViewControl::SetRect(const Rect& rect)
{
    if (javaMovieView != nullptr)
    {
        Rect rc = UIControlSystem::Instance()->vcs->ConvertVirtualToInput(rect);
        rc.dx = std::max(0.0f, rc.dx);
        rc.dy = std::max(0.0f, rc.dy);

        setRect(javaMovieView, rc.x, rc.y, rc.dx, rc.dy);
    }
}

void MovieViewControl::SetVisible(bool visible)
{
    if (javaMovieView != nullptr)
    {
        setVisible(javaMovieView, visible ? JNI_TRUE : JNI_FALSE);
    }
}

void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    if (javaMovieView != nullptr)
    {
        JNIEnv* env = JNI::GetEnv();
        jstring jpath = JNI::StringToJavaString(moviePath.GetAbsolutePathname(), env);
        openMovie(javaMovieView, jpath, static_cast<jint>(params.scalingMode));
        env->DeleteLocalRef(jpath);
    }
}

void MovieViewControl::Play()
{
    if (javaMovieView != nullptr)
    {
        doAction(javaMovieView, ACTION_PLAY);
    }
}

void MovieViewControl::Stop()
{
    if (javaMovieView != nullptr)
    {
        doAction(javaMovieView, ACTION_STOP);
    }
}

void MovieViewControl::Pause()
{
    if (javaMovieView != nullptr)
    {
        doAction(javaMovieView, ACTION_PAUSE);
    }
}

void MovieViewControl::Resume()
{
    if (javaMovieView != nullptr)
    {
        doAction(javaMovieView, ACTION_RESUME);
    }
}

bool MovieViewControl::IsPlaying() const
{
    if (javaMovieView != nullptr)
    {
        return isPlaying(javaMovieView) == JNI_TRUE;
    }
    return false;
}

void MovieViewControl::Update()
{
    if (javaMovieView != nullptr)
    {
        update(javaMovieView);
    }
}

} // namespace DAVA

#else // __DAVAENGINE_COREV2__

namespace DAVA
{
JniMovieViewControl::JniMovieViewControl(uintptr_t id)
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
    Rect rect = UIControlSystem::Instance()->vcs->ConvertVirtualToInput(_rect);

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
    Rect rect = UIControlSystem::Instance()->vcs->ConvertVirtualToInput(_rect);

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
    jniMovieViewControl(reinterpret_cast<uintptr_t>(this))
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

#endif // !__DAVAENGINE_COREV2__
#endif //__DAVAENGINE_ANDROID__
#endif // !DISABLE_NATIVE_MOVIEVIEW

#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Android/AndroidBridge.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Android/PlatformCoreAndroid.h"
#include "Engine/Private/Android/Window/WindowBackendAndroid.h"

#include "Concurrency/Thread.h"
#include "Logger/Logger.h"
#include "Debug/Backtrace.h"

extern DAVA::Private::AndroidBridge* androidBridge;

extern "C"
{

JNIEXPORT jlong JNICALL Java_com_dava_engine_DavaActivity_nativeOnCreate(JNIEnv* env, jclass jclazz)
{
    DAVA::Private::WindowBackend* wbackend = androidBridge->OnCreateActivity();
    return static_cast<jlong>(reinterpret_cast<uintptr_t>(wbackend));
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaActivity_nativeOnStart(JNIEnv* env, jclass jclazz)
{
    androidBridge->OnStartActivity();
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaActivity_nativeOnResume(JNIEnv* env, jclass jclazz)
{
    androidBridge->OnResumeActivity();
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaActivity_nativeOnPause(JNIEnv* env, jclass jclazz)
{
    androidBridge->OnPauseActivity();
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaActivity_nativeOnStop(JNIEnv* env, jclass jclazz)
{
    androidBridge->OnStopActivity();
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaActivity_nativeOnDestroy(JNIEnv* env, jclass jclazz)
{
    androidBridge->OnDestroyActivity();
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaActivity_nativeGameThread(JNIEnv* env, jclass jcls)
{
    androidBridge->GameThread();
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurface_nativeSurfaceOnResume(JNIEnv* env, jclass jclazz, jlong windowBackendPointer)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    androidBridge->SurfaceResume(wbackend);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurface_nativeSurfaceOnPause(JNIEnv* env, jclass jclazz, jlong windowBackendPointer)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    androidBridge->SurfacePause(wbackend);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurface_nativeSurfaceChanged(JNIEnv* env, jclass jclazz, jlong windowBackendPointer, jobject surface, jint width, jint height)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    androidBridge->SurfaceChanged(wbackend, env, surface, width, height);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurface_nativeSurfaceDestroyed(JNIEnv* env, jclass jclazz, jlong windowBackendPointer)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    androidBridge->SurfaceDestroyed(wbackend, env);
}

} // extern "C"

namespace DAVA
{
namespace Private
{
AndroidBridge::AndroidBridge(JavaVM* jvm)
    : javaVM(jvm)
{
}

WindowBackend* AndroidBridge::OnCreateActivity()
{
    EngineBackend* prev = engineBackend;
    engineBackend = new EngineBackend(Vector<String>());

    return core->OnCreate();
}

void AndroidBridge::OnStartActivity()
{
    core->OnStart();
}

void AndroidBridge::OnResumeActivity()
{
    core->OnResume();
}

void AndroidBridge::OnPauseActivity()
{
    core->OnPause();
}

void AndroidBridge::OnStopActivity()
{
    core->OnStop();
}

void AndroidBridge::OnDestroyActivity()
{
    core->OnDestroy();

    delete engineBackend;
    engineBackend = nullptr;
    core = nullptr;
}

void AndroidBridge::GameThread()
{
    core->GameThread();
}

void AndroidBridge::SurfaceResume(WindowBackend* wbackend)
{
    wbackend->OnResume();
}

void AndroidBridge::SurfacePause(WindowBackend* wbackend)
{
    wbackend->OnPause();
}

void AndroidBridge::SurfaceChanged(WindowBackend* wbackend, JNIEnv* env, jobject surface, int width, int height)
{
    wbackend->SurfaceChanged(env, surface, width, height);
}

void AndroidBridge::SurfaceDestroyed(WindowBackend* wbackend, JNIEnv* env)
{
    wbackend->SurfaceDestroyed();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__

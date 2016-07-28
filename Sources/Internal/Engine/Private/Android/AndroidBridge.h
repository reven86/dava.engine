#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)

#include <jni.h>

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
namespace Private
{
struct AndroidBridge final
{
    AndroidBridge(JavaVM* jvm);

    WindowBackend* OnCreateActivity();
    void OnStartActivity();
    void OnResumeActivity();
    void OnPauseActivity();
    void OnStopActivity();
    void OnDestroyActivity();

    void GameThread();

    void SurfaceResume(WindowBackend* wbackend);
    void SurfacePause(WindowBackend* wbackend);
    void SurfaceChanged(WindowBackend* wbackend, JNIEnv* env, jobject surface, int width, int height);
    void SurfaceDestroyed(WindowBackend* wbackend, JNIEnv* env);

    JavaVM* javaVM = nullptr;
    EngineBackend* engineBackend = nullptr;
    PlatformCore* core = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__

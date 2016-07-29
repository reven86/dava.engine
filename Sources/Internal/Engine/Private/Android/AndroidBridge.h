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

    static JavaVM* GetJavaVM();
    static JNIEnv* GetEnv();
    static bool AttachCurrentThreadToJavaVM();
    static bool DetachCurrentThreadFromJavaVM();

    static const String& GetExternalDocumentsDir();
    static const String& GetInternalDocumentsDir();
    static const String& GetApplicatiionPath();
    static const String& GetPackageName();

    static String JavaStringToString(jstring string, JNIEnv* jniEnv = nullptr);
    static WideString JavaStringToWideString(jstring string, JNIEnv* jniEnv = nullptr);
    static jstring WideStringToJavaString(const WideString& string, JNIEnv* jniEnv = nullptr);

    void OnInitEngine(String externalFilesDir,
                      String internalFilesDir,
                      String sourceDir,
                      String apkName,
                      String cmdline);
    WindowBackend* OnCreateActivity();
    void OnStartActivity();
    void OnResumeActivity();
    void OnPauseActivity();
    void OnStopActivity();
    void OnDestroyActivity();
    void OnTermEngine();

    void GameThread();

    void SurfaceResume(WindowBackend* wbackend);
    void SurfacePause(WindowBackend* wbackend);
    void SurfaceChanged(WindowBackend* wbackend, JNIEnv* env, jobject surface, int width, int height);
    void SurfaceDestroyed(WindowBackend* wbackend, JNIEnv* env);

    JavaVM* javaVM = nullptr;
    EngineBackend* engineBackend = nullptr;
    PlatformCore* core = nullptr;

    String externalDocumentsDir;
    String internalDocumentsDir;
    String appPath;
    String packageName;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__

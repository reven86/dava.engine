#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

#include <jni.h>
#include <android/log.h>

#include "Engine/Private/EnginePrivateFwd.h"

#define ANDROID_LOG_DEBUG(...) __android_log_print(ANDROID_LOG_DEBUG, "DAVA", __VA_ARGS__)
#define ANDROID_LOG_INFO(...) __android_log_print(ANDROID_LOG_INFO, "DAVA", __VA_ARGS__)
#define ANDROID_LOG_WARN(...) __android_log_print(ANDROID_LOG_WARN, "DAVA", __VA_ARGS__)
#define ANDROID_LOG_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, "DAVA", __VA_ARGS__)
#define ANDROID_LOG_FATAL(...) __android_log_print(ANDROID_LOG_FATAL, "DAVA", __VA_ARGS__)

namespace DAVA
{
namespace Private
{
struct AndroidBridge final
{
    AndroidBridge(JavaVM* jvm);

    void InitializeJNI(JNIEnv* env);

    static JavaVM* GetJavaVM();
    static JNIEnv* GetEnv();
    static bool AttachCurrentThreadToJavaVM();
    static bool DetachCurrentThreadFromJavaVM();
    static void PostQuitToActivity();
    static jclass LoadJavaClass(JNIEnv* env, const char8* className, bool throwJniException);
    static String toString(JNIEnv* env, jobject object);

    static const String& GetExternalDocumentsDir();
    static const String& GetInternalDocumentsDir();
    static const String& GetApplicatiionPath();
    static const String& GetPackageName();

    static void AttachPlatformCore(PlatformCore* platformCore);

    void InitializeEngine(String externalFilesDir,
                          String internalFilesDir,
                          String sourceDir,
                          String apkName,
                          String cmdline);
    void ShutdownEngine();

    WindowBackend* ActivityOnCreate(JNIEnv* env, jobject activityInstance);
    void ActivityOnResume();
    void ActivityOnPause();
    void ActivityOnDestroy(JNIEnv* env);

    void GameThread();

    void SurfaceViewOnResume(WindowBackend* wbackend);
    void SurfaceViewOnPause(WindowBackend* wbackend);
    void SurfaceViewOnSurfaceCreated(WindowBackend* wbackend, JNIEnv* env, jobject jsurfaceView);
    void SurfaceViewOnSurfaceChanged(WindowBackend* wbackend, JNIEnv* env, jobject surface, int32 width, int32 height);
    void SurfaceViewOnSurfaceDestroyed(WindowBackend* wbackend, JNIEnv* env);
    void SurfaceViewOnProcessProperties(WindowBackend* wbackend);
    void SurfaceViewOnTouch(WindowBackend* wbackend, int32 action, int32 touchId, float32 x, float32 y);

    JavaVM* javaVM = nullptr;
    jobject classLoader = nullptr; // Cached instance of ClassLoader
    jmethodID methodClassLoader_loadClass = nullptr; // ClassLoader.loadClass method
    jmethodID methodObject_toString = nullptr; // Object.toString method

    jobject activity = nullptr; // Reference to DavaActivity instance
    jmethodID methodDavaActivity_postFinish = nullptr; // DavaActivity.postFinish method

    EngineBackend* engineBackend = nullptr;
    PlatformCore* core = nullptr;

    String externalDocumentsDir;
    String internalDocumentsDir;
    String appPath;
    String packageName;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__

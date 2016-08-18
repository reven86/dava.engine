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
    static bool HandleJavaException(JNIEnv* env);

    static const String& GetExternalDocumentsDir();
    static const String& GetInternalDocumentsDir();
    static const String& GetApplicatiionPath();
    static const String& GetPackageName();

    static String JavaStringToString(jstring string, JNIEnv* jniEnv = nullptr);
    static WideString JavaStringToWideString(jstring string, JNIEnv* jniEnv = nullptr);
    static jstring WideStringToJavaString(const WideString& string, JNIEnv* jniEnv = nullptr);

    static void AttachPlatformCore(PlatformCore* platformCore);

    void InitializeEngine(String externalFilesDir,
                          String internalFilesDir,
                          String sourceDir,
                          String apkName,
                          String cmdline);
    void ShutdownEngine();

    WindowBackend* ActivityOnCreate();
    void ActivityOnResume();
    void ActivityOnPause();
    void ActivityOnDestroy();

    void GameThread();

    void SurfaceViewOnResume(WindowBackend* wbackend);
    void SurfaceViewOnPause(WindowBackend* wbackend);
    void SurfaceViewOnSurfaceCreated(WindowBackend* wbackend, JNIEnv* env, jobject jsurfaceView);
    void SurfaceViewOnSurfaceChanged(WindowBackend* wbackend, JNIEnv* env, jobject surface, int32 width, int32 height);
    void SurfaceViewOnSurfaceDestroyed(WindowBackend* wbackend, JNIEnv* env);
    void SurfaceViewOnTouch(WindowBackend* wbackend, int32 action, int32 touchId, float32 x, float32 y);

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

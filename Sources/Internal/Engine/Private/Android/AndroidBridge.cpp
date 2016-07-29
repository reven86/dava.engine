#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Android/AndroidBridge.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/CommandArgs.h"
#include "Engine/Private/Android/PlatformCoreAndroid.h"
#include "Engine/Private/Android/Window/WindowBackendAndroid.h"

#include "Concurrency/Thread.h"
#include "Logger/Logger.h"
#include "Utils/UTF8Utils.h"

#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "DAVA", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "DAVA", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, "DAVA", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "DAVA", __VA_ARGS__)

extern DAVA::Private::AndroidBridge* androidBridge;

extern "C"
{

JNIEXPORT void JNICALL Java_com_dava_engine_DavaActivity_nativeInitEngine(JNIEnv* env,
                                                                          jclass jclazz,
                                                                          jstring externalFilesDir,
                                                                          jstring internalFilesDir,
                                                                          jstring sourceDir,
                                                                          jstring packageName,
                                                                          jstring cmdline)
{
    using DAVA::Private::AndroidBridge;
    androidBridge->OnInitEngine(AndroidBridge::JavaStringToString(externalFilesDir),
                                AndroidBridge::JavaStringToString(internalFilesDir),
                                AndroidBridge::JavaStringToString(sourceDir),
                                AndroidBridge::JavaStringToString(packageName),
                                AndroidBridge::JavaStringToString(cmdline));
}

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

JNIEXPORT void JNICALL Java_com_dava_engine_DavaActivity_nativeTermEngine(JNIEnv* env, jclass jclazz)
{
    androidBridge->OnTermEngine();
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

void AndroidBridge::OnInitEngine(String externalFilesDir,
                                 String internalFilesDir,
                                 String sourceDir,
                                 String apkName,
                                 String cmdline)
{
    if (engineBackend != nullptr)
    {
        LOGE("============================= AndroidBridge::OnInitEngine: engineBackend is not null !!!! ========================== ");
        abort();
    }
    Vector<String> cmdargs = GetCommandArgs(cmdline);
    engineBackend = new EngineBackend(cmdargs);

    externalDocumentsDir = std::move(externalFilesDir);
    internalDocumentsDir = std::move(internalFilesDir);
    appPath = std::move(sourceDir);
    packageName = std::move(apkName);

    Logger::Error("***************************** externalDocumentsDir='%s'", externalDocumentsDir.c_str());
    Logger::Error("***************************** internalDocumentsDir='%s'", internalDocumentsDir.c_str());
    Logger::Error("***************************** appPath='%s'", appPath.c_str());
    Logger::Error("***************************** packageName='%s'", packageName.c_str());
    Logger::Error("***************************** cmdline='%s'", cmdline.c_str());
}

WindowBackend* AndroidBridge::OnCreateActivity()
{
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
}

void AndroidBridge::OnTermEngine()
{
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

JavaVM* AndroidBridge::GetJavaVM()
{
    return androidBridge->javaVM;
}

JNIEnv* AndroidBridge::GetEnv()
{
    JNIEnv* env = nullptr;
    jint status = androidBridge->javaVM->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    return status == JNI_OK ? env : nullptr;
}

bool AndroidBridge::AttachCurrentThreadToJavaVM()
{
    JNIEnv* env = GetEnv();
    if (env == nullptr)
    {
        jint status = GetJavaVM()->AttachCurrentThread(&env, nullptr);
        return status == JNI_OK;
    }
    return true;
}

bool AndroidBridge::DetachCurrentThreadFromJavaVM()
{
    JNIEnv* env = GetEnv();
    if (env != nullptr)
    {
        jint status = GetJavaVM()->DetachCurrentThread();
        return status == JNI_OK;
    }
    return false;
}

String AndroidBridge::JavaStringToString(jstring string, JNIEnv* jniEnv)
{
    String result;
    if (string != nullptr)
    {
        if (jniEnv == nullptr)
        {
            jniEnv = AndroidBridge::GetEnv();
        }

        if (jniEnv != nullptr)
        {
            // http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/functions.html
            const char* rawString = jniEnv->GetStringUTFChars(string, nullptr);
            if (rawString != nullptr)
            {
                result = rawString;
                jniEnv->ReleaseStringUTFChars(string, rawString);
            }
        }
    }
    return result;
}

WideString AndroidBridge::JavaStringToWideString(jstring string, JNIEnv* jniEnv)
{
    return UTF8Utils::EncodeToWideString(JavaStringToString(string, jniEnv));
}

jstring AndroidBridge::WideStringToJavaString(const WideString& string, JNIEnv* jniEnv)
{
    if (jniEnv == nullptr)
    {
        jniEnv = AndroidBridge::GetEnv();
    }

    if (jniEnv != nullptr)
    {
        return jniEnv->NewStringUTF(UTF8Utils::EncodeToUTF8(string).c_str());
    }
    return nullptr;
}

const String& AndroidBridge::GetExternalDocumentsDir()
{
    return androidBridge->externalDocumentsDir;
}

const String& AndroidBridge::GetInternalDocumentsDir()
{
    return androidBridge->internalDocumentsDir;
}

const String& AndroidBridge::GetApplicatiionPath()
{
    return androidBridge->appPath;
}

const String& AndroidBridge::GetPackageName()
{
    return androidBridge->packageName;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__

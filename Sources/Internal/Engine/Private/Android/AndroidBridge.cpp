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

JNIEXPORT void JNICALL Java_com_dava_engine_DavaActivity_nativeInitializeEngine(JNIEnv* env,
                                                                                jclass jclazz,
                                                                                jstring externalFilesDir,
                                                                                jstring internalFilesDir,
                                                                                jstring sourceDir,
                                                                                jstring packageName,
                                                                                jstring cmdline)
{
    using DAVA::Private::AndroidBridge;
    androidBridge->InitializeEngine(AndroidBridge::JavaStringToString(externalFilesDir, env),
                                    AndroidBridge::JavaStringToString(internalFilesDir, env),
                                    AndroidBridge::JavaStringToString(sourceDir, env),
                                    AndroidBridge::JavaStringToString(packageName, env),
                                    AndroidBridge::JavaStringToString(cmdline, env));
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaActivity_nativeShutdownEngine(JNIEnv* env, jclass jclazz)
{
    androidBridge->ShutdownEngine();
}

JNIEXPORT jlong JNICALL Java_com_dava_engine_DavaActivity_nativeOnCreate(JNIEnv* env, jclass jclazz)
{
    DAVA::Private::WindowBackend* wbackend = androidBridge->ActivityOnCreate();
    return static_cast<jlong>(reinterpret_cast<uintptr_t>(wbackend));
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaActivity_nativeOnResume(JNIEnv* env, jclass jclazz)
{
    androidBridge->ActivityOnResume();
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaActivity_nativeOnPause(JNIEnv* env, jclass jclazz)
{
    androidBridge->ActivityOnPause();
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaActivity_nativeOnDestroy(JNIEnv* env, jclass jclazz)
{
    androidBridge->ActivityOnDestroy();
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaActivity_nativeGameThread(JNIEnv* env, jclass jcls)
{
    androidBridge->GameThread();
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnResume(JNIEnv* env, jclass jclazz, jlong windowBackendPointer)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    androidBridge->SurfaceViewOnResume(wbackend);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnPause(JNIEnv* env, jclass jclazz, jlong windowBackendPointer)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    androidBridge->SurfaceViewOnPause(wbackend);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnSurfaceCreated(JNIEnv* env, jclass jclazz, jlong windowBackendPointer, jobject jsurfaceView)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    androidBridge->SurfaceViewOnSurfaceCreated(wbackend, env, jsurfaceView);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnSurfaceChanged(JNIEnv* env, jclass jclazz, jlong windowBackendPointer, jobject surface, jint width, jint height)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    androidBridge->SurfaceViewOnSurfaceChanged(wbackend, env, surface, width, height);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnSurfaceDestroyed(JNIEnv* env, jclass jclazz, jlong windowBackendPointer)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    androidBridge->SurfaceViewOnSurfaceDestroyed(wbackend, env);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnTouch(JNIEnv* env, jclass jclazz, jlong windowBackendPointer, jint action, jint touchId, jfloat x, jfloat y)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    androidBridge->SurfaceViewOnTouch(wbackend, action, touchId, x, y);
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

void AndroidBridge::AttachPlatformCore(PlatformCore* platformCore)
{
    if (platformCore == nullptr || androidBridge->core != nullptr)
    {
        LOGE("=========== AndroidBridge::AttachPlatformCore: platformCore is already set !!!! ===========");
        abort();
    }
    androidBridge->core = platformCore;
}

void AndroidBridge::InitializeEngine(String externalFilesDir,
                                     String internalFilesDir,
                                     String sourceDir,
                                     String apkName,
                                     String cmdline)
{
    if (engineBackend != nullptr)
    {
        LOGE("=========== AndroidBridge::InitializeEngine: engineBackend is not null !!!! ===========");
        abort();
    }

    Vector<String> cmdargs = GetCommandArgs(cmdline);
    engineBackend = new EngineBackend(cmdargs);

    externalDocumentsDir = std::move(externalFilesDir);
    internalDocumentsDir = std::move(internalFilesDir);
    appPath = std::move(sourceDir);
    packageName = std::move(apkName);

    Logger::FrameworkDebug("=========== externalDocumentsDir='%s'", externalDocumentsDir.c_str());
    Logger::FrameworkDebug("=========== internalDocumentsDir='%s'", internalDocumentsDir.c_str());
    Logger::FrameworkDebug("=========== appPath='%s'", appPath.c_str());
    Logger::FrameworkDebug("=========== packageName='%s'", packageName.c_str());
    Logger::FrameworkDebug("=========== cmdline='%s'", cmdline.c_str());
}

void AndroidBridge::ShutdownEngine()
{
    delete engineBackend;
    engineBackend = nullptr;
    core = nullptr;
}

WindowBackend* AndroidBridge::ActivityOnCreate()
{
    return core->ActivityOnCreate();
}

void AndroidBridge::ActivityOnResume()
{
    core->ActivityOnResume();
}

void AndroidBridge::ActivityOnPause()
{
    core->ActivityOnPause();
}

void AndroidBridge::ActivityOnDestroy()
{
    core->ActivityOnDestroy();
}

void AndroidBridge::GameThread()
{
    core->GameThread();
}

void AndroidBridge::SurfaceViewOnResume(WindowBackend* wbackend)
{
    wbackend->OnResume();
}

void AndroidBridge::SurfaceViewOnPause(WindowBackend* wbackend)
{
    wbackend->OnPause();
}

void AndroidBridge::SurfaceViewOnSurfaceCreated(WindowBackend* wbackend, JNIEnv* env, jobject jsurfaceView)
{
    wbackend->SurfaceCreated(env, jsurfaceView);
}

void AndroidBridge::SurfaceViewOnSurfaceChanged(WindowBackend* wbackend, JNIEnv* env, jobject surface, int32 width, int32 height)
{
    wbackend->SurfaceChanged(env, surface, width, height);
}

void AndroidBridge::SurfaceViewOnSurfaceDestroyed(WindowBackend* wbackend, JNIEnv* env)
{
    wbackend->SurfaceDestroyed();
}

void AndroidBridge::SurfaceViewOnTouch(WindowBackend* wbackend, int32 action, int32 touchId, float32 x, float32 y)
{
    wbackend->OnTouch(action, touchId, x, y);
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

bool AndroidBridge::HandleJavaException(JNIEnv* env)
{
    jthrowable e = env->ExceptionOccurred();
    if (e != nullptr)
    {
#if defined(__DAVAENGINE_DEBUG__)
        env->ExceptionDescribe();
#endif
        env->ExceptionClear();

        // TODO: log java exception description
        //if (androidBridge->jobject_toString != nullptr)
        //{
        //    jstring jstr = (jstring)env->CallObjectMethod(e, androidBridge->jobject_toString);
        //    const char* cstr = env->GetStringUTFChars(jstr, nullptr);
        //    LOGE("[java exception] %s", cstr);
        //    env->ReleaseStringUTFChars(jstr, cstr);
        //}
        return true;
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

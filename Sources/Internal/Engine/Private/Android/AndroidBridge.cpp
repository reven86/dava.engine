#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Android/AndroidBridge.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Android/JNIBridge.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/CommandArgs.h"
#include "Engine/Private/Android/PlatformCoreAndroid.h"
#include "Engine/Private/Android/Window/WindowBackendAndroid.h"

#include "Concurrency/Thread.h"
#include "Logger/Logger.h"
#include "Utils/UTF8Utils.h"

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
    using namespace DAVA::JNI;
    androidBridge->InitializeEngine(JavaStringToString(externalFilesDir, env),
                                    JavaStringToString(internalFilesDir, env),
                                    JavaStringToString(sourceDir, env),
                                    JavaStringToString(packageName, env),
                                    JavaStringToString(cmdline, env));
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaActivity_nativeShutdownEngine(JNIEnv* env, jclass jclazz)
{
    androidBridge->ShutdownEngine();
}

JNIEXPORT jlong JNICALL Java_com_dava_engine_DavaActivity_nativeOnCreate(JNIEnv* env, jclass jclazz, jobject activity)
{
    DAVA::Private::WindowBackend* wbackend = androidBridge->ActivityOnCreate(env, activity);
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
    androidBridge->ActivityOnDestroy(env);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaActivity_nativeGameThread(JNIEnv* env, jclass jcls)
{
    androidBridge->GameThread();
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

void AndroidBridge::InitializeJNI(JNIEnv* env)
{
    ANDROID_LOG_INFO("======================= Initializing JNI...");

    // Get Object.toString method to get string representation of Object
    try
    {
        jclass jclassObject = env->FindClass("java/lang/Object");
        JNI::CheckJavaException(env, true);

        methodObject_toString = env->GetMethodID(jclassObject, "toString", "()Ljava/lang/String;");
        JNI::CheckJavaException(env, true);
    }
    catch (const JNI::Exception& e)
    {
        ANDROID_LOG_ERROR("InitializeJNI: failed to get Object.toString method: %s", e.what());
    }

    // Cache Java ClassLoader
    try
    {
        // Get com.dava.engine.DavaActivity class which will be used to obtain ClassLoader instance
        jclass jclassDavaActivity = env->FindClass("com/dava/engine/DavaActivity");
        JNI::CheckJavaException(env, true);

        methodDavaActivity_postFinish = env->GetMethodID(jclassDavaActivity, "postQuit", "()V");
        JNI::CheckJavaException(env, true);

        // Get java.lang.Class<com.dava.engine.DavaActivity>
        jclass jclassClass = env->GetObjectClass(jclassDavaActivity);
        JNI::CheckJavaException(env, true);

        // Get Class<java.lang.Class>.getClassLoader method
        jmethodID jmethodClass_getClassLoader = env->GetMethodID(jclassClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
        JNI::CheckJavaException(env, true);

        // Obtain ClassLoader instance
        classLoader = env->CallObjectMethod(jclassDavaActivity, jmethodClass_getClassLoader);
        JNI::CheckJavaException(env, true);

        // Get java.lang.ClassLoader class
        jclass jclassClassLoader = env->FindClass("java/lang/ClassLoader");
        JNI::CheckJavaException(env, true);

        // Get ClassLoader.loadClass method
        methodClassLoader_loadClass = env->GetMethodID(jclassClassLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
        JNI::CheckJavaException(env, true);

        // Create new global reference on ClassLoader instance
        // jobject obtained in CallObjectMethod call is a local reference which will be deleted by java
        classLoader = env->NewGlobalRef(classLoader);
        JNI::CheckJavaException(env, true);
    }
    catch (const JNI::Exception& e)
    {
        ANDROID_LOG_FATAL("InitializeJNI: failed to cache ClassLoader instance: %s", e.what());
        env->FatalError("InitializeJNI: failed to cache ClassLoader instance");
    }
}

void AndroidBridge::AttachPlatformCore(PlatformCore* platformCore)
{
    if (platformCore == nullptr || androidBridge->core != nullptr)
    {
        ANDROID_LOG_FATAL("=========== AndroidBridge::AttachPlatformCore: platformCore is already set !!!! ===========");
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
        ANDROID_LOG_FATAL("=========== AndroidBridge::InitializeEngine: engineBackend is not null !!!! ===========");
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

WindowBackend* AndroidBridge::ActivityOnCreate(JNIEnv* env, jobject activityInstance)
{
    activity = env->NewGlobalRef(activityInstance);
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

void AndroidBridge::ActivityOnDestroy(JNIEnv* env)
{
    core->ActivityOnDestroy();

    env->DeleteGlobalRef(activity);
    activity = nullptr;
}

void AndroidBridge::GameThread()
{
    core->GameThread();
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

void AndroidBridge::PostQuitToActivity()
{
    try
    {
        JNIEnv* env = GetEnv();
        env->CallVoidMethod(androidBridge->activity, androidBridge->methodDavaActivity_postFinish);
        JNI::CheckJavaException(env, true);
    }
    catch (const JNI::Exception& e)
    {
        ANDROID_LOG_ERROR("postQuit call failed: %s", e.what());
    }
}

jclass AndroidBridge::LoadJavaClass(JNIEnv* env, const char8* className, bool throwJniException)
{
    jstring name = JNI::CStrToJavaString(className);
    if (name != nullptr)
    {
        jobject obj = env->CallObjectMethod(androidBridge->classLoader, androidBridge->methodClassLoader_loadClass, name);
        env->DeleteLocalRef(name);
        if (obj != nullptr)
        {
            jclass result = static_cast<jclass>(env->NewGlobalRef(obj));
            env->DeleteLocalRef(obj);
            return result;
        }
        JNI::CheckJavaException(env, throwJniException);
    }
    return nullptr;
}

String AndroidBridge::toString(JNIEnv* env, jobject object)
{
    String result;
    if (androidBridge->methodObject_toString != nullptr && object != nullptr)
    {
        jstring jstr = static_cast<jstring>(env->CallObjectMethod(object, androidBridge->methodObject_toString));
        JNI::CheckJavaException(env, false);
        result = JNI::JavaStringToString(jstr, env);
    }
    return result;
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

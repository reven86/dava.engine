#include "JniHelpers.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Job/JobManager.h"

jstringArray::jstringArray(const jobjectArray& arr)
{
    obj = arr;
}

namespace DAVA
{
namespace JNI
{
JNIEnv* GetEnv()
{
    JNIEnv* env;
    JavaVM* vm = GetJVM();

    if (nullptr == vm || JNI_EDETACHED == vm->GetEnv((void**)&env, JNI_VERSION_1_6))
    {
        Logger::Error("runtime_error(Thread is not attached to JNI)");
    }
    DVASSERT(nullptr != env);
    return env;
}

void AttachCurrentThreadToJVM()
{
    if (true == Thread::IsMainThread())
        return;

    JavaVM* vm = GetJVM();
    JNIEnv* env;

    if (JNI_EDETACHED == vm->GetEnv((void**)&env, JNI_VERSION_1_6))
    {
        if (vm->AttachCurrentThread(&env, NULL) != 0)
            Logger::Error("runtime_error(Could not attach current thread to JNI)");
    }
}

void DetachCurrentThreadFromJVM()
{
    if (true == Thread::IsMainThread())
        return;

    JavaVM* vm = GetJVM();
    JNIEnv* env;
    if (JNI_OK == vm->GetEnv((void**)&env, JNI_VERSION_1_6))
    {
        if (0 != vm->DetachCurrentThread())
            Logger::Error("runtime_error(Could not detach current thread from JNI)");
    }
}

Rect V2I(const Rect& srcRect)
{
    return VirtualCoordinatesSystem::Instance()->ConvertVirtualToInput(srcRect);
}

DAVA::String ToString(const jstring jniString)
{
    DAVA::String result;

    if (jniString == nullptr)
    {
        LOGE("nullptr in jniString file %s(%d)", __FILE__, __LINE__);
        return result;
    }

    JNIEnv* env = GetEnv();
    if (env == nullptr)
    {
        LOGE("nullptr in jniString file %s(%d)", __FILE__, __LINE__);
        return result;
    }

    // http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/functions.html
    const char* utf8CString = env->GetStringUTFChars(jniString, nullptr);

    if (utf8CString != nullptr)
    {
        result.assign(utf8CString);
        env->ReleaseStringUTFChars(jniString, utf8CString);
    }
    else
    {
        LOGE("Can't create utf-string from jniString file %s(%d)", __FILE__, __LINE__);
    }

    return result;
}

DAVA::WideString ToWideString(const jstring jniString)
{
    DAVA::String utf8 = ToString(jniString);
    return DAVA::UTF8Utils::EncodeToWideString(utf8);
}

jstring ToJNIString(const DAVA::WideString& string)
{
    JNIEnv* env = GetEnv();
    DVASSERT(env);

    String utf8 = DAVA::UTF8Utils::EncodeToUTF8(string);

    return env->NewStringUTF(utf8.c_str());
}

JavaClass::JavaClass(const String& className)
    : javaClass(NULL)
{
    DVASSERT(!className.empty());
    name = className;

    Function<void(String)> findJClass(this, &JavaClass::FindJavaClass);
    auto findJClassName = Bind(findJClass, name);
    uint32 jobId = JobManager::Instance()->CreateMainJob(findJClassName);
    JobManager::Instance()->WaitMainJobID(jobId);
}

JavaClass::~JavaClass()
{
    GetEnv()->DeleteGlobalRef(javaClass);
}

void JavaClass::FindJavaClass(String name)
{
    DVASSERT(Thread::IsMainThread());

    JNIEnv* env = GetEnv();

    jclass foundLocalRefClass = env->FindClass(name.c_str());
    DAVA_JNI_EXCEPTION_CHECK

    if (NULL == foundLocalRefClass)
    {
        javaClass = NULL;
        return;
    }
    javaClass = static_cast<jclass>(env->NewGlobalRef(foundLocalRefClass));
    env->DeleteLocalRef(foundLocalRefClass);
}
}
}
#endif

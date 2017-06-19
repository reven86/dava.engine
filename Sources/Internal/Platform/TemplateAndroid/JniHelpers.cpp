#include "Platform/TemplateAndroid/JniHelpers.h"

#if defined(__DAVAENGINE_ANDROID__)
#if !defined(__DAVAENGINE_COREV2__)

#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "UI/UIControlSystem.h"
#include "Job/JobManager.h"
#include "Utils/UTF8Utils.h"

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

    if (nullptr == vm || JNI_EDETACHED == vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6))
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

    if (JNI_EDETACHED == vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6))
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
    if (JNI_OK == vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6))
    {
        if (0 != vm->DetachCurrentThread())
            Logger::Error("runtime_error(Could not detach current thread from JNI)");
    }
}

Rect V2I(const Rect& srcRect)
{
    return UIControlSystem::Instance()->vcs->ConvertVirtualToInput(srcRect);
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

jobject JavaClass::classLoader = nullptr;
jmethodID JavaClass::jmethod_ClassLoader_findClass = nullptr;

void JavaClass::Initialize()
{
    JNIEnv* env = JNI::GetEnv();

    LocalRef<jclass> jclass_JNIActivity = env->FindClass("com/dava/framework/JNIActivity");
    if (jclass_JNIActivity == nullptr)
    {
        DAVA_JNI_EXCEPTION_CHECK();
        abort();
    }

    LocalRef<jclass> jclass_Class = env->GetObjectClass(jclass_JNIActivity);
    if (jclass_Class == nullptr)
    {
        DAVA_JNI_EXCEPTION_CHECK();
        abort();
    }

    LocalRef<jclass> jclass_ClassLoader = env->FindClass("java/lang/ClassLoader");
    if (jclass_ClassLoader == nullptr)
    {
        DAVA_JNI_EXCEPTION_CHECK();
        abort();
    }

    jmethodID jmethod_Class_getClassLoader = env->GetMethodID(jclass_Class, "getClassLoader", "()Ljava/lang/ClassLoader;");
    if (jmethod_Class_getClassLoader == nullptr)
    {
        DAVA_JNI_EXCEPTION_CHECK();
        abort();
    }

    LocalRef<jobject> classLoader1 = env->CallObjectMethod(jclass_JNIActivity, jmethod_Class_getClassLoader);
    if (classLoader1 == nullptr)
    {
        DAVA_JNI_EXCEPTION_CHECK();
        abort();
    }

    classLoader = env->NewGlobalRef(classLoader1);
    if (classLoader == nullptr)
    {
        DAVA_JNI_EXCEPTION_CHECK();
        abort();
    }

    jmethod_ClassLoader_findClass = env->GetMethodID(jclass_ClassLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    if (jmethod_ClassLoader_findClass == nullptr)
    {
        DAVA_JNI_EXCEPTION_CHECK();
        abort();
    }
}

JavaClass::JavaClass(const String& className)
    : javaClass(NULL)
{
    DVASSERT(!className.empty());
    name = className;
    FindJavaClass();
}

JavaClass::JavaClass(const JavaClass& copy)
    : name(copy.name)
{
    javaClass = copy.javaClass ? static_cast<jclass>(JNI::GetEnv()->NewGlobalRef(copy.javaClass)) : nullptr;
}

JavaClass::~JavaClass()
{
    if (javaClass != nullptr)
    {
        GetEnv()->DeleteGlobalRef(javaClass);
    }
}

JavaClass& JavaClass::operator=(const JavaClass& other)
{
    name = other.name;
    if (javaClass != nullptr)
    {
        GetEnv()->DeleteGlobalRef(javaClass);
    }
    javaClass = other.javaClass ? static_cast<jclass>(JNI::GetEnv()->NewGlobalRef(other.javaClass)) : nullptr;
    return *this;
}

void JavaClass::FindJavaClass()
{
    JNIEnv* env = GetEnv();

    jstring className = env->NewStringUTF(name.c_str());
    jclass foundLocalRefClass = static_cast<jclass>(env->CallObjectMethod(classLoader, jmethod_ClassLoader_findClass, className));
    env->DeleteLocalRef(className);

    DAVA_JNI_EXCEPTION_CHECK();

    if (nullptr == foundLocalRefClass)
    {
        Logger::Error("FindJavaClass error: %s not found", name.c_str());
        javaClass = NULL;
        return;
    }
    javaClass = static_cast<jclass>(env->NewGlobalRef(foundLocalRefClass));
    env->DeleteLocalRef(foundLocalRefClass);
}
}
}

#endif // !__DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__

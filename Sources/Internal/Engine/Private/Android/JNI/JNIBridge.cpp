#include "Engine/Android/JNIBridge.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Android/AndroidBridge.h"

#include "Utils/UTF8Utils.h"
#include "Logger/Logger.h"

namespace DAVA
{
namespace JNI
{
JNIEnv* GetEnv(bool abortIfNotAttachedToJVM)
{
    JNIEnv* env = Private::AndroidBridge::GetEnv();
    if (env == nullptr)
    {
        ANDROID_LOG_FATAL("Thread is not attached to Java VM");
        if (abortIfNotAttachedToJVM)
        {
            std::abort();
        }
    }
    return env;
}

void AttachCurrentThreadToJVM()
{
    Private::AndroidBridge::AttachCurrentThreadToJavaVM();
}

void DetachCurrentThreadFromJVM()
{
    Private::AndroidBridge::DetachCurrentThreadFromJavaVM();
}

bool CheckJavaException(JNIEnv* env, bool throwJniException)
{
    jthrowable e = env->ExceptionOccurred();
    if (e != nullptr)
    {
#if defined(__DAVAENGINE_DEBUG__)
        env->ExceptionDescribe();
#endif
        env->ExceptionClear();

        String exceptionText = GetJavaExceptionText(env, e);
        if (throwJniException)
        {
            throw Exception(exceptionText);
        }
        else
        {
            // Use native android logging mechanism as DAVA::Logger may not be constructed yet
            ANDROID_LOG_ERROR("[java exception] %s", exceptionText.c_str());
        }
        return true;
    }
    return false;
}

String GetJavaExceptionText(JNIEnv* env, jthrowable e)
{
    return Private::AndroidBridge::toString(env, e);
}

jclass LoadJavaClass(const char8* className, bool throwJniException, JNIEnv* env)
{
    if (env == nullptr)
    {
        env = Private::AndroidBridge::GetEnv();
    }

    if (env != nullptr)
    {
        return Private::AndroidBridge::LoadJavaClass(env, className, throwJniException);
    }
    return nullptr;
}

String JavaStringToString(jstring string, JNIEnv* env)
{
    String result;
    if (string != nullptr)
    {
        if (env == nullptr)
        {
            env = Private::AndroidBridge::GetEnv();
        }

        if (env != nullptr)
        {
            JavaClass stringClass("java/lang/String");
            Function<jbyteArray(jobject, jstring)> methodGetBytes = stringClass.GetMethod<jbyteArray, jstring>("getBytes");
            LocalRef<jstring> charsetName(env->NewStringUTF("UTF-8"));
            LocalRef<jbyteArray> stringJbytes = methodGetBytes(string, charsetName);

            const jsize length = env->GetArrayLength(stringJbytes);
            jbyte* pBytes = env->GetByteArrayElements(stringJbytes, NULL);
            JNI::CheckJavaException(env, true);
            if (pBytes != nullptr)
            {
                result.assign(reinterpret_cast<char*>(pBytes), reinterpret_cast<char*>(pBytes + length));
                env->ReleaseByteArrayElements(stringJbytes, pBytes, JNI_ABORT);
                JNI::CheckJavaException(env, true);
            }
        }
    }
    return result;
}

WideString JavaStringToWideString(jstring string, JNIEnv* env)
{
    return UTF8Utils::EncodeToWideString(JavaStringToString(string, env));
}

jstring CStrToJavaString(const char* cstr, JNIEnv* env)
{
    if (env == nullptr)
    {
        env = Private::AndroidBridge::GetEnv();
    }

    if (env != nullptr)
    {
        return env->NewStringUTF(cstr);
    }
    return nullptr;
}

jstring StringToJavaString(const String& string, JNIEnv* env)
{
    return CStrToJavaString(string.c_str(), env);
}

jstring WideStringToJavaString(const WideString& string, JNIEnv* env)
{
    return CStrToJavaString(UTF8Utils::EncodeToUTF8(string).c_str(), env);
}

} // namespace JNI
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)
#if !defined(__DAVAENGINE_COREV2__)
#include "Platform/TemplateAndroid/JniHelpers.h"
#else

#include <jni.h>
#include <stdexcept>

#include "Utils/StringFormat.h"
#include "Debug/DVAssert.h"
#include "Math/Rect.h"

#include "Engine/Private/Android/JNI/JNIDecl.h"

#define DAVA_DECLARE_CUSTOM_JNI_TYPE(jnitype, base, signature) \
    class dava_custom_jni_type##jnitype : public std::remove_pointer<base>::type {}; \
    using jnitype = dava_custom_jni_type##jnitype*; \
    template <> struct DAVA::JNI::TypeSignature<jnitype> { static const DAVA::char8* value() { return signature; } }

DAVA_DECLARE_CUSTOM_JNI_TYPE(jstringArray, jobjectArray, "[Ljava/lang/String;");

#define DAVA_JNI_EXCEPTION_CHECK \
    do { \
        try { \
            JNIEnv* env = DAVA::JNI::GetEnv(); \
            DAVA::JNI::CheckJavaException(env, true); \
        } catch (const DAVA::JNI::Exception& e) { \
            DVASSERT(false, e.what()); \
        } \
    } while (0);

namespace DAVA
{
namespace JNI
{
class Exception : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

JNIEnv* GetEnv();
void AttachCurrentThreadToJVM();
void DetachCurrentThreadFromJVM();

bool CheckJavaException(JNIEnv* env, bool throwJniException = false);
String GetJavaExceptionText(JNIEnv* env, jthrowable e);

jclass LoadJavaClass(const char8* className, bool throwJniException = false, JNIEnv* env = nullptr);

String JavaStringToString(jstring string, JNIEnv* env = nullptr);
WideString JavaStringToWideString(jstring string, JNIEnv* env = nullptr);
jstring CStrToJavaString(const char* cstr, JNIEnv* env = nullptr);
jstring StringToJavaString(const String& string, JNIEnv* env = nullptr);
jstring WideStringToJavaString(const WideString& string, JNIEnv* env = nullptr);

// Functions left for compatibility
Rect V2I(const Rect& rect);
inline String ToString(const jstring jniString)
{
    return JavaStringToString(jniString);
}
inline WideString ToWideString(const jstring jniString)
{
    return JavaStringToWideString(jniString);
}
inline jstring ToJNIString(const DAVA::WideString& string)
{
    return WideStringToJavaString(string);
}

class JavaClass
{
public:
    JavaClass() = default;
    JavaClass(const char8* className);
    JavaClass(const String& className);

    JavaClass(const JavaClass& other);
    JavaClass& operator=(const JavaClass& other);

    JavaClass(JavaClass&& other);
    JavaClass& operator=(JavaClass&& other);

    ~JavaClass();

    operator jclass() const;

    template <typename R, typename... Args>
    Function<R(jobject, Args...)> GetMethod(const char8* name) const;

    template <typename R, typename... Args>
    Function<R(Args...)> GetStaticMethod(const char8* name) const;

private:
    void DoCopy(const JavaClass& other);

    template <typename R, typename... Args>
    struct MethodCaller
    {
        MethodCaller(jmethodID m)
            : methodID(m)
        {
        }
        R operator()(jobject object, Args... args) const
        {
            JNIEnv* env = GetEnv();
            R r = static_cast<R>((env->*TypedMethod<R>::Call)(object, methodID, args...));
            CheckJavaException(env, true);
            return r;
        }
        jmethodID methodID = nullptr;
    };

    template <typename... Args>
    struct MethodCaller<void, Args...>
    {
        MethodCaller(jmethodID m)
            : methodID(m)
        {
        }
        void operator()(jobject object, Args... args) const
        {
            JNIEnv* env = GetEnv();
            (env->*TypedMethod<void>::Call)(object, methodID, args...);
            CheckJavaException(env, true);
        }
        jmethodID methodID = nullptr;
    };

    template <typename R, typename... Args>
    struct StaticMethodCaller
    {
        StaticMethodCaller(jclass c, jmethodID m)
            : clazz(c)
            , methodID(m)
        {
        }
        R operator()(Args... args) const
        {
            JNIEnv* env = GetEnv();
            R r = static_cast<R>((env->*TypedMethod<R>::CallStatic)(clazz, methodID, args...));
            CheckJavaException(env, true);
            return r;
        }
        jclass clazz = nullptr;
        jmethodID methodID = nullptr;
    };

    template <typename... Args>
    struct StaticMethodCaller<void, Args...>
    {
        StaticMethodCaller(jclass c, jmethodID m)
            : clazz(c)
            , methodID(m)
        {
        }
        void operator()(Args... args) const
        {
            JNIEnv* env = GetEnv();
            (env->*TypedMethod<void>::CallStatic)(clazz, methodID, args...);
            CheckJavaException(env, true);
        }
        jclass clazz = nullptr;
        jmethodID methodID = nullptr;
    };

private:
    jclass clazz = nullptr;
};

inline JavaClass::operator jclass() const
{
    return clazz;
}

template <typename R, typename... Args>
Function<R(jobject, Args...)> JavaClass::GetMethod(const char8* name) const
{
    JNIEnv* env = GetEnv();
    jmethodID method = env->GetMethodID(clazz, name, TypeSignature<R(Args...)>::value());
    CheckJavaException(env, true);
    return Function<R(jobject, Args...)>(MethodCaller<R, Args...>(method));
}

template <typename R, typename... Args>
Function<R(Args...)> JavaClass::GetStaticMethod(const char8* name) const
{
    JNIEnv* env = GetEnv();
    jmethodID method = env->GetStaticMethodID(clazz, name, TypeSignature<R(Args...)>::value());
    CheckJavaException(env, true);
    return Function<R(Args...)>(StaticMethodCaller<R, Args...>(clazz, method));
}

} // namespace JNI
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__

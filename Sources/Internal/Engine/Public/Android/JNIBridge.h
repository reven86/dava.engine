#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

#include <jni.h>

#include <stdexcept>

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

jclass LoadJavaClass(const char8* className, JNIEnv* env = nullptr);

String JavaStringToString(jstring string, JNIEnv* env = nullptr);
WideString JavaStringToWideString(jstring string, JNIEnv* env = nullptr);
jstring CStrToJavaString(const char* cstr, JNIEnv* env = nullptr);
jstring StringToJavaString(const String& string, JNIEnv* env = nullptr);
jstring WideStringToJavaString(const WideString& string, JNIEnv* env = nullptr);

} // namespace JNI
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__

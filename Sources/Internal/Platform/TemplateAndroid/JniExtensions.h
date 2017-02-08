#ifndef __JNI_EXTENSIONS_H__
#define __JNI_EXTENSIONS_H__

#include "Base/BaseTypes.h"

#if !defined(__DAVAENGINE_COREV2__)

#if defined(__DAVAENGINE_ANDROID__)

#include <jni.h>

namespace DAVA
{
class Rect;

class JniExtension
{
public:
    JniExtension();
    virtual ~JniExtension();

    static void SetJavaClass(JNIEnv* env, const char* className, jclass* gJavaClass, const char** gJavaClassName);

protected:
    virtual jclass GetJavaClass() const = 0;
    virtual const char* GetJavaClassName() const = 0;
    jmethodID GetMethodID(const char* methodName, const char* paramCode) const;
    JNIEnv* GetEnvironment() const;
    Rect V2P(const Rect& rect) const;

protected:
    JavaVM* vm;
};
}

#endif // __DAVAENGINE_ANDROID__

#endif // __DAVAENGINE_COREV2__

#endif // __JNI_EXTENSIONS_H__

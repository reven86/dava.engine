#include "JniExtensions.h"
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"

namespace DAVA
{
JniExtension::JniExtension()
{
    CorePlatformAndroid* core = (CorePlatformAndroid*)Core::Instance();
    AndroidSystemDelegate* delegate = core->GetAndroidSystemDelegate();
    vm = delegate->GetVM();
}

JniExtension::~JniExtension()
{
}

void JniExtension::SetJavaClass(JNIEnv* env, const char* className, jclass* gJavaClass, const char** gJavaClassName)
{
    *gJavaClass = (jclass)env->NewGlobalRef(env->FindClass(className));
    if (gJavaClassName)
        *gJavaClassName = className;
}

jmethodID JniExtension::GetMethodID(const char* methodName, const char* paramCode) const
{
    jclass javaClass = GetJavaClass();
    DVASSERT(javaClass && "Not initialized Java class");
    if (!javaClass)
        return 0;

    jmethodID mid = GetEnvironment()->GetStaticMethodID(javaClass, methodName, paramCode);

    if (!mid)
    {
        Logger::Error("get method id of %s.%s%s error ", GetJavaClassName(), methodName, paramCode);
    }

    return mid;
}

JNIEnv* JniExtension::GetEnvironment() const
{
    // right way to take JNIEnv
    // JNIEnv is valid only for the thread where it was gotten.
    // we shouldn't store JNIEnv.

    JNIEnv* env;
    if (JNI_EDETACHED == vm->GetEnv((void**)&env, JNI_VERSION_1_6))
    {
        Logger::Error("runtime_error(Thread is not attached to JNI)");
    }
    return env;
};

Rect JniExtension::V2P(const Rect& srcRect) const
{
    Vector2 offset = VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset();
    Rect rect = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(srcRect);

    rect += offset;
    return rect;
}

} //namespace DAVA

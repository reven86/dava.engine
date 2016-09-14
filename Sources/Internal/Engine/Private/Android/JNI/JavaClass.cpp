#include "Engine/Android/JNIBridge.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

namespace DAVA
{
namespace JNI
{
JavaClass::JavaClass(const char8* className)
    : clazz(LoadJavaClass(className, true))
{
}

JavaClass::JavaClass(const String& className)
    : JavaClass(className.c_str())
{
}

JavaClass::JavaClass(const JavaClass& other)
{
    DoCopy(other);
}

JavaClass& JavaClass::operator=(const JavaClass& other)
{
    if (this != &other)
    {
        DoCopy(other);
    }
    return *this;
}

JavaClass::JavaClass(JavaClass&& other)
    : clazz(other.clazz)
{
    other.clazz = nullptr;
}

JavaClass& JavaClass::operator=(JavaClass&& other)
{
    if (this != &other)
    {
        if (clazz != nullptr)
        {
            GetEnv()->DeleteGlobalRef(clazz);
            clazz = nullptr;
        }

        clazz = other.clazz;
        other.clazz = nullptr;
    }
    return *this;
}

JavaClass::~JavaClass()
{
    if (clazz != nullptr)
    {
        GetEnv()->DeleteGlobalRef(clazz);
        clazz = nullptr;
    }
}

void JavaClass::DoCopy(const JavaClass& other)
{
    JNIEnv* env = GetEnv();
    if (clazz != nullptr)
    {
        env->DeleteGlobalRef(clazz);
        clazz = nullptr;
    }
    if (other.clazz != nullptr)
    {
        clazz = static_cast<jclass>(env->NewGlobalRef(other.clazz));
    }
}

} // namespace JNI
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__

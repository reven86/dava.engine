#ifndef __DAVAENGINE_UTILS_ANDROID_H__
#define __DAVAENGINE_UTILS_ANDROID_H__

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/JniHelpers.h"

namespace DAVA
{
class JniUtils
{
public:
    JniUtils();
    bool DisableSleepTimer();
    bool EnableSleepTimer();
    void OpenURL(const String& url);
    String GenerateGUID();

private:
    JNI::JavaClass jniUtils;
    Function<void()> disableSleepTimer;
    Function<void()> enableSleepTimer;
    Function<void(jstring)> openURL;
    Function<jstring()> generateGUID;
};
};

#endif //__DAVAENGINE_ANDROID__

#endif // __DAVAENGINE_UTILS_H__

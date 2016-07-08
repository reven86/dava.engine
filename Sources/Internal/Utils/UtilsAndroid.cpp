#include "Utils/Utils.h"
#include "UtilsAndroid.h"

using namespace DAVA;

#if defined(__DAVAENGINE_ANDROID__)

JniUtils::JniUtils()
    : jniUtils("com/dava/framework/JNIUtils")
{
    disableSleepTimer = jniUtils.GetStaticMethod<void>("DisableSleepTimer");
    enableSleepTimer = jniUtils.GetStaticMethod<void>("EnableSleepTimer");
    openURL = jniUtils.GetStaticMethod<void, jstring>("OpenURL");
    generateGUID = jniUtils.GetStaticMethod<jstring>("GenerateGUID");
}

bool JniUtils::DisableSleepTimer()
{
    disableSleepTimer();
    return true;
}

bool JniUtils::EnableSleepTimer()
{
    enableSleepTimer();
    return true;
}

void JniUtils::OpenURL(const String& url)
{
    JNIEnv* env = JNI::GetEnv();
    jstring jUrl = env->NewStringUTF(url.c_str());
    openURL(jUrl);
    env->DeleteLocalRef(jUrl);
}

String JniUtils::GenerateGUID()
{
    JNIEnv* env = JNI::GetEnv();
    jstring jstr = generateGUID();
    const char* str = env->GetStringUTFChars(jstr, 0);
    DAVA::String result(str);
    env->ReleaseStringUTFChars(jstr, str);
    return result;
}

void DAVA::DisableSleepTimer()
{
    JniUtils jniUtils;
    jniUtils.DisableSleepTimer();
}

void DAVA::EnableSleepTimer()
{
    JniUtils jniUtils;
    jniUtils.EnableSleepTimer();
}

uint64 DAVA::EglGetCurrentContext()
{
    //TODO: in case if context checking will ever be needed on Android,
    //TODO: implement this method similar to any other platform
    //TODO: it should return uint64 representation of the current OpenGL context
    //TDOD: see iOS implementation for example
    DVASSERT(false && "Implement this method for Android if needed");
    return 0;
}

void DAVA::OpenURL(const String& url)
{
    JniUtils jniUtils;
    jniUtils.OpenURL(url);
}

String DAVA::GenerateGUID()
{
    JniUtils jniUtils;
    return jniUtils.GenerateGUID();
}

#endif //#if defined(__DAVAENGINE_ANDROID__)

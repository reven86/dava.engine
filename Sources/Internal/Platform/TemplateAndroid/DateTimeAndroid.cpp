#include "Platform/DateTime.h"
#include "Utils/Utils.h"
#include "Utils/UTF8Utils.h"
#include "FileSystem/LocalizationSystem.h"


#if defined(__DAVAENGINE_ANDROID__)
#include "DateTimeAndroid.h"
#include "ExternC/AndroidLayer.h"

namespace DAVA
{
JniDateTime::JniDateTime()
    : jniDateTime("com/dava/framework/JNIDateTime")
{
    getTimeAsString = jniDateTime.GetStaticMethod<jstring, jstring, jstring, jlong, jint>("GetTimeAsString");
    getLocalTimeZoneOffset = jniDateTime.GetStaticMethod<jint>("GetLocalTimeZoneOffset");
}

WideString JniDateTime::AsWString(const WideString& format, const String& countryCode, long timeStamp, int tzOffset)
{
    JNIEnv* env = JNI::GetEnv();

    jstring jFormat = env->NewStringUTF(UTF8Utils::EncodeToUTF8(format).c_str());
    jstring jCountryCode = env->NewStringUTF(countryCode.c_str());

    jstring obj = getTimeAsString(jFormat, jCountryCode, (long long)timeStamp, tzOffset);

    env->DeleteLocalRef(jFormat);
    env->DeleteLocalRef(jCountryCode);

    WideString result = JNI::ToWideString(obj);
    return result;
}

int JniDateTime::GetLocalTimeZoneOffset()
{
    return getLocalTimeZoneOffset();
}

WideString DateTime::AsWString(const wchar_t* format) const
{
    JniDateTime jniDateTime;
    String countryCode = LocalizationSystem::Instance()->GetCountryCode();
    WideString retString = jniDateTime.AsWString(WideString(format), countryCode, innerTime, timeZoneOffset);

    return retString;
}

WideString DateTime::GetLocalizedDate() const
{
    return AsWString(L"%x");
}

WideString DateTime::GetLocalizedTime() const
{
    return AsWString(L"%X");
}

int32 DateTime::GetLocalTimeZoneOffset()
{
    JniDateTime jniDateTime;
    return jniDateTime.GetLocalTimeZoneOffset();
}
}

#endif

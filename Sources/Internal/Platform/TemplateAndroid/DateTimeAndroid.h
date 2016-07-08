#ifndef __FRAMEWORK__DATETIMEANDROID__
#define __FRAMEWORK__DATETIMEANDROID__

#include "Base/BaseTypes.h"
#include "Platform/TemplateAndroid/JniHelpers.h"

namespace DAVA
{
class JniDateTime
{
public:
    JniDateTime();
    WideString AsWString(const WideString& format, const String& countryCode, long timeStamp, int tzOffset);
    int GetLocalTimeZoneOffset();

private:
    JNI::JavaClass jniDateTime;
    Function<jstring(jstring, jstring, jlong, jint)> getTimeAsString;
    Function<jint()> getLocalTimeZoneOffset;
};
};

#endif /* defined(__FRAMEWORK__DATETIMEANDROID__) */

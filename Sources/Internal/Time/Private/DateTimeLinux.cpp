#include "Time/DateTime.h"

#if defined(__DAVAENGINE_LINUX__)

#include "Utils/UTF8Utils.h"

#include <ctime>

namespace DAVA
{
WideString DateTime::AsWString(const wchar_t* format) const
{
    // TODO: linux: get locale from LocalizationSystem

    struct tm tm
    {
    };
    Timestamp timeWithTZ = innerTime + timeZoneOffset;
    GmTimeThreadSafe(&tm, &timeWithTZ);

    char buf[256];
    String formatUtf8 = UTF8Utils::MakeUTF8String(format);
    strftime(buf, 256, formatUtf8.c_str(), &tm);
    return UTF8Utils::EncodeToWideString(buf);
}

int32 DateTime::GetLocalTimeZoneOffset()
{
    // Taken from here: https://stackoverflow.com/questions/32424125/c-code-to-get-local-time-offset-in-minutes-relative-to-utc
    time_t rawTime = time(nullptr);

    struct tm gtm;
    gmtime_r(&rawTime, &gtm);

    gtm.tm_isdst = -1;
    time_t gtime = mktime(&gtm);

    return static_cast<int32>(difftime(rawTime, gtime) / 60.);
}

WideString DateTime::GetLocalizedDate() const
{
    return AsWString(L"%x");
}

WideString DateTime::GetLocalizedTime() const
{
    return AsWString(L"%X");
}
}

#endif // __DAVAENGINE_LINUX__

#include "Time/DateTime.h"

// TODO: linux
#if defined(__DAVAENGINE_LINUX__)

namespace DAVA
{
WideString DateTime::AsWString(const wchar_t* format) const
{
    DVASSERT(0, "Implement DateTime::AsWString");
    return WideString();
}

int32 DateTime::GetLocalTimeZoneOffset()
{
    DVASSERT(0, "Implement DateTime::GetLocalTimeZoneOffset");
    return 0;
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

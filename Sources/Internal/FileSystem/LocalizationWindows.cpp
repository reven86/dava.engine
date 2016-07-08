#include "Base/Platform.h"

#if defined(__DAVAENGINE_WINDOWS__)

#include "FileSystem/LocalizationSystem.h"
#include "Platform/DeviceInfo.h"

namespace DAVA
{
String LocalizationSystem::GetDeviceLocale(void) const
{
    String locale = DeviceInfo::GetLocale();
    String::size_type posEnd = locale.find('-', 2);
    if (String::npos != posEnd)
    {
        locale = locale.substr(0, posEnd);
    }
    return locale;
}
};

#endif // __DAVAENGINE_WIN_UAP__
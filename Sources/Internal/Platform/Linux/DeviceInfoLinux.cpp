#include "Base/Platform.h"

// TODO: linux
#if defined(__DAVAENGINE_LINUX__)

#include "Base/GlobalEnum.h"
#include "Platform/DeviceInfo.h"
#include "Platform/Linux/DeviceInfoLinux.h"
#include "Utils/UTF8Utils.h"

#include <sys/utsname.h>

namespace DAVA
{
DeviceInfoPrivate::DeviceInfoPrivate() = default;

DeviceInfo::ePlatform DeviceInfoPrivate::GetPlatform()
{
    return DeviceInfo::PLATFORM_LINUX;
}

String DeviceInfoPrivate::GetPlatformString()
{
    return GlobalEnumMap<DeviceInfo::ePlatform>::Instance()->ToString(GetPlatform());
}

String DeviceInfoPrivate::GetVersion()
{
    struct utsname buf
    {
    };
    if (uname(&buf) == 0)
    {
        return String(buf.release);
    }
    return String();
}

String DeviceInfoPrivate::GetManufacturer()
{
    return String();
}

String DeviceInfoPrivate::GetModel()
{
    return String();
}

String DeviceInfoPrivate::GetLocale()
{
    return String();
}

String DeviceInfoPrivate::GetRegion()
{
    return String();
}

String DeviceInfoPrivate::GetTimeZone()
{
    return String();
}

String DeviceInfoPrivate::GetHTTPProxyHost()
{
    return String();
}

String DeviceInfoPrivate::GetHTTPNonProxyHosts()
{
    return String();
}

int32 DeviceInfoPrivate::GetHTTPProxyPort()
{
    return 0;
}

int32 DeviceInfoPrivate::GetZBufferSize()
{
    return 24;
}

List<DeviceInfo::StorageInfo> DeviceInfoPrivate::GetStoragesList()
{
    return List<DeviceInfo::StorageInfo>();
}

String DeviceInfoPrivate::GetUDID()
{
    return String();
}

WideString DeviceInfoPrivate::GetName()
{
    struct utsname buf
    {
    };
    if (uname(&buf) == 0)
    {
        return UTF8Utils::EncodeToWideString(buf.nodename);
    }
    return WideString();
}

eGPUFamily DeviceInfoPrivate::GetGPUFamilyImpl()
{
    return GPU_ORIGIN;
}

DeviceInfo::NetworkInfo DeviceInfoPrivate::GetNetworkInfo()
{
    return DeviceInfo::NetworkInfo();
}

bool DeviceInfoPrivate::IsHIDConnected(DeviceInfo::eHIDType type)
{
    return false;
}

bool DeviceInfoPrivate::IsTouchPresented()
{
    return false;
}

String DeviceInfoPrivate::GetCarrierName()
{
    return String();
}

} // namespace DAVA

#endif // defined(__DAVAENGINE_LINUX__)

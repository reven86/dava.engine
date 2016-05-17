#ifndef __FRAMEWORK__DEVICEINFO_WIN32__
#define __FRAMEWORK__DEVICEINFO_WIN32__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN32__)

#include "Platform/DeviceInfoPrivateBase.h"

namespace DAVA
{
class DeviceInfoPrivate : public DeviceInfoPrivateBase
{
public:
    DeviceInfoPrivate();
    DeviceInfo::ePlatform GetPlatform();
    String GetPlatformString();
    String GetVersion();
    String GetManufacturer();
    String GetModel();
    String GetLocale();
    String GetRegion();
    String GetTimeZone();
    String GetUDID();
    WideString GetName();
    String GetHTTPProxyHost();
    String GetHTTPNonProxyHosts();
    int32 GetHTTPProxyPort();
    DeviceInfo::ScreenInfo& GetScreenInfo();
    int32 GetZBufferSize();
    eGPUFamily GetGPUFamily();
    DeviceInfo::NetworkInfo GetNetworkInfo();
    List<DeviceInfo::StorageInfo> GetStoragesList();
    void InitializeScreenInfo();
    bool IsHIDConnected(DeviceInfo::eHIDType type);
    bool IsTouchPresented();

private:
    DeviceInfo::ScreenInfo screenInfo;
};
};

#endif //  defined(__DAVAENGINE_WIN_32__)

#endif //  defined(__FRAMEWORK__DEVICEINFO_WIN32__)

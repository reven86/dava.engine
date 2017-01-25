#ifndef __FRAMEWORK__DEVICEINFO_MACOS__
#define __FRAMEWORK__DEVICEINFO_MACOS__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_MACOS__)

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
    int32 GetZBufferSize();
    eGPUFamily GetGPUFamilyImpl() override;
    DeviceInfo::NetworkInfo GetNetworkInfo();
    List<DeviceInfo::StorageInfo> GetStoragesList();
    bool IsHIDConnected(DeviceInfo::eHIDType type);
    bool IsTouchPresented();
    String GetCarrierName();

#if !defined(__DAVAENGINE_COREV2__)
    DeviceInfo::ScreenInfo screenInfo;
    DeviceInfo::ScreenInfo& GetScreenInfo();
    void InitializeScreenInfo();
#endif
};

}; // namespace DAVA

#endif //defined(__DAVAENGINE_MACOS__)

#endif /* defined(__FRAMEWORK__DEVICEINFO_MACOS__) */

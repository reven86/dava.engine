#ifndef __FRAMEWORK__DEVICEINFO_IOS__
#define __FRAMEWORK__DEVICEINFO_IOS__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Platform/DeviceInfoPrivateBase.h"

namespace DAVA
{
struct DeviceInfoObjcBridge;

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
    String GetCarrierName();

private:
    DeviceInfo::ScreenInfo screenInfo;
    std::unique_ptr<DeviceInfoObjcBridge> bridge;
};

}; // namespace DAVA

#endif //defined(__DAVAENGINE_IPHONE__)

#endif /* defined(__FRAMEWORK__DEVICEINFO_IOS__) */

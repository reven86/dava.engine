#pragma once

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
    eGPUFamily GetGPUFamilyImpl() override;
    DeviceInfo::NetworkInfo GetNetworkInfo();
    List<DeviceInfo::StorageInfo> GetStoragesList();
    void InitializeScreenInfo();
    bool IsHIDConnected(DeviceInfo::eHIDType type);
    bool IsTouchPresented();
    String GetCarrierName();

private:
    DeviceInfo::ScreenInfo screenInfo;
};
};

#endif //  defined(__DAVAENGINE_WIN32__)

#ifndef __FRAMEWORK__DEVICEINFOANDROID__
#define __FRAMEWORK__DEVICEINFOANDROID__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "JniExtensions.h"
#include "Base/BaseTypes.h"
#include "Platform/DeviceInfoPrivateBase.h"
#include "Platform/TemplateAndroid/JniHelpers.h"

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

protected:
    DeviceInfo::StorageInfo StorageInfoFromJava(jobject object);

private:
    int32 GetNetworkType();
    int32 GetSignalStrength(int32 networkType);
    bool IsPrimaryExternalStoragePresent();
    DeviceInfo::StorageInfo GetInternalStorageInfo();
    DeviceInfo::StorageInfo GetPrimaryExternalStorageInfo();
    List<DeviceInfo::StorageInfo> GetSecondaryExternalStoragesList();

    JNI::JavaClass jniDeviceInfo;
    Function<jstring()> getVersion;
    Function<jstring()> getManufacturer;
    Function<jstring()> getModel;
    Function<jstring()> getLocale;
    Function<jstring()> getRegion;
    Function<jstring()> getTimeZone;
    Function<jstring()> getUDID;
    Function<jstring()> getName;
    Function<jint()> getZBufferSize;
    Function<jstring()> getHTTPProxyHost;
    Function<jstring()> getHTTPNonProxyHosts;
    Function<jint()> getHTTPProxyPort;
    Function<jint()> getNetworkType;
    Function<jint(jint)> getSignalStrength;
    Function<jboolean()> isPrimaryExternalStoragePresent;

    DeviceInfo::ScreenInfo screenInfo;
};
};

#endif //defined(__DAVAENGINE_ANDROID__)

#endif /* defined(__FRAMEWORK__DEVICEINFOANDROID__) */

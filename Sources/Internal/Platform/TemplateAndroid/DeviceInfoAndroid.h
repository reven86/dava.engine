#ifndef __FRAMEWORK__DEVICEINFOANDROID__
#define __FRAMEWORK__DEVICEINFOANDROID__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Android/JNIBridge.h"
#include "JniExtensions.h"
#include "Base/BaseTypes.h"
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
    Function<jstring()> getCarrierName;
    Function<jbyte()> getGpuFamily;
};
};

#endif //defined(__DAVAENGINE_ANDROID__)

#endif /* defined(__FRAMEWORK__DEVICEINFOANDROID__) */

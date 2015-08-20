/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __FRAMEWORK__DEVICEINFOANDROID__
#define __FRAMEWORK__DEVICEINFOANDROID__

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "JniExtensions.h"
#include "Base/BaseTypes.h"
#include "Platform/DeviceInfo.h"
#include "Platform/TemplateAndroid/JniHelpers.h"

namespace DAVA
{

class DeviceInfoPrivate
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
    int32 GetCpuCount();
    void InitializeScreenInfo();
    bool IsHIDConnected(DeviceInfo::eHIDType type);
    void SetHIDConnectionCallback(DeviceInfo::eHIDType type, DeviceInfo::HIDCallBackFunc&& callback);

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
    Function<jint()> getGPUFamily;
    Function<jint()> getNetworkType;
    Function<jint(jint)> getSignalStrength;
    Function<jboolean()> isPrimaryExternalStoragePresent;

    DeviceInfo::ScreenInfo screenInfo;
};

};

#endif //defined(__DAVAENGINE_ANDROID__)

#endif /* defined(__FRAMEWORK__DEVICEINFOANDROID__) */

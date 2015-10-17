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

#include "Platform/DeviceInfo.h"

#if defined(__DAVAENGINE_IPHONE__)
    #include "Platform/TemplateiOS/DeviceInfoiOS.h"
#elif defined(__DAVAENGINE_MACOS__)
    #include "Platform/TemplateMacOS/DeviceInfoMacOS.h"
#elif defined(__DAVAENGINE_ANDROID__)
    #include "Platform/TemplateAndroid/DeviceInfoAndroid.h"
#elif defined(__DAVAENGINE_WIN32__)
    #include "Platform/TemplateWin32/DeviceInfoWin32.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
    #include "Platform/TemplateWin32/DeviceInfoWinUAP.h"
#endif

namespace DAVA
{
DeviceInfoPrivate* DeviceInfo::GetPrivateImpl()
{
    static DeviceInfoPrivate privateImpl;
    return &privateImpl;
}

DeviceInfo::ePlatform DeviceInfo::GetPlatform()
{
    return GetPrivateImpl()->GetPlatform();
}

String DeviceInfo::GetPlatformString()
{
    return GetPrivateImpl()->GetPlatformString();
}

String DeviceInfo::GetVersion()
{
    return GetPrivateImpl()->GetVersion();
}

String DeviceInfo::GetManufacturer()
{
    return GetPrivateImpl()->GetManufacturer();
}

String DeviceInfo::GetModel()
{
    return GetPrivateImpl()->GetModel();
}

String DeviceInfo::GetLocale()
{
    return GetPrivateImpl()->GetLocale();
}

String DeviceInfo::GetRegion()
{
    return GetPrivateImpl()->GetRegion();
}

String DeviceInfo::GetTimeZone()
{
    return GetPrivateImpl()->GetTimeZone();
}

String DeviceInfo::GetUDID()
{
    return GetPrivateImpl()->GetUDID();
}

WideString DeviceInfo::GetName()
{
    return GetPrivateImpl()->GetName();
}

String DeviceInfo::GetHTTPProxyHost()
{
    return GetPrivateImpl()->GetHTTPProxyHost();
}

String DeviceInfo::GetHTTPNonProxyHosts()
{
    return GetPrivateImpl()->GetHTTPNonProxyHosts();
}

int32 DeviceInfo::GetHTTPProxyPort()
{
    return GetPrivateImpl()->GetHTTPProxyPort();
}

DeviceInfo::ScreenInfo & DeviceInfo::GetScreenInfo()
{
    return GetPrivateImpl()->GetScreenInfo();
}

int32 DeviceInfo::GetZBufferSize()
{
    return GetPrivateImpl()->GetZBufferSize();
}

eGPUFamily DeviceInfo::GetGPUFamily()
{
    return GetPrivateImpl()->GetGPUFamily();
}

DeviceInfo::NetworkInfo DeviceInfo::GetNetworkInfo()
{
    return GetPrivateImpl()->GetNetworkInfo();
}

List<DeviceInfo::StorageInfo> DeviceInfo::GetStoragesList()
{
    return GetPrivateImpl()->GetStoragesList();
}

int32 DeviceInfo::GetCpuCount()
{
    return GetPrivateImpl()->GetCpuCount();
}

void DeviceInfo::InitializeScreenInfo()
{
    GetPrivateImpl()->InitializeScreenInfo();
}

bool DeviceInfo::IsHIDConnected(eHIDType type)
{
    return GetPrivateImpl()->IsHIDConnected(type);
}

DeviceInfo::HIDConnectionSignal& DeviceInfo::GetHIDConnectionSignal(DeviceInfo::eHIDType type)
{
    return GetPrivateImpl()->GetHIDConnectionSignal(type);
}

}  // namespace DAVA
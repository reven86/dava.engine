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
#import "Platform/Reachability.h"

#ifdef __DAVAENGINE_MACOS__

#include "Platform/TemplateMacOS/DeviceInfoMacOS.h"
#include "Base/GlobalEnum.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#import <Foundation/NSLocale.h>
#import <Foundation/NSTimeZone.h>
#import <Foundation/NSProcessInfo.h>
#import <AppKit/NSScreen.h>
#include "Utils/StringFormat.h"
#include "OpenUDIDMacOS.h"
#include "Platform/DeviceInfo.h"

namespace DAVA
{
String GetSysCtlByName(const String& param)
{
    size_t len = 0;
    sysctlbyname(param.c_str(), NULL, &len, NULL, 0);

    if (len)
    {
        char model[len];
        sysctlbyname(param.c_str(), model, &len, NULL, 0);
        NSString* model_ns = [NSString stringWithUTF8String:model];
        return String([model_ns UTF8String]);
    }

    return String("");
}

DeviceInfoPrivate::DeviceInfoPrivate()
{
}

DeviceInfo::ePlatform DeviceInfoPrivate::GetPlatform()
{
    return DeviceInfo::PLATFORM_MACOS;
}

String DeviceInfoPrivate::GetPlatformString()
{
    return GlobalEnumMap<DeviceInfo::ePlatform>::Instance()->ToString(GetPlatform());
}

String DeviceInfoPrivate::GetVersion()
{
    NSOperatingSystemVersion sysVersion;
    NSProcessInfo* procInfo = [NSProcessInfo processInfo];
    sysVersion = procInfo.operatingSystemVersion;

    NSString* systemVersion =
    [NSString stringWithFormat:@"%ld.%ld.%ld", sysVersion.majorVersion,
                               sysVersion.minorVersion,
                               sysVersion.patchVersion];

    return String([systemVersion UTF8String]);
}

String DeviceInfoPrivate::GetManufacturer()
{
    return "Apple inc.";
}

String DeviceInfoPrivate::GetModel()
{
    String model = GetSysCtlByName("hw.model");

    if (0 < model.length())
    {
        return model;
    }

    return "Just an Apple Computer";
}

String DeviceInfoPrivate::GetLocale()
{
    NSLocale* english = [[[NSLocale alloc] initWithLocaleIdentifier:@"en_US"] autorelease];

    NSString* langID = [[NSLocale preferredLanguages] objectAtIndex:0];
    NSString* lang = [english displayNameForKey:NSLocaleLanguageCode value:langID];

    String res = Format("%s (%s)", [langID UTF8String], [lang UTF8String]);
    return res;
}

String DeviceInfoPrivate::GetRegion()
{
    NSLocale* english = [[[NSLocale alloc] initWithLocaleIdentifier:@"en_US"] autorelease];

    NSString* countryCode = [[NSLocale currentLocale] objectForKey:NSLocaleCountryCode];
    NSString* country = [english displayNameForKey:NSLocaleCountryCode value:countryCode];

    String res = Format("%s (%s)", [countryCode UTF8String], [country UTF8String]);
    return res;
}

String DeviceInfoPrivate::GetTimeZone()
{
    NSTimeZone* localTime = [NSTimeZone systemTimeZone];

    String res = Format("%s", [[localTime name] UTF8String]);
    return res;
}

String DeviceInfoPrivate::GetUDID()
{
    OpenUDIDMacOS* udid = [[[OpenUDIDMacOS alloc] init] autorelease];
    return [[udid value] UTF8String];
}

WideString DeviceInfoPrivate::GetName()
{
    NSString* deviceName = [[NSHost currentHost] localizedName];

    NSStringEncoding pEncode = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE);
    NSData* pSData = [deviceName dataUsingEncoding:pEncode];

    return WideString(reinterpret_cast<const wchar_t*>([pSData bytes]), [pSData length] / sizeof(wchar_t));
}

// Not impletemted yet
String DeviceInfoPrivate::GetHTTPProxyHost()
{
    return String();
}

// Not impletemted yet
String DeviceInfoPrivate::GetHTTPNonProxyHosts()
{
    return String();
}

// Not impletemted yet
int32 DeviceInfoPrivate::GetHTTPProxyPort()
{
    return 0;
}

DeviceInfo::ScreenInfo& DeviceInfoPrivate::GetScreenInfo()
{
    return screenInfo;
}

int32 DeviceInfoPrivate::GetZBufferSize()
{
    return 24;
}

eGPUFamily DeviceInfoPrivate::GetGPUFamily()
{
    return GPU_POWERVR_IOS;
}

DeviceInfo::NetworkInfo DeviceInfoPrivate::GetNetworkInfo()
{
    static const struct
    {
        NetworkStatus platformNetworkStatus;
        DeviceInfo::eNetworkType internalNetworkType;
    } networkStatusMap[] =
    {
      { NotReachable, DeviceInfo::NETWORK_TYPE_NOT_CONNECTED },
      { ReachableViaWiFi, DeviceInfo::NETWORK_TYPE_WIFI },
      { ReachableViaWWAN, DeviceInfo::NETWORK_TYPE_CELLULAR }
    };

    DeviceInfo::NetworkInfo networkInfo;

    Reachability* reachability = [Reachability reachabilityForInternetConnection];
    [reachability startNotifier];

    NetworkStatus networkStatus = [reachability currentReachabilityStatus];

    uint32 networkStatusMapCount = COUNT_OF(networkStatusMap);
    for (uint32 i = 0; i < networkStatusMapCount; i++)
    {
        if (networkStatusMap[i].platformNetworkStatus == networkStatus)
        {
            networkInfo.networkType = networkStatusMap[i].internalNetworkType;
            break;
        }
    }

    [reachability stopNotifier];

    // No way to determine signal strength under iOS.
    return networkInfo;
}

List<DeviceInfo::StorageInfo> DeviceInfoPrivate::GetStoragesList()
{
    List<DeviceInfo::StorageInfo> l;
    return l;
}

void DeviceInfoPrivate::InitializeScreenInfo()
{
    screenInfo.width = [[NSScreen mainScreen] frame].size.width;
    screenInfo.height = [[NSScreen mainScreen] frame].size.height;
    screenInfo.scale = [[NSScreen mainScreen] backingScaleFactor];
}

bool DeviceInfoPrivate::IsHIDConnected(DeviceInfo::eHIDType type)
{
    //TODO: remove this empty realization and implement detection of HID connection
    if (type == DeviceInfo::HID_MOUSE_TYPE || type == DeviceInfo::HID_KEYBOARD_TYPE)
    {
        return true;
    }
    return false;
}

bool DeviceInfoPrivate::IsTouchPresented()
{
    return false;
}
}

#endif

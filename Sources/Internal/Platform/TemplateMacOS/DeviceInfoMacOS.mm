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

#ifdef __DAVAENGINE_MACOS__

#include "Platform/TemplateMacOS/DeviceInfoMacOS.h"
#include "Base/GlobalEnum.h"

#import <Foundation/NSLocale.h>
#import <Foundation/NSTimeZone.h>
#import <AppKit/NSScreen.h>
#include "Utils/StringFormat.h"
#include "OpenUDIDMacOS.h"

namespace DAVA
{

DeviceInfoPrivate::DeviceInfoPrivate()
{
}

DeviceInfo::ePlatform DeviceInfoPrivate::GetPlatform()
{
    return 	DeviceInfo::PLATFORM_MACOS;
}

String DeviceInfoPrivate::GetPlatformString()
{
    return GlobalEnumMap<DeviceInfo::ePlatform>::Instance()->ToString(GetPlatform());
}

String DeviceInfoPrivate::GetVersion()
{
	return "Not yet implemented";
}

String DeviceInfoPrivate::GetManufacturer()
{
	return "Apple inc.";
}

String DeviceInfoPrivate::GetModel()
{
	return "Not yet implemented";
}

String DeviceInfoPrivate::GetLocale()
{
	NSLocale *english = [[[NSLocale alloc] initWithLocaleIdentifier:@"en_US"] autorelease];

	NSString* langID = [[NSLocale preferredLanguages] objectAtIndex:0];
	NSString *lang = [english displayNameForKey:NSLocaleLanguageCode value:langID];

	String res = Format("%s (%s)", [langID UTF8String], [lang UTF8String]);
	return res;
}

String DeviceInfoPrivate::GetRegion()
{
	NSLocale *english = [[[NSLocale alloc] initWithLocaleIdentifier:@"en_US"] autorelease];

	NSString *countryCode = [[NSLocale currentLocale] objectForKey: NSLocaleCountryCode];
	NSString *country = [english displayNameForKey: NSLocaleCountryCode value: countryCode];

	String res = Format("%s (%s)", [countryCode UTF8String], [country UTF8String]);
	return res;
}

String DeviceInfoPrivate::GetTimeZone()
{
	NSTimeZone *localTime = [NSTimeZone systemTimeZone];
    
    String res = Format("%s", [[localTime name] UTF8String]);
	return res;
}
    
String DeviceInfoPrivate::GetUDID()
{
    OpenUDIDMacOS*  udid = [[[OpenUDIDMacOS alloc] init] autorelease];
    return [[udid value] UTF8String];
}
    
WideString DeviceInfoPrivate::GetName()
{
    NSString * deviceName = [[NSHost currentHost] localizedName];
    
    NSStringEncoding pEncode    =   CFStringConvertEncodingToNSStringEncoding ( kCFStringEncodingUTF32LE );
    NSData* pSData              =   [ deviceName dataUsingEncoding : pEncode ];
    
    return WideString ( (wchar_t*) [ pSData bytes ], [ pSData length] / sizeof ( wchar_t ) );
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
    return GPU_INVALID;
}

DeviceInfo::NetworkInfo DeviceInfoPrivate::GetNetworkInfo()
{
    // For now return default network info for MacOS.
    return DeviceInfo::NetworkInfo();
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
	screenInfo.scale = 1;
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
    //TODO: remove this empty realization and implement detection touch
    return false;
}

}

#endif

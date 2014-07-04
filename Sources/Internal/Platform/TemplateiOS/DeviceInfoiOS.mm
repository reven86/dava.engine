/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Platform/DeviceInfo.h"

#ifdef __DAVAENGINE_IPHONE__

#include "Utils/StringFormat.h"

#import <UIKit/UIDevice.h>
#import <Foundation/NSLocale.h>
#import <sys/utsname.h>
#import <AdSupport/ASIdentifierManager.h>

namespace DAVA
{

String DeviceInfo::GetVersion()
{
	NSString* systemVersion = [[UIDevice currentDevice] systemVersion];
	return String([systemVersion UTF8String]);
}

String DeviceInfo::GetManufacturer()
{
	return "Apple inc.";
}

String DeviceInfo::GetModel()
{
	String model = "";

	if (GetPlatform() == PLATFORM_IOS_SIMULATOR)
	{
		model = [[[UIDevice currentDevice] model] UTF8String];
	}
	else
	{
		struct  utsname systemInfo;
		uname(&systemInfo);

		NSString* modelName = [NSString stringWithCString:systemInfo.machine encoding:NSUTF8StringEncoding];

		//General
		if ([modelName hasPrefix:@"iPhone"])
			model = [modelName UTF8String];
		if ([modelName hasPrefix:@"iPad"])
			model = [modelName UTF8String];
		if ([modelName hasPrefix:@"iPod"])
			model = [modelName UTF8String];
		if ([modelName hasPrefix:@"AppleTV"])
			model = [modelName UTF8String];

		// iPhone
		if ([modelName hasPrefix:@"iPhone1,1"])
			model = "iPhone 1G";
		if ([modelName hasPrefix:@"iPhone1,2"])
			model = "iPhone 3G";
		if ([modelName hasPrefix:@"iPhone2,1"])
			model = "iPhone 3GS";

		if ([modelName hasPrefix:@"iPhone3,1"])
			model = "iPhone 4 GSM";
		if ([modelName hasPrefix:@"iPhone3,3"])
			model = "iPhone 4 CDMA";

		if ([modelName hasPrefix:@"iPhone4,1"])
			model = "iPhone 4S";

		if ([modelName hasPrefix:@"iPhone5,1"])
			model = "iPhone 5 GSM LTE";
		if ([modelName hasPrefix:@"iPhone5,2"])
			model = "iPhone 5 CDMA LTE";

		if ([modelName hasPrefix:@"iPhone5,3"])
			model = "iPhone 5C GSM";
		if ([modelName hasPrefix:@"iPhone5,4"])
			model = "iPhone 5C GSM+CDMA";

		if ([modelName hasPrefix:@"iPhone6,1"])
			model = "iPhone 5S GSM";
		if ([modelName hasPrefix:@"iPhone6,2"])
			model = "iPhone 5S GSM+CDMA";

		// iPad
		if ([modelName hasPrefix:@"iPad1,1"])
			model = "iPad 1";

		if ([modelName hasPrefix:@"iPad2,1"])
			model = "iPad 2 WiFi";
		if ([modelName hasPrefix:@"iPad2,2"])
			model = "iPad 2 3G GSM";
		if ([modelName hasPrefix:@"iPad2,3"])
			model = "iPad 2 3G CDMA";
		if ([modelName hasPrefix:@"iPad2,4"])
			model = "iPad 2 WiFi";

		if ([modelName hasPrefix:@"iPad2,5"])
			model = "iPad Mini WiFi";
		if ([modelName hasPrefix:@"iPad2,6"])
			model = "iPad Mini GSM LTE";
		if ([modelName hasPrefix:@"iPad2,7"])
			model = "iPad Mini GSM CDMA LTE";

		if ([modelName hasPrefix:@"iPad3,1"])
			model = "iPad 3 WiFi";
		if ([modelName hasPrefix:@"iPad3,2"])
			model = "iPad 3 CDMA LTE";
		if ([modelName hasPrefix:@"iPad3,3"])
			model = "iPad 3 GSM LTE";

		if ([modelName hasPrefix:@"iPad3,4"])
			model = "iPad 4 WiFi";
		if ([modelName hasPrefix:@"iPad3,5"])
			model = "iPad 4 GSM LTE";
		if ([modelName hasPrefix:@"iPad3,6"])
			model = "iPad 4 CDMA LTE";

		if ([modelName hasPrefix:@"iPad4,1"])
			model = "iPad 5 WiFi";
		if ([modelName hasPrefix:@"iPad4,2"])
			model = "iPad 5 GSM CDMA LTE";

		if ([modelName hasPrefix:@"iPad4,4"])
			model = "iPad Mini 2 WiFi";
		if ([modelName hasPrefix:@"iPad4,5"])
			model = "iPad Mini 2 GSM CDMA LTE";

		// iPod
		if ([modelName hasPrefix:@"iPod1,1"])
			model = "iPod Touch";
		if ([modelName hasPrefix:@"iPod2,1"])
			model = "iPod Touch 2G";
		if ([modelName hasPrefix:@"iPod3,1"])
			model = "iPod Touch 3G";
		if ([modelName hasPrefix:@"iPod4,1"])
			model = "iPod Touch 4G";
		if ([modelName hasPrefix:@"iPod5,1"])
			model = "iPod Touch 5G";

		//AppleTV
		if ([modelName hasPrefix:@"AppleTV1,1"])
			model = "AppleTV";
		if ([modelName hasPrefix:@"AppleTV2,1"])
			model = "AppleTV 2G";
		if ([modelName hasPrefix:@"AppleTV3,1"])
			model = "AppleTV 3G early 2012";
		if ([modelName hasPrefix:@"AppleTV3,2"])
			model = "AppleTV 3G early 2013";
		
		if (model.empty())
		{
			// Unknown at this moment, return what is returned by system.
			model = [modelName UTF8String];
		}
	}

	return model;
}

String DeviceInfo::GetLocale()
{
	NSLocale *english = [[[NSLocale alloc] initWithLocaleIdentifier:@"en_US"] autorelease];

	NSString* langID = [[NSLocale preferredLanguages] objectAtIndex:0];
	NSString *lang = [english displayNameForKey:NSLocaleLanguageCode value:langID];

	String res = Format("%s (%s)", [langID UTF8String], [lang UTF8String]);
	return res;
}

String DeviceInfo::GetRegion()
{
	NSLocale *english = [[[NSLocale alloc] initWithLocaleIdentifier:@"en_US"] autorelease];

	NSString *countryCode = [[NSLocale currentLocale] objectForKey: NSLocaleCountryCode];
	NSString *country = [english displayNameForKey: NSLocaleCountryCode value: countryCode];

	String res = Format("%s (%s)", [countryCode UTF8String], [country UTF8String]);
	return res;
}

String DeviceInfo::GetTimeZone()
{
	NSTimeZone *localTime = [NSTimeZone systemTimeZone];
	return [[localTime name] UTF8String];
}
    
String DeviceInfo::GetUDID()
{
	bool hasAdvertisingId = (NSClassFromString(@"ASIdentifierManager") != nil);

	bool iOSLowerThan7 = false;
	NSString* version = [NSString stringWithCString:GetVersion().c_str()
										   encoding:[NSString defaultCStringEncoding]];
	if ([version compare:@"7.0" options:NSNumericSearch] == NSOrderedAscending)
	{
		iOSLowerThan7 = true;
	}

	NSString* udid = nil;
	if (iOSLowerThan7 || !hasAdvertisingId)
	{
		udid = @"";
	}
	else
	{
		udid = [[[ASIdentifierManager sharedManager] advertisingIdentifier] UUIDString];
	}

	return [udid UTF8String];
}
    
WideString DeviceInfo::GetName()
{
    NSString * deviceName = [[UIDevice currentDevice] name];
    
    NSStringEncoding pEncode    =   CFStringConvertEncodingToNSStringEncoding ( kCFStringEncodingUTF32LE );
    NSData* pSData              =   [ deviceName dataUsingEncoding : pEncode ];
    
    return WideString ( (wchar_t*) [ pSData bytes ], [ pSData length] / sizeof ( wchar_t ) );
}

// Not impletemted yet
String DeviceInfo::GetHTTPProxyHost()
{
	return String();
}

// Not impletemted yet
String DeviceInfo::GetHTTPNonProxyHosts()
{
	return String();
}

// Not impletemted yet
int DeviceInfo::GetHTTPProxyPort()
{
	return 0;
}
    
eGPUFamily DeviceInfo::GetGPUFamily()
{
    return GPU_POWERVR_IOS;
}
    
}

#endif
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


#include "DeviceInfo.h"

#if defined(__DAVAENGINE_IPHONE__)
#include "TargetConditionals.h"
#endif

namespace DAVA
{

DeviceInfo::ScreenInfo DeviceInfo::screenInfo;

DeviceInfo::ePlatform DeviceInfo::GetPlatform()
{
	ePlatform platform = PLATFORM_UNKNOWN;

#if defined(__DAVAENGINE_MACOS__)
	platform = PLATFORM_MACOS;

#elif defined(__DAVAENGINE_IPHONE__)
	platform = PLATFORM_IOS;
	#if defined(TARGET_IPHONE_SIMULATOR) && TARGET_IPHONE_SIMULATOR == 1
		platform = PLATFORM_IOS_SIMULATOR;
	#endif

#elif defined(__DAVAENGINE_ANDROID__)
	platform = PLATFORM_ANDROID;

#elif defined(__DAVAENGINE_WIN_UAP__)
    platform = PLATFORM_WIN_UAP;

#elif defined(__DAVAENGINE_WIN32__)
	platform = PLATFORM_WIN32;
#endif

	return platform;
}

String DeviceInfo::GetPlatformString()
{
    return GlobalEnumMap<ePlatform>::Instance()->ToString(GetPlatform());
}


DeviceInfo::ScreenInfo & DeviceInfo::GetScreenInfo()
{
	return screenInfo;
}

#ifndef __DAVAENGINE_ANDROID__
int DeviceInfo::GetZBufferSize()
{
	return 24;
}

List<DeviceInfo::StorageInfo> DeviceInfo::GetStoragesList()
{
    List<DeviceInfo::StorageInfo> l;
    return l;
}
    
#endif

#ifdef __DAVAENGINE_WINDOWS__

int32 DeviceInfo::GetCpuCount()
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
}

#endif

}

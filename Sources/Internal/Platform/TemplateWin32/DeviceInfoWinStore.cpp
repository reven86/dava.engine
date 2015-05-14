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
#include "Utils/StringFormat.h"
#include "Utils/MD5.h"
#include "Utils/Utils.h"
#include "Debug/DVAssert.h"

#if defined(__DAVAENGINE_WINDOWS_STORE__)

#include "winsock2.h"
#include "Iphlpapi.h"

using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::Graphics::Display;

namespace DAVA
{

String DeviceInfo::GetVersion()
{
    __DAVAENGINE_WINDOWS_STORE_INCOMPLETE_IMPLEMENTATION__
	return "Not yet implemented";
}

String DeviceInfo::GetManufacturer()
{
    __DAVAENGINE_WINDOWS_STORE_INCOMPLETE_IMPLEMENTATION__
	return "Not yet implemented";
}

String DeviceInfo::GetModel()
{
    __DAVAENGINE_WINDOWS_STORE_INCOMPLETE_IMPLEMENTATION__
	return "Not yet implemented";
}

String DeviceInfo::GetLocale()
{
    __DAVAENGINE_WINDOWS_STORE_INCOMPLETE_IMPLEMENTATION__
	return "Not yet implemented";
}

String DeviceInfo::GetRegion()
{
    __DAVAENGINE_WINDOWS_STORE_INCOMPLETE_IMPLEMENTATION__
	return "Not yet implemented";
}

String DeviceInfo::GetTimeZone()
{
    __DAVAENGINE_WINDOWS_STORE_INCOMPLETE_IMPLEMENTATION__
	return "Not yet implemented";
}
String DeviceInfo::GetHTTPProxyHost()
{
	return "Not yet implemented";
}

String DeviceInfo::GetHTTPNonProxyHosts()
{
    __DAVAENGINE_WINDOWS_STORE_INCOMPLETE_IMPLEMENTATION__
	return "Not yet implemented";
}

int DeviceInfo::GetHTTPProxyPort()
{
    __DAVAENGINE_WINDOWS_STORE_INCOMPLETE_IMPLEMENTATION__
	return 0;
}

String DeviceInfo::GetUDID()
{
    __DAVAENGINE_WINDOWS_STORE_INCOMPLETE_IMPLEMENTATION__
        return "Not yet implemented";
}

WideString DeviceInfo::GetName()
{
    __DAVAENGINE_WINDOWS_STORE_INCOMPLETE_IMPLEMENTATION__
    return L"Not yet implemented";
}

eGPUFamily DeviceInfo::GetGPUFamily()
{
    return GPU_INVALID;
}

DeviceInfo::NetworkInfo DeviceInfo::GetNetworkInfo()
{
    // For now return default network info for Windows.
    return NetworkInfo();
}

void DeviceInfo::InitializeScreenInfo()
{
	DeviceInfo::ComputeDisplayRotation();
}
//to::do add in public
void DeviceInfo::ComputeDisplayRotation()
{
	CoreWindow^ window = CoreWindow::GetForCurrentThread();
	Size localVal = Size(window->Bounds.Width, window->Bounds.Height);
	screenInfo.width = localVal.Width;
	screenInfo.height = localVal.Width;
	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
	DisplayOrientations currentOrientation = currentDisplayInformation->CurrentOrientation;
	switch (currentOrientation)
	{
	case DisplayOrientations::Landscape:
		screenInfo.scale = 1;

	case DisplayOrientations::Portrait:
		screenInfo.scale = 0;
	}
}

}

#endif // defined(__DAVAENGINE_WINDOWS_STORE__)
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

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN32__)

#include "Platform/DeviceInfo.h"
#include "Utils/StringFormat.h"
#include "Utils/MD5.h"
#include "Utils/Utils.h"
#include "Debug/DVAssert.h"
#include "Platform/TemplateWin32/DeviceInfoWin32.h"
#include "Base/GlobalEnum.h"
#include "winsock2.h"
#include "Iphlpapi.h"

namespace DAVA
{

DeviceInfoPrivate::DeviceInfoPrivate()
{
}

DeviceInfo::ePlatform DeviceInfoPrivate::GetPlatform()
{
    return DeviceInfo::PLATFORM_WIN32;
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
	return "Not yet implemented";
}

String DeviceInfoPrivate::GetModel()
{
	return "Not yet implemented";
}

String DeviceInfoPrivate::GetLocale()
{
	return "Not yet implemented";
}

String DeviceInfoPrivate::GetRegion()
{
	return "Not yet implemented";
}

String DeviceInfoPrivate::GetTimeZone()
{
	return "Not yet implemented";
}
String DeviceInfoPrivate::GetHTTPProxyHost()
{
	return "Not yet implemented";
}

String DeviceInfoPrivate::GetHTTPNonProxyHosts()
{
	return "Not yet implemented";
}

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

List<DeviceInfo::StorageInfo> DeviceInfoPrivate::GetStoragesList()
{
    List<DeviceInfo::StorageInfo> l;
    return l;
}

int32 DeviceInfoPrivate::GetCpuCount()
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}

String DeviceInfoPrivate::GetUDID()
{
    ULONG family = AF_INET;
    ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_UNICAST;

    PIP_ADAPTER_ADDRESSES buf;
    ULONG bufLength = 15000;

    DWORD retVal;

    do
    {
        buf = (PIP_ADAPTER_ADDRESSES)malloc(bufLength);
        retVal = GetAdaptersAddresses(family, flags, 0, buf, &bufLength);

        if (retVal == ERROR_BUFFER_OVERFLOW)
        {
            free(buf);
            bufLength *= 2;
        }
    } while (retVal == ERROR_BUFFER_OVERFLOW);

    String res = "";
    if (retVal == NO_ERROR)
    {
        PIP_ADAPTER_ADDRESSES curAddress = buf;

        if(curAddress)
        {
            if (curAddress->PhysicalAddressLength)
            {
                for (int32 i = 0; i < (int32)curAddress->PhysicalAddressLength; ++i)
                {
                    res += String(Format("%.2x", curAddress->PhysicalAddress[i]));
                }
            }
        }
    }

    if (buf)
    {
        free(buf);
    }

    if (res == "")
    {
        bool idOk = false;
        DWORD bufSize = 1024;
        LPBYTE buf = new BYTE[bufSize];
        HKEY key;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ, &key) == ERROR_SUCCESS)
        {

            if (RegQueryValueEx(key, L"MachineGuid", 0, 0, buf, &bufSize) == ERROR_SUCCESS)
            {
                idOk = true;
            }
            else
            {
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ | KEY_WOW64_64KEY,  &key) == ERROR_SUCCESS)
                {
                    if (RegQueryValueEx(key, L"MachineGuid", 0, 0, buf, &bufSize) == ERROR_SUCCESS)
                    {
                        idOk = true;
                    }
                }
            }
        }

        if (idOk)
        {
            WideString wstr = WideString((wchar_t*)buf);
            res = WStringToString(wstr);
        }
        else
        {
            DVASSERT(false && "Invalid UDID");
            res = "Invalid UDID";
        }
        SafeDeleteArray(buf);
    }

    MD5::MD5Digest md5Digest;
    MD5::ForData(reinterpret_cast<const uint8*>(res.c_str()), static_cast<uint32>(res.size()), md5Digest);

    String digest(MD5::MD5Digest::DIGEST_SIZE * 2 + 1, '\0');
    MD5::HashToChar(md5Digest, const_cast<char8*>(digest.data()), digest.size());
    return digest;
}

WideString DeviceInfoPrivate::GetName()
{
	//http://msdn.microsoft.com/en-us/library/windows/desktop/ms724295(v=vs.85).aspx
	char16 compName[MAX_COMPUTERNAME_LENGTH + 1];
	uint32 length = MAX_COMPUTERNAME_LENGTH + 1;

	bool nameRecieved = GetComputerNameW(compName, (LPDWORD) &length) != FALSE;
	if(nameRecieved)
	{
		return WideString(compName, length);
	}

    return WideString ();
}

eGPUFamily DeviceInfoPrivate::GetGPUFamily()
{
    return GPU_INVALID;
}

DeviceInfo::NetworkInfo DeviceInfoPrivate::GetNetworkInfo()
{
    // For now return default network info for Windows.
    return DeviceInfo::NetworkInfo();
}

void DeviceInfoPrivate::InitializeScreenInfo()
{
	screenInfo.width = ::GetSystemMetrics(SM_CXSCREEN);
	screenInfo.height = ::GetSystemMetrics(SM_CYSCREEN);
	screenInfo.scale = 1;
}

bool DeviceInfoPrivate::IsHIDConnected(DeviceInfo::eHIDType type)
{
        DVASSERT(false && "Not Implement");
        return false;
}

void DeviceInfoPrivate::SetHIDConnectionCallback(DeviceInfo::eHIDType type, DeviceInfo::HIDCallBackFunc&& callback)
{
        DVASSERT(false && "Not Implement");
}

}

#endif // defined(__DAVAENGINE_WIN32__)

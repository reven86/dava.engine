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


#include "Debug/DVAssert.h"
#include "FileSystem/FileSystem.h"
#include "Platform/DeviceInfo.h"
#include "Utils/MD5.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Iphlpapi.h"
#include "winsock2.h"

using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::Graphics::Display;

namespace DAVA
{

String DeviceInfo::GetVersion()
{
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
	return "Not yet implemented";
}

String DeviceInfo::GetManufacturer()
{
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
	return "Not yet implemented";
}

String DeviceInfo::GetModel()
{
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
	return "Not yet implemented";
}

String DeviceInfo::GetLocale()
{
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
	return "Not yet implemented";
}

String DeviceInfo::GetRegion()
{
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
	return "Not yet implemented";
}

String DeviceInfo::GetTimeZone()
{
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
	return "Not yet implemented";
}
String DeviceInfo::GetHTTPProxyHost()
{
	return "Not yet implemented";
}

String DeviceInfo::GetHTTPNonProxyHosts()
{
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
	return "Not yet implemented";
}

int DeviceInfo::GetHTTPProxyPort()
{
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
	return 0;
}

String DeviceInfo::GetUDID()
{
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
        return "Not yet implemented";
}

WideString DeviceInfo::GetName()
{
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
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

#if defined (__DAVAENGINE_WIN_UAP__)
void DeviceInfo::InitializeScreenInfo(int32 width, int32 height)
{
    screenInfo.width = width;
    screenInfo.height = height;
}
#elif //  __DAVAENGINE_WIN_UAP__
void DeviceInfo::InitializeScreenInfo()
{
}
#endif

#if defined (__DAVAENGINE_WIN_UAP__)
bool DeviceInfo::IsRunningOnEmulator()
{
    using namespace Windows::Security::ExchangeActiveSyncProvisioning;
    EasClientDeviceInformation deviceInfo;
    bool isEmulator = ("Virtual" == deviceInfo.SystemProductName);
    return isEmulator;
}
#endif //  __DAVAENGINE_WIN_UAP__

bool FillStorageSpaceInfo(DeviceInfo::StorageInfo& storage_info)
{
    ULARGE_INTEGER freeBytesAvailable;
    ULARGE_INTEGER totalNumberOfBytes;
    ULARGE_INTEGER totalNumberOfFreeBytes;

    BOOL res = ::GetDiskFreeSpaceExA(storage_info.path.GetAbsolutePathname().c_str(),
        &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes);

    if (res == FALSE)
        return false;

    storage_info.totalSpace = totalNumberOfBytes.QuadPart;
    storage_info.freeSpace = freeBytesAvailable.QuadPart;

    return true;
}

List<DeviceInfo::StorageInfo> DeviceInfo::GetStoragesList()
{
    using namespace Windows::Storage;

    List<DeviceInfo::StorageInfo> result;
    FileSystem* fileSystem = FileSystem::Instance();

    //information about internal storage
    DeviceInfo::StorageInfo storage;
    storage.path = fileSystem->GetUserDocumentsPath();
    storage.type = STORAGE_TYPE_INTERNAL;
    if (FillStorageSpaceInfo(storage))
    {
        result.push_back(storage);
    }

    //information about removable storages
    storage.type = STORAGE_TYPE_PRIMARY_EXTERNAL;
    storage.removable = true;

    auto removableStorages = WaitAsync(KnownFolders::RemovableDevices->GetFoldersAsync());
    size_t size = removableStorages->Size;
    for (size_t i = 0; i < size; ++i)
    {
        auto path = removableStorages->GetAt(i)->Path;
        storage.path = WStringToString(path->Data());
        if (FillStorageSpaceInfo(storage))
        {
            result.push_back(storage);
            //all subsequent external storages are secondary
            storage.type = STORAGE_TYPE_SECONDARY_EXTERNAL;
        }
    }

    return result;
}

}

#endif // defined(__DAVAENGINE_WIN_UAP__)

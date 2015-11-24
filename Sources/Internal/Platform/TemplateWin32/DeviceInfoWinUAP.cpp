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

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include <Iphlpapi.h>
#include <winsock2.h>

#include "Debug/DVAssert.h"
#include "FileSystem/FileSystem.h"
#include "Utils/MD5.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"
#include "Base/GlobalEnum.h"

#include "Platform/TemplateWin32/DeviceInfoWinUAP.h"

__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"

using namespace ::Windows::UI::Core;
using namespace ::Windows::Graphics::Display;
using namespace ::Windows::Devices::Input;
using namespace ::Windows::UI::ViewManagement;
using namespace ::Windows::Foundation;
using namespace ::Windows::Devices;
using namespace ::Windows::Devices::Enumeration;
using namespace ::Windows::Devices::HumanInterfaceDevice;
using namespace ::Windows::Security::ExchangeActiveSyncProvisioning;
using namespace ::Windows::Networking::Connectivity;
using namespace ::Windows::System::UserProfile;
using namespace ::Windows::UI::Xaml;
using namespace ::Windows::System::Profile;
using namespace ::Windows::Globalization;

namespace DAVA
{

DeviceInfoPrivate::DeviceInfoPrivate()
{
    TouchCapabilities touchCapabilities;
    isTouchPresent = (1 == touchCapabilities.TouchPresent); //  Touch is always present in MSVS simulator
    isMobileMode = Windows::Foundation::Metadata::ApiInformation::IsApiContractPresent("Windows.Phone.PhoneContract", 1);
    platform = isMobileMode ? DeviceInfo::PLATFORM_PHONE_WIN_UAP : DeviceInfo::PLATFORM_DESKTOP_WIN_UAP;

    AnalyticsVersionInfo ^ versionInfo = AnalyticsInfo::VersionInfo;
    Platform::String ^ deviceVersion = versionInfo->DeviceFamilyVersion;
    Platform::String ^ deviceFamily = versionInfo->DeviceFamily;
    String vertionString = RTStringToString(deviceVersion);
    int64 versionInt = _atoi64(vertionString.c_str());
    std::stringstream versionStream;
    versionStream << ((versionInt & 0xFFFF000000000000L) >> 48) << ".";
    versionStream << ((versionInt & 0x0000FFFF00000000L) >> 32) << ".";
    versionStream << ((versionInt & 0x00000000FFFF0000L) >> 16) << ".";
    versionStream << (versionInt & 0x000000000000FFFFL);
    version = versionStream.str();
    platformString = RTStringToString(versionInfo->DeviceFamily);

    EasClientDeviceInformation deviceInfo;
    manufacturer = RTStringToString(deviceInfo.SystemManufacturer);
    modelName = RTStringToString(deviceInfo.SystemSku);
    localDeviceName = RTStringToString(deviceInfo.FriendlyName);
    deviceName = WideString(deviceInfo.FriendlyName->Data());
    gpu = GPUFamily();
    uDID = RTStringToString(Windows::System::UserProfile::AdvertisingManager::AdvertisingId);
}

DeviceInfo::ePlatform DeviceInfoPrivate::GetPlatform()
{
    return platform;
}

String DeviceInfoPrivate::GetPlatformString()
{
    return platformString;
}

String DeviceInfoPrivate::GetVersion()
{
    return version;
}

String DeviceInfoPrivate::GetManufacturer()
{
    return manufacturer;
}

String DeviceInfoPrivate::GetModel()
{
    return modelName;
}

String DeviceInfoPrivate::GetLocale()
{
    return RTStringToString(GlobalizationPreferences::Languages->GetAt(0));
}

String DeviceInfoPrivate::GetRegion()
{
    return RTStringToString(GlobalizationPreferences::HomeGeographicRegion);
}

String DeviceInfoPrivate::GetTimeZone()
{
    Calendar calendar;
    return RTStringToString(calendar.GetTimeZone());
}

String DeviceInfoPrivate::GetHTTPProxyHost()
{
    return "Not yet implemented";
}

String DeviceInfoPrivate::GetHTTPNonProxyHosts()
{
    return "Not yet implemented";
}

int DeviceInfoPrivate::GetHTTPProxyPort()
{
    return 0;
}

DeviceInfo::ScreenInfo& DeviceInfoPrivate::GetScreenInfo()
{
    return screenInfo;
}

int DeviceInfoPrivate::GetZBufferSize()
{
    return zBufferSize;
}

String DeviceInfoPrivate::GetUDID()
{
    return uDID;
}

WideString DeviceInfoPrivate::GetName()
{
    return deviceName;
}

eGPUFamily DeviceInfoPrivate::GetGPUFamily()
{   
    return gpu;
}

DeviceInfo::NetworkInfo DeviceInfoPrivate::GetNetworkInfo()
{
    DeviceInfo::NetworkInfo networkInfo;
    ConnectionProfile^ icp = NetworkInformation::GetInternetConnectionProfile();
    if (icp != nullptr && icp->NetworkAdapter != nullptr)
    {
        if (icp->IsWlanConnectionProfile)
        {
            networkInfo.networkType = DeviceInfo::NETWORK_TYPE_WIFI;
        }
        else if (icp->IsWwanConnectionProfile)
        {
            networkInfo.networkType = DeviceInfo::NETWORK_TYPE_CELLULAR;
        }
        else
        {
            // in other case Ethernet
            networkInfo.networkType = DeviceInfo::NETWORK_TYPE_ETHERNET;
        }
    }
    return networkInfo;
}

// temporary decision
void DeviceInfoPrivate::InitializeScreenInfo()
{
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__
    
    CorePlatformWinUAP* core = static_cast<CorePlatformWinUAP*>(Core::Instance());
    DVASSERT(nullptr != core && "DeviceInfo::InitializeScreenInfo(): Core::Instance() is null");

    auto func = [this]() {
        // should be started on UI thread
        CoreWindow ^ coreWindow = Window::Current->CoreWindow;
        DVASSERT(coreWindow != nullptr);

        screenInfo.width = static_cast<int32>(coreWindow->Bounds.Width);
        screenInfo.height = static_cast<int32>(coreWindow->Bounds.Height);

        DisplayInformation^ displayInfo = DisplayInformation::GetForCurrentView();
        DVASSERT(displayInfo != nullptr);
        screenInfo.scale = static_cast<float32>(displayInfo->RawPixelsPerViewPixel);
        DisplayOrientations curOrientation = DisplayInformation::GetForCurrentView()->CurrentOrientation;
        if (DisplayOrientations::Portrait == curOrientation || DisplayOrientations::PortraitFlipped == curOrientation)
        {
            std::swap(screenInfo.width, screenInfo.height);
        }
    };
    core->RunOnUIThreadBlocked(func);
    // start device watchers, after creation main thread dispatcher
    CreateAndStartHIDWatcher();
}

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

List<DeviceInfo::StorageInfo> DeviceInfoPrivate::GetStoragesList()
{
    using namespace Windows::Storage;

    List<DeviceInfo::StorageInfo> result;
    FileSystem* fileSystem = FileSystem::Instance();

    //information about internal storage
    DeviceInfo::StorageInfo storage;
    storage.path = fileSystem->GetUserDocumentsPath();
    storage.type = DeviceInfo::STORAGE_TYPE_INTERNAL;
    if (FillStorageSpaceInfo(storage))
    {
        result.push_back(storage);
    }

    //information about removable storages
    storage.type = DeviceInfo::STORAGE_TYPE_PRIMARY_EXTERNAL;
    storage.removable = true;

    auto removableStorages = WaitAsync(KnownFolders::RemovableDevices->GetFoldersAsync());
    for (unsigned i = 0; i < removableStorages->Size; ++i)
    {
        Platform::String^ path = removableStorages->GetAt(i)->Path;
        storage.path = WStringToString(path->Data());
        if (FillStorageSpaceInfo(storage))
        {
            result.push_back(storage);
            //all subsequent external storages are secondary
            storage.type = DeviceInfo::STORAGE_TYPE_SECONDARY_EXTERNAL;
        }
    }

    return result;
}

bool DeviceInfoPrivate::IsHIDConnected(DeviceInfo::eHIDType type)
{
    auto func = [type](HIDConvPair pair)->bool {
        return pair.second == type;
    };
    auto it = std::find_if(HidConvSet.begin(), HidConvSet.end(), func);
    return IsEnabled(it->first);
}

bool DeviceInfoPrivate::IsTouchPresented()
{
    return isTouchPresent; //  Touch is always present in MSVS simulator
}

void DeviceInfoPrivate::NotifyAllClients(NativeHIDType type, bool isConnected)
{
    auto func = [type](HIDConvPair pair)->bool {
        return pair.first == type;
    };
    DeviceInfo::eHIDType hidType = std::find_if(HidConvSet.begin(), HidConvSet.end(), func)->second;

    //pass notification in main thread
    CorePlatformWinUAP* core = static_cast<CorePlatformWinUAP*>(Core::Instance());

    DeviceInfo::HIDConnectionSignal* signal = &GetHIDConnectionSignal(hidType);
    core->RunOnMainThread([=] { signal->Emit(hidType, isConnected); });
}

eGPUFamily DeviceInfoPrivate::GPUFamily()
{
    return GPU_DX11;
}

DeviceWatcher^ DeviceInfoPrivate::CreateDeviceWatcher(NativeHIDType type)
{
    DeviceWatcher^ watcher = DeviceInformation::CreateWatcher(HidDevice::GetDeviceSelector(USAGE_PAGE, type));
    auto added = ref new TypedEventHandler<DeviceWatcher^, DeviceInformation^>([this, type](DeviceWatcher^ watcher, DeviceInformation^ information) {
        OnDeviceAdded(type, information);
    });
    auto removed = ref new TypedEventHandler<DeviceWatcher^ , DeviceInformationUpdate^>([this, type](DeviceWatcher^ watcher, DeviceInformationUpdate^ information) {
        OnDeviceRemoved(type, information);
    });

    watcher->Added += added;
    watcher->Removed += removed;
    watcher->Start();
    return watcher;
}

void DeviceInfoPrivate::CreateAndStartHIDWatcher()
{
    watchers.emplace_back(CreateDeviceWatcher(POINTER));
    watchers.emplace_back(CreateDeviceWatcher(MOUSE));
    watchers.emplace_back(CreateDeviceWatcher(JOYSTICK));
    watchers.emplace_back(CreateDeviceWatcher(GAMEPAD));
    watchers.emplace_back(CreateDeviceWatcher(KEYBOARD));
    watchers.emplace_back(CreateDeviceWatcher(KEYPAD));
    watchers.emplace_back(CreateDeviceWatcher(SYSTEM_CONTROL));
}

void DeviceInfoPrivate::OnDeviceAdded(NativeHIDType type, DeviceInformation^ information)
{
    if (isTouchPresent)
    {
        // skip because Windows touch mimics under mouse and keyboard
        if (localDeviceName.compare(RTStringToString(information->Name)) == 0)
        {
            return;
        }
    }
    
    auto it = hids.find(type);
    if (it != hids.end())
    {
        it->second++;
        NotifyAllClients(type, true);
    }
}

void DeviceInfoPrivate::OnDeviceRemoved(NativeHIDType type, DeviceInformationUpdate^ information)
{
    auto it = hids.find(type);
    if (it != hids.end())
    {
        it->second--;
        NotifyAllClients(type, false);
    }
}

bool DeviceInfoPrivate::IsEnabled(NativeHIDType type)
{
    return (hids[type] > 0);
}

}

#endif // defined(__DAVAENGINE_WIN_UAP__)

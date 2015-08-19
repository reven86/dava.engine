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

#if defined(__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__)
#include <GLES2/gl2.h>
#endif //  (__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__)

#include <Iphlpapi.h>
#include <winsock2.h>

#include "Debug/DVAssert.h"
#include "FileSystem/FileSystem.h"
#include "Utils/MD5.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"
#include "Base/GlobalEnum.h"

#include "Platform/TemplateWin32/DeviceInfoWinUAP.h"

#if defined(__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__)
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#endif //  (__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__)

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

namespace DAVA
{

DeviceInfoPrivate::DeviceInfoPrivate()
{
    TouchCapabilities touchCapabilities;
    isTouchPresent = (1 == touchCapabilities.TouchPresent); //  Touch is always present in MSVS simulator
    isMobileMode = Windows::Foundation::Metadata::ApiInformation::IsApiContractPresent("Windows.Phone.PhoneContract", 1);

    platform = isMobileMode ? DeviceInfo::PLATFORM_PHONE_WIN_UAP : DeviceInfo::PLATFORM_DESKTOP_WIN_UAP;
    platformString = GlobalEnumMap<DeviceInfo::ePlatform>::Instance()->ToString(GetPlatform());

    try
    {
        EasClientDeviceInformation deviceInfo;
        version.swap(RTStringToString(deviceInfo.SystemFirmwareVersion));
        manufacturer.swap(RTStringToString(deviceInfo.SystemManufacturer));
        modelName.swap(RTStringToString(deviceInfo.FriendlyName));
        productName = WideString(deviceInfo.SystemProductName->Data());
        gpu = GPUFamily();
        uDID.swap(RTStringToString(deviceInfo.Id.ToString()));
    }
    catch (Platform::Exception^ e)
    {
    }
    cpuCount = static_cast<int32>(std::thread::hardware_concurrency());
    if (0 == cpuCount)
    {
        cpuCount = 1;
    }

    vectorWatchers.emplace_back(WatcherForDeviceEvents(AQS_POINTER));
    vectorWatchers.emplace_back(WatcherForDeviceEvents(AQS_MOUSE));
    vectorWatchers.emplace_back(WatcherForDeviceEvents(AQS_JOYSTICK));
    vectorWatchers.emplace_back(WatcherForDeviceEvents(AQS_GAMEPAD));
    vectorWatchers.emplace_back(WatcherForDeviceEvents(AQS_KEYBOARD));
    vectorWatchers.emplace_back(WatcherForDeviceEvents(AQS_KEYPAD));
    vectorWatchers.emplace_back(WatcherForDeviceEvents(AQS_SYSTEM_CONTROL));
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
    // TO::DO uncomment after add Windows.System.SystemManagementContract
    // RTStringToString(Windows::System::TimeZoneSettings::CurrentTimeZoneDisplayName);
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
    return productName;
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
#if defined(__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__)
    int32 w = 0, h = 0;
    CorePlatformWinUAP* core = static_cast<CorePlatformWinUAP*>(Core::Instance());
    DVASSERT(nullptr != core && "In DeviceInfo, InitializeScreenInfo() function CorePlatformWinUAP* = nullptr");
    auto func = [&w, &h]()
    {
        CoreWindow^ window = CoreWindow::GetForCurrentThread();
        if (nullptr != window)
        {
            w = static_cast<int32>(window->Bounds.Width);
            h = static_cast<int32>(window->Bounds.Height);
            DisplayOrientations current = DisplayInformation::GetForCurrentView()->CurrentOrientation;
            if (DisplayOrientations::Portrait == current || DisplayOrientations::PortraitFlipped == current)
            {
                std::swap(w, h);
            }
        }
    };
    core->RunOnUIThreadBlocked(func);
    screenInfo.width = w;
    screenInfo.height = h;
#endif
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
    size_t size = removableStorages->Size;
    for (size_t i = 0; i < size; ++i)
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

int32 DeviceInfoPrivate::GetCpuCount()
{
    return cpuCount;
}

bool DeviceInfoPrivate::IsHIDConnected(DeviceInfo::eHIDType hid)
{
    return IsEnabled(std::find_if(unionAqsAndHid.begin(), unionAqsAndHid.end(), [hid](AqsHidPair pair)->bool {
        return pair.second == hid; 
    })->first);
}

// warning!!! notification occur in DeviceWatcher's thread
// for notify in main's thread use MainThreadRedirector
// it pass call from Watcher's thread in main
// for example DeviceInfo::SubscribeHID(DeviceInfo::eHIDType::HID_MOUSE_TYPE, MainThreadRedirector([this](int32 a, bool b) { OnMouseAdd(a, b);}));
void DeviceInfoPrivate::SubscribeHID(DeviceInfo::eHIDType hid, DeviceInfo::HIDCallBackFunc&& func)
{
    AQSyntax aqsId = std::find_if(unionAqsAndHid.begin(), unionAqsAndHid.end(), [hid](AqsHidPair pair)->bool {
        return pair.second == hid;
    })->first;
    deviceTypes[aqsId].callbacks.emplace_back(std::forward<DeviceInfo::HIDCallBackFunc>(func));
}

void DeviceInfoPrivate::NotifyAllClients(AQSyntax usageId, bool connectState)
{
    for (auto iter : (deviceTypes[usageId].callbacks))
    {
        (iter)(std::find_if(unionAqsAndHid.begin(), unionAqsAndHid.end(), [usageId](AqsHidPair pair)->bool {
            return pair.first == usageId;
        })->second, connectState);
    }
}

eGPUFamily DeviceInfoPrivate::GPUFamily()
{
    eGPUFamily gpuFamily(GPU_INVALID);
#if defined(__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__)
    const GLubyte* rendererTemp = glGetString(GL_RENDERER);
    if (nullptr == rendererTemp)
    {
        DVASSERT(false && "GL not initialized");
        return gpuFamily;
    }
    String renderer((const char8 *)rendererTemp);
    std::transform(renderer.begin(), renderer.end(), renderer.begin(), ::tolower);
    if (renderer.find("tegra") != String::npos)
    {
        gpuFamily = GPU_TEGRA;
    }
    else if (renderer.find("powervr") != String::npos)
    {
        gpuFamily = GPU_POWERVR_ANDROID;
    }
    else if (renderer.find("adreno") != String::npos)
    {
        gpuFamily = GPU_ADRENO;
    }
    else if (renderer.find("mali") != String::npos)
    {
        gpuFamily = GPU_MALI;
    }

    if (gpuFamily == GPU_INVALID)
    {
        const GLubyte* extensionsTemp = glGetString(GL_EXTENSIONS);
        if (nullptr == extensionsTemp)
        {
            DVASSERT(false && "GL not initialized");
            return gpuFamily;
        }
        String extensions((const char8 *)extensionsTemp);

        if (extensions.find("GL_IMG_texture_compression_pvrtc") != String::npos)
            gpuFamily = GPU_POWERVR_ANDROID;
        else if (extensions.find("GL_NV_draw_texture") >= 0)
            gpuFamily = GPU_TEGRA;
        else if (extensions.find("GL_AMD_compressed_ATC_texture") != String::npos)
            gpuFamily = GPU_ADRENO;
        else if (extensions.find("GL_OES_compressed_ETC1_RGB8_texture") != String::npos)
            gpuFamily = GPU_MALI;
    }
    return gpuFamily;
#else
        return gpuFamily;
#endif //  (__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__)
}

bool DeviceInfoPrivate::CompareWithModelName(Platform::String^ str)
{
    return (modelName.compare(RTStringToString(str)) == 0);
}

DeviceWatcher^ DeviceInfoPrivate::WatcherForDeviceEvents(AQSyntax usageId)
{
    DeviceWatcher^ watcher = DeviceInformation::CreateWatcher(HidDevice::GetDeviceSelector(AQS_USAGE_PAGE, usageId));
    auto added = ref new TypedEventHandler<DeviceWatcher^, DeviceInformation^>([this, usageId](DeviceWatcher^ watcher, DeviceInformation^ information) {
        OnDeviceAdded(usageId, information);
    });
    auto removed = ref new TypedEventHandler<DeviceWatcher^ , DeviceInformationUpdate^>([this, usageId](DeviceWatcher^ watcher, DeviceInformationUpdate^ information) {
        OnDeviceRemoved(usageId, information);
    });

    watcher->Added += added;
    watcher->Removed += removed;
    watcher->Start();
    return watcher;
}

void DeviceInfoPrivate::OnDeviceAdded(AQSyntax usageId, DeviceInformation^ information)
{
    if (isTouchPresent)
    {
        // skip because Windows touch mimics under mouse and keyboard
        if (CompareWithModelName(information->Name))
        {
            return;
        }
    }
    
    auto it = deviceTypes.find(usageId);
    if (it != deviceTypes.end())
    {
        it->second.deviceInfo[information->Id] = information->Name;
        NotifyAllClients(usageId, true);
    }
}

void DeviceInfoPrivate::OnDeviceRemoved(AQSyntax usageId, DeviceInformationUpdate^ information)
{
    auto it = deviceTypes.find(usageId);
    if (it != deviceTypes.end())
    {
        it->second.deviceInfo.erase(information->Id);
        NotifyAllClients(usageId, false);
    }
}

bool DeviceInfoPrivate::IsEnabled(AQSyntax usageId)
{
    return (deviceTypes[usageId].deviceInfo.size() > 0);
}

}

#endif // defined(__DAVAENGINE_WIN_UAP__)

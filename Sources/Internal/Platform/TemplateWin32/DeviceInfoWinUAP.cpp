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

#include <GLES2/gl2.h>
#include <Iphlpapi.h>
#include <winsock2.h>

#include "Debug/DVAssert.h"
#include "FileSystem/FileSystem.h"
#include "Utils/MD5.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"

#include "Platform/TemplateWin32/DeviceInfoWinUAP.h"
// temporary decision
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

namespace DAVA
{

DeviceInfoPrivate::DeviceInfoPrivate()
{
    TouchCapabilities touchCapabilities;
    isTouchPresent = (1 == touchCapabilities.TouchPresent); //  Touch is always present in MSVS simulator
                                                            //add watchers
    mapWatchers.insert(PairForWatchersAndType(WatcherForDeviceEvents(AQS_USAGE_PAGE, AQS_POINTER), AQS_POINTER));
    mapWatchers.insert(PairForWatchersAndType(WatcherForDeviceEvents(AQS_USAGE_PAGE, AQS_MOUSE), AQS_MOUSE));
    mapWatchers.insert(PairForWatchersAndType(WatcherForDeviceEvents(AQS_USAGE_PAGE, AQS_JOYSTICK), AQS_JOYSTICK));
    mapWatchers.insert(PairForWatchersAndType(WatcherForDeviceEvents(AQS_USAGE_PAGE, AQS_GAMEPAD), AQS_GAMEPAD));
    mapWatchers.insert(PairForWatchersAndType(WatcherForDeviceEvents(AQS_USAGE_PAGE, AQS_KEYBOARD), AQS_KEYBOARD));
    mapWatchers.insert(PairForWatchersAndType(WatcherForDeviceEvents(AQS_USAGE_PAGE, AQS_KEYPAD), AQS_KEYPAD));
    mapWatchers.insert(PairForWatchersAndType(WatcherForDeviceEvents(AQS_USAGE_PAGE, AQS_SYSTEM_CONTROL), AQS_SYSTEM_CONTROL));
}

DeviceInfo::ePlatform DeviceInfoPrivate::GetPlatform()
{
    if (IsMobileMode())
    {
        return DeviceInfo::PLATFORM_PHONE_WIN_UAP;
    }
    return DeviceInfo::PLATFORM_WIN_UAP;
}

String DeviceInfoPrivate::GetPlatformString()
{
    Logger::FrameworkDebug("[DeviceInfoPrivate::GetPlatformString] \"%s\".", GlobalEnumMap<DeviceInfo::ePlatform>::Instance()->ToString(GetPlatform()));
    return GlobalEnumMap<DeviceInfo::ePlatform>::Instance()->ToString(GetPlatform());
}

String DeviceInfoPrivate::GetVersion()
{
    EasClientDeviceInformation deviceInfo;
    Logger::FrameworkDebug("[DeviceInfoPrivate::GetVersion] \"%s\".", RTStringToString(deviceInfo.SystemFirmwareVersion).c_str());
    return RTStringToString(deviceInfo.SystemFirmwareVersion);
}

String DeviceInfoPrivate::GetManufacturer()
{
    EasClientDeviceInformation deviceInfo;
    Logger::FrameworkDebug("[DeviceInfoPrivate::GetManufacturer] \"%s\".", RTStringToString(deviceInfo.SystemManufacturer).c_str());
    return RTStringToString(deviceInfo.SystemManufacturer);
}

String DeviceInfoPrivate::GetModel()
{
    EasClientDeviceInformation deviceInfo;
    Logger::FrameworkDebug("[DeviceInfoPrivate::GetModel] \"%s\".", RTStringToString(deviceInfo.FriendlyName).c_str());
    return RTStringToString(deviceInfo.FriendlyName);
}

String DeviceInfoPrivate::GetLocale()
{
    Logger::FrameworkDebug("[DeviceInfoPrivate::GetLocale] \"%s\".", RTStringToString(GlobalizationPreferences::Languages->GetAt(0)).c_str());
    return RTStringToString(GlobalizationPreferences::Languages->GetAt(0));
}

String DeviceInfoPrivate::GetRegion()
{
    Logger::FrameworkDebug("[DeviceInfoPrivate::GetRegion] \"%s\".", RTStringToString(GlobalizationPreferences::HomeGeographicRegion).c_str());
    return RTStringToString(GlobalizationPreferences::HomeGeographicRegion);
}

String DeviceInfoPrivate::GetTimeZone()
{
    //uncomment after add Windows.System.SystemManagementContract
    // Logger::FrameworkDebug("[DeviceInfoPrivate::GetTimeZone] \"%s\".", RTStringToString(Windows::System::TimeZoneSettings::CurrentTimeZoneDisplayName).c_str());
    // return RTStringToString(Windows::System::TimeZoneSettings::CurrentTimeZoneDisplayName);
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
    return 24;
}

String DeviceInfoPrivate::GetUDID()
{
    Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation deviceInfo;
    Logger::FrameworkDebug("[DeviceInfoPrivate::GetUDID] \"%s\".", RTStringToString(deviceInfo.Id.ToString()).c_str());
    return RTStringToString(deviceInfo.Id.ToString());
}

WideString DeviceInfoPrivate::GetName()
{
    EasClientDeviceInformation deviceInfo;
    Logger::FrameworkDebug("[DeviceInfoPrivate::GetModel] \"%s\".", RTStringToString(deviceInfo.SystemProductName).c_str());
    return deviceInfo.SystemProductName->Data();
}

eGPUFamily DeviceInfoPrivate::GetGPUFamily()
{
    eGPUFamily gpuFamily(GPU_INVALID);
    auto rendererTemp = glGetString(GL_RENDERER);
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
        auto extensionsTemp = glGetString(GL_EXTENSIONS);
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
    int32 w = 0, h = 0;
    CorePlatformWinUAP* core = static_cast<CorePlatformWinUAP*>(Core::Instance());
    DVASSERT(nullptr != core && "In DeviceInfo, InitializeScreenInfo() function CorePlatformWinUAP* = nullptr");
    auto func = [&w, &h]()
    {
        auto window = CoreWindow::GetForCurrentThread();
        if (nullptr != window)
        {
            w = static_cast<int32>(Max(window->Bounds.Width, window->Bounds.Height));
            h = static_cast<int32>(Min(window->Bounds.Width, window->Bounds.Height));
        }
    };
    core->RunOnUIThreadBlocked(func);
    screenInfo.width = w;
    screenInfo.height = h;
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
        auto path = removableStorages->GetAt(i)->Path;
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
    int32 cpuCount = static_cast<int32>(std::thread::hardware_concurrency());
    return cpuCount == 0 ? 1 : cpuCount;
}

bool DeviceInfoPrivate::IsHIDConnected(DeviceInfo::eHIDType hid)
{
    return IsEnabled(ConvertHIDToAQS(hid));
}

// warning!!! notification occur in DeviceWatcher's thread
// for notify in main's thread use MainThreadRedirector
// it pass call from Watcher's thread in main
// for example DeviceInfo::SubscribeHID(DeviceInfo::eHIDType::HID_MOUSE_TYPE, MainThreadRedirector([this](int32 a, bool b) { OnMouseAdd(a, b);}));
void DeviceInfoPrivate::SubscribeHID(DeviceInfo::eHIDType hid, DeviceInfo::HIDCallBackFunc&& func)
{
    connections[ConvertHIDToAQS(hid)].emplace_back(std::forward<DeviceInfo::HIDCallBackFunc>(func));
}

bool DeviceInfoPrivate::IsMobileMode()
{
    return Windows::Foundation::Metadata::ApiInformation::IsApiContractPresent("Windows.Phone.PhoneContract", 1);
}

bool DeviceInfoPrivate::IsRunningOnEmulator()
{
    Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation deviceInfo;
    return ("Virtual" == deviceInfo.SystemProductName);
}

DeviceInfoPrivate::AQSyntax DeviceInfoPrivate::ConvertHIDToAQS(DeviceInfo::eHIDType hid)
{
    AQSyntax aqsType(AQS_UNKNOWN);
    switch (hid)
    {
    case DeviceInfo::HID_POINTER_TYPE:
        aqsType = AQS_POINTER;
        break;
    case DeviceInfo::HID_MOUSE_TYPE:
        aqsType = AQS_MOUSE;
        break;
    case DeviceInfo::HID_JOYSTICK_TYPE:
        aqsType = AQS_JOYSTICK;
        break;
    case DeviceInfo::HID_GAMEPAD_TYPE:
        aqsType = AQS_GAMEPAD;
        break;
    case DeviceInfo::HID_KEYBOARD_TYPE:
        aqsType = AQS_KEYBOARD;
        break;
    case DeviceInfo::HID_KEYPAD_TYPE:
        aqsType = AQS_KEYPAD;
        break;
    case DeviceInfo::HID_SYSTEM_CONTROL_TYPE:
        aqsType = AQS_SYSTEM_CONTROL;
        break;
    default:
        DVASSERT(false && "DeviceDetector ( HID_UNKNOWN_TYPE )");
    }
    return aqsType;
}

DeviceInfo::eHIDType DeviceInfoPrivate::ConvertAQSToHID(AQSyntax aqs)
{
    DeviceInfo::eHIDType hid(DeviceInfo::HID_UNKNOWN_TYPE);
    switch (aqs)
    {
    case AQS_POINTER:
        hid = DeviceInfo::HID_POINTER_TYPE;
    	break;
    case AQS_MOUSE:
        hid = DeviceInfo::HID_MOUSE_TYPE;
        break;
    case AQS_JOYSTICK:
        hid = DeviceInfo::HID_JOYSTICK_TYPE;
        break;
    case AQS_GAMEPAD:
        hid = DeviceInfo::HID_GAMEPAD_TYPE;
        break;
    case AQS_KEYBOARD:
        hid = DeviceInfo::HID_KEYBOARD_TYPE;
        break;
    case AQS_KEYPAD:
        hid = DeviceInfo::HID_KEYPAD_TYPE;
        break;
    case AQS_SYSTEM_CONTROL:
        hid = DeviceInfo::HID_SYSTEM_CONTROL_TYPE;
        break;
    default:
        DVASSERT(false && "DeviceDetector ( HID_AQS_TYPE )");
    }
    return hid;
}

void DeviceInfoPrivate::NotifyAllClients(AQSyntax usageId, bool connectState)
{
    MapForTypeAndConnections::iterator itForTypes = connections.find(usageId);
    if (itForTypes == connections.end())
    {
        return;
    }
    for (auto iter : (itForTypes->second))
    {
        (iter)(ConvertAQSToHID(usageId), connectState);
    }
}

DeviceWatcher^ DeviceInfoPrivate::WatcherForDeviceEvents(uint16 usagePage, uint16 usageId)
{
    Windows::Devices::Enumeration::DeviceWatcher^ watcher = Enumeration::DeviceInformation::CreateWatcher(HidDevice::GetDeviceSelector(usagePage, usageId));
    auto added = ref new TypedEventHandler<DeviceWatcher^, DeviceInformation^>([this](DeviceWatcher^ watcher, DeviceInformation^ information) {
        OnDeviceAdded(watcher, information);
    });
    auto removed = ref new TypedEventHandler<DeviceWatcher^ , DeviceInformationUpdate^>([this](DeviceWatcher^ watcher, DeviceInformationUpdate^ information) {
        OnDeviceRemoved(watcher, information);
    });

    watcher->Added += added;
    watcher->Removed += removed;
    watcher->Start();
    return watcher;
}

void DeviceInfoPrivate::OnDeviceAdded(DeviceWatcher^ watcher, DeviceInformation^ information)
{
    Logger::FrameworkDebug("[DeviceDetector] device added with name \"%s\", id = \"%s\", isEnabled = %d", String(WStringToString(information->Name->Data())).c_str(), String(WStringToString(information->Id->Data())).c_str(), static_cast<int32>(information->IsEnabled));
    auto iter = mapWatchers.find(watcher);
    if (iter != mapWatchers.end())
    {
        devices[iter->second][information->Id] = information->Name;
        NotifyAllClients(iter->second, true);
    }
}

void DeviceInfoPrivate::OnDeviceRemoved(DeviceWatcher^ watcher, DeviceInformationUpdate^ information)
{
    Logger::FrameworkDebug("[DeviceDetector] device removed with id = \"%s\".", String(WStringToString(information->Id->Data())).c_str());
    MapForWatchers::iterator iter = mapWatchers.find(watcher);
    if (iter != mapWatchers.end())
    {
        devices[iter->second].erase(information->Id);
        NotifyAllClients(iter->second, false);
    }
}

bool DeviceInfoPrivate::IsEnabled(AQSyntax usageId)
{
    return (devices[usageId].size() > 0);
}

}

#endif // defined(__DAVAENGINE_WIN_UAP__)

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
#include <collection.h>

#include "Debug/DVAssert.h"
#include "FileSystem/FileSystem.h"
#include "Utils/MD5.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"
#include "Base/GlobalEnum.h"

#include "Platform/TemplateWin32/DeviceInfoWinUAP.h"

__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
const wchar_t* KOSTIL_SURFACE_MOUSE = L"NTRG0001";
const wchar_t* KOSTIL_SURFACE_KEYBOARD = L"MSHW0029";

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
using namespace ::Windows::UI::ViewManagement;

namespace DAVA
{
// MSDN:: https://msdn.microsoft.com/en-us/library/windows/hardware/ff541364(v=vs.85).aspx
const wchar_t* GUID_DEVINTERFACE_MOUSE = L"System.Devices.InterfaceClassGuid:=\"{378DE44C-56EF-11D1-BC8C-00A0C91405DD}\"";
const wchar_t* GUID_DEVINTERFACE_KEYBOARD = L"System.Devices.InterfaceClassGuid:=\"{884b96c3-56ef-11d1-bc8c-00a0c91405dd}\"";
const wchar_t* GUID_DEVINTERFACE_TOUCH = L"System.Devices.InterfaceClassGuid:=\"{4D1E55B2-F16F-11CF-88CB-001111000030}\"";
const char* DEFAULT_TOUCH_ID = "touchId";

DeviceInfoPrivate::DeviceInfoPrivate()
{
    isMobileMode = Windows::Foundation::Metadata::ApiInformation::IsApiContractPresent("Windows.Phone.PhoneContract", 1);
    platform = isMobileMode ? DeviceInfo::PLATFORM_PHONE_WIN_UAP : DeviceInfo::PLATFORM_DESKTOP_WIN_UAP;
    TouchCapabilities touchCapabilities;
    isTouchPresent = (1 == touchCapabilities.TouchPresent); //  Touch is always present in MSVS simulator
    if (isTouchPresent)
    {
        auto hidsAccessor(hids.GetAccessor());
        Set<String>& setIdDevices = (*(hidsAccessor))[TOUCH];
        setIdDevices.emplace(DEFAULT_TOUCH_ID);
    }

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
    deviceName = WideString(deviceInfo.FriendlyName->Data());
    gpu = GPUFamily();

    try
    {
        uDID = RTStringToString(Windows::System::UserProfile::AdvertisingManager::AdvertisingId);
    }
    catch (Platform::Exception ^ e)
    {
        Logger::Error("[DeviceInfo] failed to get AdvertisingId: hresult=0x%08X, message=%s", e->HResult, WStringToString(e->Message->Data()).c_str());
        uDID = "";
    }
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
    ConnectionProfile ^ icp = NetworkInformation::GetInternetConnectionProfile();
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

        DisplayInformation ^ displayInfo = DisplayInformation::GetForCurrentView();
        DVASSERT(displayInfo != nullptr);
        screenInfo.scale = static_cast<float32>(displayInfo->RawPixelsPerViewPixel);
        DisplayOrientations curOrientation = DisplayInformation::GetForCurrentView()->CurrentOrientation;
        if (DisplayOrientations::Portrait == curOrientation || DisplayOrientations::PortraitFlipped == curOrientation)
        {
            std::swap(screenInfo.width, screenInfo.height);
        }
        //  in Continuum mode, we don't have touch
        if (isMobileMode)
        {
            bool last = isContinuumMode;
            if (UserInteractionMode::Mouse == UIViewSettings::GetForCurrentView()->UserInteractionMode)
            {
                isContinuumMode = true;
            }
            else
            {
                isContinuumMode = false;
            }
            if (last != isContinuumMode)
            {
                NotifyAllClients(TOUCH, !isContinuumMode);
            }
        }
    };
    core->RunOnUIThreadBlocked(func);
    // start device watchers, after creation main thread dispatcher
    if (!watchersCreated)
    {
        CreateAndStartHIDWatcher();
        watchersCreated = true;
    }
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
        Platform::String ^ path = removableStorages->GetAt(i)->Path;
        storage.path = FilePath::FromNativeString(path->Data());
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
    // continuum mode don't have touch
    if (DeviceInfo::HID_TOUCH_TYPE == type && isContinuumMode)
    {
        return false;
    }
    auto func = [type](HIDConvPair pair) -> bool {
        return pair.second == type;
    };
    auto it = std::find_if(HidConvSet.begin(), HidConvSet.end(), func);
    return IsEnabled(it->first);
}

bool DeviceInfoPrivate::IsTouchPresented()
{
    return isTouchPresent && !isContinuumMode; //  Touch is always present in MSVS simulator
}

void DeviceInfoPrivate::NotifyAllClients(NativeHIDType type, bool isConnected)
{
    auto func = [type](HIDConvPair pair) -> bool {
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

DeviceWatcher ^ DeviceInfoPrivate::CreateDeviceWatcher(NativeHIDType type)
{
    hids.GetAccessor()->emplace(type, Set<String>());
    DeviceWatcher ^ watcher = nullptr;
    Platform::Collections::Vector<Platform::String ^> ^ requestedProperties = ref new Platform::Collections::Vector<Platform::String ^>();
    requestedProperties->Append("System.Devices.InterfaceClassGuid");
    requestedProperties->Append("System.ItemNameDisplay");
    if (MOUSE == type)
    {
        watcher = DeviceInformation::CreateWatcher(ref new Platform::String(GUID_DEVINTERFACE_MOUSE), requestedProperties);
    }
    else if (KEYBOARD == type)
    {
        watcher = DeviceInformation::CreateWatcher(ref new Platform::String(GUID_DEVINTERFACE_KEYBOARD), requestedProperties);
    }
    else if (TOUCH == type)
    {
        watcher = DeviceInformation::CreateWatcher(ref new Platform::String(GUID_DEVINTERFACE_TOUCH), requestedProperties);
    }
    else
    {
        watcher = DeviceInformation::CreateWatcher(HidDevice::GetDeviceSelector(USAGE_PAGE, type));
    }
    auto added = ref new TypedEventHandler<DeviceWatcher ^, DeviceInformation ^>([this, type](DeviceWatcher ^ watcher, DeviceInformation ^ information) {
        OnDeviceAdded(type, information);
    });
    auto removed = ref new TypedEventHandler<DeviceWatcher ^, DeviceInformationUpdate ^>([this, type](DeviceWatcher ^ watcher, DeviceInformationUpdate ^ information) {
        OnDeviceRemoved(type, information);
    });
    auto updated = ref new TypedEventHandler<DeviceWatcher ^, DeviceInformationUpdate ^>([this, type](DeviceWatcher ^ watcher, DeviceInformationUpdate ^ information) {
        OnDeviceUpdated(type, information);
    });
    if (TOUCH != type)
    {
        watcher->Added += added;
        watcher->Removed += removed;
    }
    watcher->Updated += updated;
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
    watchers.emplace_back(CreateDeviceWatcher(TOUCH));
}

void DeviceInfoPrivate::OnDeviceAdded(NativeHIDType type, DeviceInformation ^ information)
{
    if (!information->IsEnabled)
    {
        return;
    }
    //TODO: delete it, kostil for surface mouse
    if (MOUSE == type)
    {
        if (wcsstr(information->Id->Data(), KOSTIL_SURFACE_MOUSE) != nullptr)
        {
            return;
        }
    }
    //TODO: delete it, kostil for surface keyboard
    if (KEYBOARD == type)
    {
        if (wcsstr(information->Id->Data(), KOSTIL_SURFACE_KEYBOARD) != nullptr)
        {
            return;
        }
    }
    auto hidsAccessor(hids.GetAccessor());
    String id = RTStringToString(information->Id);
    Set<String>& setIdDevices = (*(hidsAccessor))[type];
    auto idIter = setIdDevices.find(id);
    if (idIter == setIdDevices.end())
    {
        setIdDevices.emplace(std::move(id));
        NotifyAllClients(type, true);
    }
}

void DeviceInfoPrivate::OnDeviceRemoved(NativeHIDType type, DeviceInformationUpdate ^ information)
{
    String id = RTStringToString(information->Id);
    auto hidsAccessor(hids.GetAccessor());
    Set<String>& setIdDevices = (*(hidsAccessor))[type];
    auto idIter = setIdDevices.find(id);
    if (idIter != setIdDevices.end())
    {
        setIdDevices.erase(idIter);
        NotifyAllClients(type, false);
    }
}

void DeviceInfoPrivate::OnDeviceUpdated(NativeHIDType type, DeviceInformationUpdate ^ information)
{
    auto hidsAccessor(hids.GetAccessor());
    Set<String>& setIdDevices = (*(hidsAccessor))[type];
    if (TOUCH == type)
    {
        TouchCapabilities touchCapabilities;
        bool newState = (1 == touchCapabilities.TouchPresent);
        if (isTouchPresent != newState)
        {
            isTouchPresent = newState;
            if (isTouchPresent)
            {
                setIdDevices.emplace(DEFAULT_TOUCH_ID);
            }
            else
            {
                setIdDevices.erase(DEFAULT_TOUCH_ID);
            }
            NotifyAllClients(type, isTouchPresent);
        }
    }
    else
    {
        bool isEnabled = false;
        Windows::Foundation::Collections::IMapView<Platform::String ^, Platform::Object ^> ^ properties = information->Properties;
        if (properties->HasKey(L"System.Devices.InterfaceEnabled"))
        {
            try
            {
                isEnabled = safe_cast<bool>(properties->Lookup(L"System.Devices.InterfaceEnabled"));
            }
            catch (Platform::InvalidCastException ^ e)
            {
                Logger::FrameworkDebug("DeviceInfoPrivate::OnDeviceUpdated. Can't cast System.Devices.InterfaceEnabled.");
            }
            catch (Platform::OutOfBoundsException ^ e)
            {
                Logger::FrameworkDebug("DeviceInfoPrivate::OnDeviceUpdated. OutOfBoundsException.");
            }
        }
        String id = RTStringToString(information->Id);
        auto iterId = setIdDevices.find(id);
        if (isEnabled)
        {
            if (iterId == setIdDevices.end())
            {
                setIdDevices.emplace(std::move(id));
                NotifyAllClients(type, isEnabled);
            }
        }
        else
        {
            if (iterId != setIdDevices.end())
            {
                setIdDevices.erase(id);
                NotifyAllClients(type, isEnabled);
            }
        }
    }
}

bool DeviceInfoPrivate::IsEnabled(NativeHIDType type)
{
    auto hidsAccessor(hids.GetAccessor());
    Set<String>& setIdDevices = (*(hidsAccessor))[type];
    return (setIdDevices.size() > 0);
}
}

#endif // defined(__DAVAENGINE_WIN_UAP__)

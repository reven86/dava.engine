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

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Utils/Utils.h"
#include "DeviceDetectorWinUAP.h"

namespace DAVA
{
using namespace ::Windows::Devices::Input;
using namespace ::Windows::UI::ViewManagement;
using namespace ::Windows::Foundation;
using namespace ::Windows::Devices;
using namespace ::Windows::Devices::Enumeration;
using namespace ::Windows::Devices::HumanInterfaceDevice;

DeviceDetector::AQSyntax DeviceDetector::ConvertHIDToAQS(DeviceInfo::eHIDType hid)
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

DeviceInfo::eHIDType DeviceDetector::ConvertAQSToHID(AQSyntax aqs)
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

void DeviceDetector::NotifyAllClients(AQSyntax usageId, bool connectState)
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

void DeviceDetector::AddCallBack(DeviceInfo::eHIDType hid, DeviceInfo::HIDCallBackFunc&& func)
{
    connections[ConvertHIDToAQS(hid)].emplace_back(std::forward<DeviceInfo::HIDCallBackFunc>(func));
}

bool DeviceDetector::IsHIDConnected(DeviceInfo::eHIDType hid)
{
    return IsEnabled(ConvertHIDToAQS(hid));
}

DeviceDetector::DeviceDetector()
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

DeviceWatcher^ DeviceDetector::WatcherForDeviceEvents(uint16 usagePage, uint16 usageId)
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

void DeviceDetector::OnDeviceAdded(DeviceWatcher^ watcher, DeviceInformation^ information)
{
    Logger::FrameworkDebug("[DeviceDetector] device added with name \"%s\", id = \"%s\", isEnabled = %d", String(WStringToString(information->Name->Data())).c_str(), String(WStringToString(information->Id->Data())).c_str(), static_cast<int32>(information->IsEnabled));
    auto iter = mapWatchers.find(watcher);
    if (iter != mapWatchers.end())
    {
        devices[iter->second][information->Id] = information->Name;
        NotifyAllClients(iter->second, true);
    }
}

void DeviceDetector::OnDeviceRemoved(DeviceWatcher^ watcher, DeviceInformationUpdate^ information)
{
    Logger::FrameworkDebug("[DeviceDetector] device removed with id = \"%s\".", String(WStringToString(information->Id->Data())).c_str());
    MapForWatchers::iterator iter = mapWatchers.find(watcher);
    if (iter != mapWatchers.end())
    {
        devices[iter->second].erase(information->Id);
        NotifyAllClients(iter->second, false);
    }
}

bool DeviceDetector::IsEnabled(AQSyntax usageId)
{
    return (devices[usageId].size() > 0);
}

DeviceDetector* DeviceDetector::GetDeviceDetector()
{
    static DeviceDetector instance;
    return &instance;
}

} //  namespace DAVA

#endif //  (__DAVAENGINE_WIN_UAP__)
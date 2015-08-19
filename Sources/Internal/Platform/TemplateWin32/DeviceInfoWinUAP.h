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


#ifndef __FRAMEWORK__DEVICEINFO_WINUAP__
#define __FRAMEWORK__DEVICEINFO_WINUAP__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Platform/DeviceInfo.h"

namespace DAVA
{
    class DeviceInfoPrivate
    {
    public:
        DeviceInfoPrivate();
        DeviceInfo::ePlatform GetPlatform();
        String GetPlatformString();
        String GetVersion();
        String GetManufacturer();
        String GetModel();
        String GetLocale();
        String GetRegion();
        String GetTimeZone();
        String GetUDID();
        WideString GetName();
        String GetHTTPProxyHost();
        String GetHTTPNonProxyHosts();
        int GetHTTPProxyPort();
        DeviceInfo::ScreenInfo& GetScreenInfo();
        int GetZBufferSize();
        eGPUFamily GetGPUFamily();
        DeviceInfo::NetworkInfo GetNetworkInfo();
        List<DeviceInfo::StorageInfo> GetStoragesList();
        int32 GetCpuCount();
        void InitializeScreenInfo();
        bool IsHIDConnected(DeviceInfo::eHIDType hid);
        void SubscribeHID(DeviceInfo::eHIDType hid, DeviceInfo::HIDCallBackFunc&& func);

    private:
        enum AQSyntax
        {
            AQS_UNKNOWN = 0x00,
            AQS_POINTER = 0x01,
            AQS_MOUSE = 0x02,
            AQS_JOYSTICK = 0x04,
            AQS_GAMEPAD = 0x05,
            AQS_KEYBOARD = 0x06,
            AQS_KEYPAD = 0x07,
            AQS_SYSTEM_CONTROL = 0x80
        };

        UnorderedMap<AQSyntax, DeviceInfo::eHIDType> convAqsToHid =
        { { AQS_UNKNOWN , DeviceInfo::HID_UNKNOWN_TYPE }, { AQS_POINTER, DeviceInfo::HID_POINTER_TYPE }, { AQS_MOUSE, DeviceInfo::HID_MOUSE_TYPE },
          { AQS_JOYSTICK, DeviceInfo::HID_JOYSTICK_TYPE }, { AQS_GAMEPAD, DeviceInfo::HID_GAMEPAD_TYPE }, { AQS_KEYBOARD, DeviceInfo::HID_KEYBOARD_TYPE },
          { AQS_KEYPAD, DeviceInfo::HID_KEYPAD_TYPE }, { AQS_SYSTEM_CONTROL, DeviceInfo::HID_SYSTEM_CONTROL_TYPE } };

        UnorderedMap<DeviceInfo::eHIDType, AQSyntax> convHidToAqs =
        { { DeviceInfo::HID_UNKNOWN_TYPE, AQS_UNKNOWN }, { DeviceInfo::HID_POINTER_TYPE, AQS_POINTER }, { DeviceInfo::HID_MOUSE_TYPE, AQS_MOUSE },
          { DeviceInfo::HID_JOYSTICK_TYPE, AQS_JOYSTICK }, { DeviceInfo::HID_GAMEPAD_TYPE, AQS_GAMEPAD }, { DeviceInfo::HID_KEYBOARD_TYPE, AQS_KEYBOARD },
          { DeviceInfo::HID_KEYPAD_TYPE, AQS_KEYPAD }, { DeviceInfo::HID_SYSTEM_CONTROL_TYPE, AQS_SYSTEM_CONTROL } };

        const uint16 AQS_USAGE_PAGE = 0x01;
        struct cmpDeviceWatcher {
            bool operator()(Windows::Devices::Enumeration::DeviceWatcher^ a, Windows::Devices::Enumeration::DeviceWatcher^ b) const {
                return a->GetHashCode() < b->GetHashCode();
            }
        };
        using PairForWatchersAndType = std::pair<Windows::Devices::Enumeration::DeviceWatcher^, AQSyntax>;
        using MapForWatchers = Map<PairForWatchersAndType::first_type, PairForWatchersAndType::second_type, cmpDeviceWatcher>;
        using MapForIdAndName = Map<Platform::String^, Platform::String^>;
        using MapForTypeAndDeviceInfo = Map<AQSyntax, MapForIdAndName>;
        using MapForTypeAndConnections = Map<AQSyntax, Vector<DeviceInfo::HIDCallBackFunc > >;

        Windows::Devices::Enumeration::DeviceWatcher^ WatcherForDeviceEvents(uint16 usagePage, uint16 usageId);
        void OnDeviceAdded(Windows::Devices::Enumeration::DeviceWatcher^, Windows::Devices::Enumeration::DeviceInformation^);
        void OnDeviceRemoved(Windows::Devices::Enumeration::DeviceWatcher^, Windows::Devices::Enumeration::DeviceInformationUpdate^);
        bool IsEnabled(AQSyntax usageId);
        void NotifyAllClients(AQSyntax usageId, bool connectState);
        eGPUFamily GPUFamily();
        bool IsMobileMode();

        bool isTouchPresent = false;
        MapForWatchers mapWatchers;
        MapForTypeAndDeviceInfo devices;
        MapForTypeAndConnections connections;

        DeviceInfo::ePlatform platform = DeviceInfo::PLATFORM_UNKNOWN;
        DeviceInfo::ScreenInfo screenInfo;
        eGPUFamily gpu = GPU_INVALID;
        String platformString;
        String version;
        String manufacturer;
        String uDID;
        WideString productName;
        int32 zBufferSize = 24;
        int32 cpuCount = 1;
    };

};

#endif //  (__DAVAENGINE_WIN_UAP__)

#endif /* defined(__FRAMEWORK__DEVICEINFO_WINUAP__) */

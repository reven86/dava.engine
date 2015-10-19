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

#include "Platform/DeviceInfoPrivateBase.h"

namespace DAVA
{
class DeviceInfoPrivate : public DeviceInfoPrivateBase
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
    void InitializeScreenInfo();
    bool IsHIDConnected(DeviceInfo::eHIDType type);

private:
    enum NativeHIDType
    {
        UNKNOWN = 0x00,
        POINTER = 0x01,
        MOUSE = 0x02,
        JOYSTICK = 0x04,
        GAMEPAD = 0x05,
        KEYBOARD = 0x06,
        KEYPAD = 0x07,
        SYSTEM_CONTROL = 0x80
    };
    const uint16 USAGE_PAGE = 0x01;
    using HIDConvPair = std::pair<NativeHIDType, DeviceInfo::eHIDType>;
    Vector<HIDConvPair> HidConvSet =
    {
    {UNKNOWN, DeviceInfo::HID_UNKNOWN_TYPE},
    {POINTER, DeviceInfo::HID_POINTER_TYPE},
    {MOUSE, DeviceInfo::HID_MOUSE_TYPE},
    {JOYSTICK, DeviceInfo::HID_JOYSTICK_TYPE},
    {GAMEPAD, DeviceInfo::HID_GAMEPAD_TYPE},
    {KEYBOARD, DeviceInfo::HID_KEYBOARD_TYPE},
    {KEYPAD, DeviceInfo::HID_KEYPAD_TYPE},
    {SYSTEM_CONTROL, DeviceInfo::HID_SYSTEM_CONTROL_TYPE}};

    Windows::Devices::Enumeration::DeviceWatcher ^ CreateDeviceWatcher(NativeHIDType type);
    void OnDeviceAdded(NativeHIDType type, Windows::Devices::Enumeration::DeviceInformation ^ information);
    void OnDeviceRemoved(NativeHIDType type, Windows::Devices::Enumeration::DeviceInformationUpdate ^ information);
    bool IsEnabled(NativeHIDType type);
    void NotifyAllClients(NativeHIDType type, bool isConnected);
    eGPUFamily GPUFamily();

    bool isTouchPresent = false;
    bool isMobileMode = false;
    Map<NativeHIDType, uint16> hids;
    Vector<Windows::Devices::Enumeration::DeviceWatcher ^> watchers;

    DeviceInfo::ePlatform platform = DeviceInfo::PLATFORM_UNKNOWN;
    DeviceInfo::ScreenInfo screenInfo;
    eGPUFamily gpu = GPU_INVALID;
    String platformString;
    String version;
    String manufacturer;
    String modelName;
    String uDID;
    WideString productName;
    int32 zBufferSize = 24;
};
};

#endif //  (__DAVAENGINE_WIN_UAP__)

#endif /* defined(__FRAMEWORK__DEVICEINFO_WINUAP__) */

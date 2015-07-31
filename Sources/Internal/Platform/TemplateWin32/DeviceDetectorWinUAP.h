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
#ifndef __FRAMEWORK__DEVICEDETECTOR__
#define __FRAMEWORK__DEVICEDETECTOR__

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Platform/DeviceInfo.h"

namespace DAVA
{

class DeviceDetector : public Singleton<DeviceDetector>
{
public:
    DeviceDetector();

    void AddCallBack(int32 hid, DeviceInfo::HIDCallBackFunc&& func);
    bool IsHIDConnect(int32 hid);

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
    const uint16 AQS_USAGE_PAGE = 0x01;
    struct cmpDeviceWatcher {
        bool operator()(Windows::Devices::Enumeration::DeviceWatcher^ a, Windows::Devices::Enumeration::DeviceWatcher^ b) const {
            return a->GetHashCode() < b->GetHashCode();
        }
    };
    typedef std::pair<Windows::Devices::Enumeration::DeviceWatcher^, AQSyntax> MapDeviceWatchersPair;
    typedef Map<MapDeviceWatchersPair::first_type, MapDeviceWatchersPair::second_type, cmpDeviceWatcher> MapDeviceWatchers;
    typedef Map<Platform::String^, Platform::String^> MapIdName;
    typedef Map<AQSyntax, MapIdName> MapTypesDevice;
    typedef Map<AQSyntax, Vector<DeviceInfo::HIDCallBackFunc > > MapTypesConnections;

    Windows::Devices::Enumeration::DeviceWatcher^ WatcherForDeviceEvents(uint16 usagePage, uint16 usageId);
    void OnDeviceAdded(Windows::Devices::Enumeration::DeviceWatcher^, Windows::Devices::Enumeration::DeviceInformation^);
    void OnDeviceRemoved(Windows::Devices::Enumeration::DeviceWatcher^, Windows::Devices::Enumeration::DeviceInformationUpdate^);
    bool IsEnabled(AQSyntax usageId);
    AQSyntax GetAQS(int32 hid);
    int ConvertAQSToInt(DeviceDetector::AQSyntax aqsType);
    void NotifyAllClients(AQSyntax usageId, bool connectState);

    bool isTouchPresent = false;
    MapDeviceWatchers mapWatchers;
    MapTypesDevice devices;
    MapTypesConnections connections;
};

DeviceDetector* GetDeviceDetector();

} //  namespace DAVA

#endif //  defined(__DAVAENGINE_WIN_UAP__)

#endif //  __FRAMEWORK__DEVICEDETECTOR__
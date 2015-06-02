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


#ifndef __DAVAENGINE_PEERDESRIPTION_H__
#define __DAVAENGINE_PEERDESRIPTION_H__

#include <Base/BaseTypes.h>
#include <Platform/DeviceInfo.h>

#include <Network/Base/IfAddress.h>
#include <Network/NetConfig.h>

namespace DAVA
{
namespace Net
{

class PeerDescription
{
public:
    PeerDescription();
    PeerDescription(const NetConfig& config);

    DeviceInfo::ePlatform GetPlatform() const;
    const String& GetPlatformString() const;
    const String& GetVersion() const;
    const String& GetManufacturer() const;
    const String& GetModel() const;
    const String& GetUDID() const;
    const String& GetName() const;
    const DeviceInfo::ScreenInfo& GetScreenInfo() const;
    eGPUFamily GetGPUFamily() const;

    const NetConfig& NetworkConfig() const;
    const Vector<IfAddress>& NetworkInterfaces() const;
    void SetNetworkInterfaces(const Vector<IfAddress>& availIfAddr);

    size_t SerializedSize() const;
    size_t Serialize(void* dstBuffer, size_t buflen) const;
    size_t Deserialize(const void* srcBuffer, size_t buflen);

#ifdef __DAVAENGINE_DEBUG__
    void DumpToStdout() const;
#endif

private:
    DeviceInfo::ePlatform platformType;
    String platform;
    String version;
    String manufacturer;
    String model;
    String udid;
    String name;
    DeviceInfo::ScreenInfo screenInfo;
    eGPUFamily gpuFamily;

    NetConfig netConfig;
    Vector<IfAddress> ifaddr;
};

//////////////////////////////////////////////////////////////////////////
inline DeviceInfo::ePlatform PeerDescription::GetPlatform() const
{
    return platformType;
}

inline const String& PeerDescription::GetPlatformString() const
{
    return platform;
}

inline const String& PeerDescription::GetVersion() const
{
    return version;
}

inline const String& PeerDescription::GetManufacturer() const
{
    return manufacturer;
}

inline const String& PeerDescription::GetModel() const
{
    return model;
}

inline const String& PeerDescription::GetUDID() const
{
    return udid;
}

inline const String& PeerDescription::GetName() const
{
    return name;
}

inline const DeviceInfo::ScreenInfo& PeerDescription::GetScreenInfo() const
{
    return screenInfo;
}

inline eGPUFamily PeerDescription::GetGPUFamily() const
{
    return gpuFamily;
}

inline const NetConfig& PeerDescription::NetworkConfig() const
{
    return netConfig;
}

inline const Vector<IfAddress>& PeerDescription::NetworkInterfaces() const
{
    return ifaddr;
}

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_PEERDESRIPTION_H__

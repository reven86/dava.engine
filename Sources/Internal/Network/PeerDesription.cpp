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

#include <Debug/DVAssert.h>
#include <Utils/UTF8Utils.h>

#include <Network/PeerDesription.h>

namespace DAVA
{
namespace Net
{

PeerDescription::PeerDescription()
    : platformType()
    , gpuFamily()
{

}

PeerDescription::PeerDescription(const NetConfig& config)
    : platformType(DeviceInfo::GetPlatform())
    , platform(DeviceInfo::GetPlatformString())
    , version(DeviceInfo::GetVersion())
    , manufacturer(DeviceInfo::GetManufacturer())
    , model(DeviceInfo::GetModel())
    , udid(DeviceInfo::GetUDID())
    , name(UTF8Utils::EncodeToUTF8(DeviceInfo::GetName()))
    , screenInfo(DeviceInfo::GetScreenInfo())
    , gpuFamily(DeviceInfo::GetGPUFamily())
    , netConfig(config)
{
    DVASSERT(true == netConfig.Validate());
}

void PeerDescription::SetNetworkInterfaces(const Vector<IfAddress>& availIfAddr)
{
    ifaddr = availIfAddr;
}

#ifdef __DAVAENGINE_DEBUG__
void PeerDescription::DumpToStdout() const
{
    printf("PeerDescription: %s, screen %dx%d, UDID=%s\n", name.c_str(), screenInfo.width, screenInfo.height, udid.c_str());
    printf("  %s %s %s %s\n", manufacturer.c_str(), model.c_str(), platform.c_str(), version.c_str());
    printf("  Network interfaces:\n");
    for (size_t i = 0, n = ifaddr.size();i < n;++i)
    {
        printf("    %s\n", ifaddr[i].Address().ToString().c_str());
    }
    printf("  Network configuration:\n");
    for (size_t i = 0, n = netConfig.Transports().size();i < n;++i)
    {
        const char* s = "unknown";
        switch (netConfig.Transports()[i].type)
        {
        case TRANSPORT_TCP:
            s = "TCP";
            break;
        }
        printf("    %s: %hu\n", s, netConfig.Transports()[i].endpoint.Port());
    }
    printf("    services: ");
    for (size_t i = 0, n = netConfig.Services().size();i < n;++i)
    {
        printf("%u; ", netConfig.Services()[i]);
    }
    printf("\n");
}
#endif

}   // namespace Net
}   // namespace DAVA

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


#include "Platform/TemplateWin32/UAPNetworkHelper.h"
#include "Platform/DeviceInfo.h"

namespace DAVA
{

const char* UAPNetworkHelper::UAP_IP_ADDRESS = "127.0.0.1";

DeviceInfo::ePlatform GetPlatformChecked()
{
    DeviceInfo::ePlatform platform = DeviceInfo::GetPlatform();
    bool uapPlatform = platform == DeviceInfo::PLATFORM_DESKTOP_WIN_UAP ||
                       platform == DeviceInfo::PLATFORM_PHONE_WIN_UAP;

    DVASSERT_MSG(uapPlatform, "Not UAP platform");
    return platform;
}

Net::eNetworkRole UAPNetworkHelper::GetCurrentNetworkRole()
{
    DeviceInfo::ePlatform platform = GetPlatformChecked();
    return platform == DeviceInfo::PLATFORM_DESKTOP_WIN_UAP ? Net::CLIENT_ROLE : Net::SERVER_ROLE;
}

Net::Endpoint UAPNetworkHelper::GetCurrentEndPoint()
{
    return GetEndPoint(GetCurrentNetworkRole());
}

Net::Endpoint UAPNetworkHelper::GetEndPoint(Net::eNetworkRole role)
{
    DeviceInfo::ePlatform platform = GetPlatformChecked();
    uint16 port;
    if (platform == DeviceInfo::PLATFORM_DESKTOP_WIN_UAP)
    {
        port = UAP_DESKTOP_TCP_PORT;
    }
    else
    {
        port = UAP_MOBILE_TCP_PORT;
    }

    switch (role)
    {
    case Net::SERVER_ROLE:
        return Net::Endpoint(port);
    case Net::CLIENT_ROLE:
        return Net::Endpoint(UAP_IP_ADDRESS, port);
    default:
        DVASSERT_MSG(false, "Something wrong");
        return Net::Endpoint();
    }
}

}  // namespace DAVA

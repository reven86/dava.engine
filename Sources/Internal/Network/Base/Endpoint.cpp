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


#include <cstdio>

#include "Base/BaseTypes.h"
#include <libuv/uv.h>

#include "Debug/DVAssert.h"
#include "Network/Base/Endpoint.h"

namespace DAVA
{
namespace Net
{

Endpoint::Endpoint(const char8* address, uint16 port) : data()
{
    InitSockaddrIn(IPAddress::FromString(address).ToUInt(), port);
}

Endpoint::Endpoint(const sockaddr* sa)
{
    DVASSERT(sa);
    Memcpy(&data, sa, sizeof(data));
}

Endpoint::Endpoint(const sockaddr_in* sin)
{
    DVASSERT(sin);
    Memcpy(&data, sin, sizeof(data));
}

bool Endpoint::ToString(char8* buffer, size_t size) const
{
    DVASSERT(buffer != NULL && size > 0);
    Array<char8, 20> addr;
    if(Address().ToString(addr.data(), addr.size()))
    {
        // TODO: Snprintf on Win32 do not conform standard
        Snprintf(buffer, size, "%s:%hu", addr.data(), Port());
        return true;
    }
    return false;
}

String Endpoint::ToString() const
{
    Array<char8, 50> buf;
    if(ToString(buf.data(), buf.size()))
        return String(buf.data());
    return String();
}

void Endpoint::InitSockaddrIn(uint32 addr, uint16 port)
{
    data.sin_family = AF_INET;
    data.sin_port   = htons(port);
#ifdef __DAVAENGINE_WINDOWS__
    data.sin_addr.S_un.S_addr = htonl(addr);
#else   // __DAVAENGINE_WINDOWS__
    data.sin_addr.s_addr      = htonl(addr);
#endif  // __DAVAENGINE_WINDOWS__
}

uint32 Endpoint::GetSockaddrAddr() const
{
#ifdef __DAVAENGINE_WINDOWS__
    return ntohl(data.sin_addr.S_un.S_addr);
#else   // __DAVAENGINE_WINDOWS__
    return ntohl(data.sin_addr.s_addr);
#endif  // __DAVAENGINE_WINDOWS__
}

}   // namespace Net
}   // namespace DAVA

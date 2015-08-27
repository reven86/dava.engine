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

#include "Debug/DVAssert.h"

#include "Network/Base/Endpoint.h"
#include "Network/Base/IPAddress.h"

namespace DAVA
{
namespace Net
{

IPAddress::IPAddress(const char8* address) : addr(0)
{
    DVASSERT(address != NULL);
    *this = FromString(address);
}

bool IPAddress::ToString(char8* buffer, size_t size) const
{
#ifdef __DAVAENGINE_WIN_UAP__
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return false;
#else
    DVASSERT(buffer != NULL && size > 0);
    return 0 == uv_ip4_name(Endpoint(*this, 0).CastToSockaddrIn(), buffer, size);
#endif
}

String IPAddress::ToString() const
{
    char8 buf[20];  // This should be enough for IPv4 address
    bool ret = ToString(buf, 20);
    DVASSERT(ret);
    return String(ret ? buf : "");
}

IPAddress IPAddress::FromString(const char8* addr)
{
#ifdef __DAVAENGINE_WIN_UAP__
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return IPAddress();
#else
    DVASSERT(addr != NULL);

    Endpoint endp;
    if(0 == uv_ip4_addr(addr, 0, endp.CastToSockaddrIn()))
        return endp.Address();
    return IPAddress();
#endif
}

}   // namespace Net
}   // namespace DAVA

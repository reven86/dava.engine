/*==================================================================================
    Copyright(c) 2008, binaryzebra
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

#include "Endpoint.h"
#include "IPAddress.h"

namespace DAVA
{

bool IPAddress::ToString(char8* buffer, std::size_t size) const
{
    DVASSERT(buffer != NULL && size > 0);
    return 0 == uv_ip4_name(Endpoint(*this, 0).CastToSockaddrIn(), buffer, size);
}

String IPAddress::ToString() const
{
    char8 buf[50];
    if(ToString(buf, COUNT_OF(buf)))
        return String(buf);
    return String();
}

IPAddress IPAddress::FromString(const char8* addr)
{
    DVASSERT(addr != NULL);

    Endpoint endp;
    if(0 == uv_ip4_addr(addr, 0, endp.CastToSockaddrIn()))
        return endp.Address();
    return IPAddress();
}

}   // namespace DAVA

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

#include "Network/Base/AddressResolver.h"
#include "Network/Base/Endpoint.h"
#include "Network/Base/IOLoop.h"
#include "Network/Base/NetworkUtils.h"
#include "FileSystem/Logger.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
namespace Net
{
AddressResolver::AddressResolver(IOLoop* _loop)
    : loop(_loop)
{
    DVASSERT(nullptr != loop);
}

AddressResolver::~AddressResolver()
{
    Cancel();
}

bool AddressResolver::AsyncResolve(const char8* address, uint16 port, ResolverCallbackFn cbk)
{
    DVASSERT(loop != nullptr);
    DVASSERT(handle == nullptr);

    struct addrinfo hints;
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_TCP;

    handle = new uv_getaddrinfo_t;
    handle->data = this;

    Array<char, 6> portstring;
    Snprintf(portstring.data(), portstring.size(), "%hu", port);

    int32 res = uv_getaddrinfo(loop->Handle(), handle, &AddressResolver::GetAddrInfoCallback, address, portstring.data(), &hints);
    if (0 == res)
    {
        resolverCallbackFn = cbk;
        return true;
    }
    else
    {
        SafeDelete(handle);

        Logger::Error("[AddressResolver::StartResolving] Can't get addr info: %s", Net::ErrorToString(res));
        return false;
    }
}

void AddressResolver::Cancel()
{
    if (nullptr != handle)
    {
        uv_cancel(reinterpret_cast<uv_req_t*>(handle));
        handle = nullptr;
    }
}

void AddressResolver::GetAddrInfoCallback(uv_getaddrinfo_t* handle, int status, struct addrinfo* response)
{
    AddressResolver* resolver = static_cast<AddressResolver*>(handle->data);
    if (nullptr != resolver)
    {
        resolver->GotAddrInfo(status, response);
    }

    SafeDelete(handle);

    uv_freeaddrinfo(response);
}

void AddressResolver::GotAddrInfo(int status, struct addrinfo* response)
{
    if (handle)
    {
        Endpoint endpoint;
        if (0 == status)
        {
            endpoint = Endpoint(response->ai_addr);
        }

        resolverCallbackFn(endpoint, status);
    }
}

} // end of namespace Net
} // end of namespace DAVA

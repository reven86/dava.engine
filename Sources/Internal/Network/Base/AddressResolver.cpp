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
#include "FileSystem/Logger.h"
#include "Debug/DVAssert.h"


namespace DAVA {
namespace Net {

AddressResolver::AddressResolver(IOLoop* loop_) 
    : loop(loop_)
    , handle(nullptr)
{
}

AddressResolver::~AddressResolver()
{
    Stop();
}

bool AddressResolver::StartResolving(const char8* address, uint16 port, ResolverCallbackFn cbk)
{
    DVASSERT(loop != nullptr);
    DVASSERT(handle == nullptr);

    struct addrinfo hints;
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_TCP;

    uv_getaddrinfo_t* handle = new uv_getaddrinfo_t;
    handle->data = this;

    Array<char, 6> portstring;
    Snprintf(portstring.data(), portstring.size(), "%u", port);

    int res = uv_getaddrinfo(loop->Handle(), handle, AddressResolver::GetAddrInfoCallback, address, portstring.data(), &hints);

    if (!res)
    {
        resolverCallbackFn = cbk;
        return true;
    }
    else
    {
        delete handle;
        const char* err = uv_err_name(res);
        Logger::Error("[AddressResolver::StartResolving] Can't get addr info: %s", err);
        return false;
    }
}

void AddressResolver::Stop()
{
    if (handle)
    {
        handle->data = nullptr;
        handle = nullptr;
    }
}

void AddressResolver::GetAddrInfoCallback(uv_getaddrinfo_t* handle, int status, struct addrinfo* response)
{
    AddressResolver* resolver = static_cast<AddressResolver*>(handle->data);

    if (resolver != nullptr)
    {
        resolver->GotAddrInfo(status, response);
        resolver->Stop();
    }
    else
    {
        delete handle;
    }

    uv_freeaddrinfo(response);
}

void AddressResolver::GotAddrInfo(int status, struct addrinfo* response)
{
    EndpointPtr endpoint;

    if (status == 0)
    {
         endpoint.reset(new Endpoint(response->ai_addr));
    }
    else
    {
        const char* err = uv_err_name(status);
        Logger::Error("[AddressResolver::GotAddrInfo] Can't get addr info: %s", err);
        endpoint.reset();
    }

    resolverCallbackFn(endpoint);
}

}};
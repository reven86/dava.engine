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

#include <string>

#include "Network/Base/AddressResolver.h"
#include "Concurrency/LockGuard.h"
#include "FileSystem/Logger.h"
#include "Debug/DVAssert.h"


namespace DAVA {
namespace Net {

List<AddressResolver::HandlePtr> AddressResolver::handlesHolder;
Map<uv_getaddrinfo_t*, AddressResolver::HandleContext> AddressResolver::handles;
Mutex AddressResolver::handlesMutex;

AddressResolver::AddressResolver(AddressRequester& rq)
    : requester(rq)
    , state(State::NOT_REQUESTED)
    , handle(nullptr)
{
}

AddressResolver::~AddressResolver()
{
    Stop();
}

uv_getaddrinfo_t* AddressResolver::OccupyHandle(AddressResolver* resolver)
{
    DVASSERT(resolver);

    LockGuard<Mutex> lock(handlesMutex);

    for (auto& handle : handles)
    {
        if (handle.second.isFree)
        {
            handle.second.resolver = resolver;
            handle.second.isFree = false;
            return handle.first;
        }
    }

    uv_getaddrinfo_t* handle = new uv_getaddrinfo_t;
    handlesHolder.emplace_back(handle);
    handles[handle] = { resolver, false };
    return handle;
}

void AddressResolver::UnbindResolver(uv_getaddrinfo_t* handle, const AddressResolver* resolver)
{
    LockGuard<Mutex> lock(handlesMutex);

    const auto& handleEntry = handles.find(handle);
    if (handleEntry != handles.end() && handleEntry->second.resolver == resolver)
    {
        handleEntry->second.resolver = nullptr;
    }
}

bool AddressResolver::StartResolving(const char8* address, uint16 port)
{
    uv_loop_t* loop = uv_default_loop();

    struct addrinfo hints;
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_TCP;

    uv_getaddrinfo_t* handle = OccupyHandle(this);

    int res = uv_getaddrinfo(loop, handle, AddressResolver::GetAddrInfoCallback, address, std::to_string(port).c_str(), &hints);

    if (!res)
    {
        state = State::RESOLVING;
        return true;
    }
    else
    {
        state = State::RESOLVE_ERROR;
        const char* err = uv_err_name(res);
        Logger::FrameworkDebug("Can't get addr info: %s", err);
        return false;
    }
}

void AddressResolver::Stop()
{
    if (handle)
    {
        UnbindResolver(handle, this);
        handle = nullptr;
        state = State::NOT_REQUESTED;
    }
}

void AddressResolver::GetAddrInfoCallback(uv_getaddrinfo_t* handle, int status, struct addrinfo* response)
{
    LockGuard<Mutex> lock(handlesMutex);

    AddressResolver* resolver = nullptr;
    auto handleAndContext = handles.find(handle);
    DVASSERT(handleAndContext != handles.end());

    resolver = handleAndContext->second.resolver;
    handleAndContext->second.isFree = true;

    if (resolver)
    {
        resolver->GotAddrInfo(status, response);
    }
    
    uv_freeaddrinfo(response);
}

void AddressResolver::GotAddrInfo(int status, struct addrinfo* response)
{
    if (status == 0)
    {
        state = State::RESOLVED;
        result = *response;
    }
    else
    {
        state = State::RESOLVE_ERROR;
        const char* err = uv_err_name(status);
        Logger::FrameworkDebug("Can't get addr info: %s", err);
    }

    requester.OnAddressResolved();
}

}};
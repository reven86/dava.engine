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


#ifndef __DAVAENGINE_ADDRESS_RESOLVER_H__
#define __DAVAENGINE_ADDRESS_RESOLVER_H__

#include "Base/BaseTypes.h"
#include "Concurrency/Mutex.h"
#include <libuv/uv.h>

namespace DAVA {
namespace Net {

struct AddressRequester
{
    virtual void OnAddressResolved() = 0;
};

class AddressResolver
{
public:
    AddressResolver(AddressRequester& requester);
    ~AddressResolver();

    enum class State { NOT_REQUESTED, RESOLVING, RESOLVED, RESOLVE_ERROR };
    State GetState() const;

    bool StartResolving(const char8* address, uint16 port);
    void Stop();

    const addrinfo& Result() const;

public:
    struct HandleContext
    {
        AddressResolver* resolver;
        bool isFree;
    };

    using HandlePtr = std::unique_ptr < uv_getaddrinfo_t > ;

private:
    static uv_getaddrinfo_t* OccupyHandle(AddressResolver* resolver);
    static void UnbindResolver(uv_getaddrinfo_t* handle, const AddressResolver* resolver);

    static void GetAddrInfoCallback(uv_getaddrinfo_t* handle, int status, addrinfo* response);

    void GotAddrInfo(int status, addrinfo* response);

private:
    static List<HandlePtr> handlesHolder;
    static Map<uv_getaddrinfo_t*, HandleContext> handles;
    static Mutex handlesMutex;

private:

    uv_getaddrinfo_t* handle;
    addrinfo result;

    AddressRequester& requester;
    State state;
};




inline AddressResolver::State AddressResolver::GetState() const
{
    return state;
}

inline const addrinfo& AddressResolver::Result() const
{
    return result;
}

}
}

#endif

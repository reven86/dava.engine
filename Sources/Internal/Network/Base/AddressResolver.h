#ifndef __DAVAENGINE_ADDRESS_RESOLVER_H__
#define __DAVAENGINE_ADDRESS_RESOLVER_H__

#include "Base/BaseTypes.h"
#include "Functional/Function.h"
#include "Network/Base/Endpoint.h"
#include <libuv/uv.h>

namespace DAVA
{
namespace Net
{
class IOLoop;

class AddressResolver
{
public:
    using ResolverCallbackFn = Function<void(const Endpoint&, int32)>;

public:
    explicit AddressResolver(IOLoop* loop);
    ~AddressResolver();

    bool AsyncResolve(const char8* address, uint16 port, ResolverCallbackFn cbk);
    void Cancel();

private:
    static void GetAddrInfoCallback(uv_getaddrinfo_t* handle, int status, addrinfo* response);
    void GotAddrInfo(int status, addrinfo* response);

private:
    IOLoop* loop = nullptr;
    uv_getaddrinfo_t* handle = nullptr;
    ResolverCallbackFn resolverCallbackFn = nullptr;
};
}
}

#endif //__DAVAENGINE_ADDRESS_RESOLVER_H__

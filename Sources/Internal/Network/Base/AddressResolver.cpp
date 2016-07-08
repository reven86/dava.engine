#include "Network/Base/AddressResolver.h"
#include "Network/Base/Endpoint.h"
#include "Network/Base/IOLoop.h"
#include "Network/Base/NetworkUtils.h"
#include "Logger/Logger.h"
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
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(loop != nullptr);
    DVASSERT(handle == nullptr);
    DVASSERT(cbk != nullptr);

    handle = new uv_getaddrinfo_t;
    handle->data = this;

    struct addrinfo hints;
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_TCP;

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
#else
    return false;
#endif
}

void AddressResolver::Cancel()
{
#if !defined(DAVA_NETWORK_DISABLE)
    if (handle != nullptr)
    {
        handle->data = nullptr;
        uv_cancel(reinterpret_cast<uv_req_t*>(handle));
        handle = nullptr;
    }
#endif
}

void AddressResolver::GetAddrInfoCallback(uv_getaddrinfo_t* handle, int status, struct addrinfo* response)
{
#if !defined(DAVA_NETWORK_DISABLE)
    AddressResolver* resolver = static_cast<AddressResolver*>(handle->data);
    if (nullptr != resolver && resolver->handle != nullptr)
    {
        DVASSERT(resolver->handle == handle);
        resolver->GotAddrInfo(status, response);
        resolver->handle = nullptr;
    }

    SafeDelete(handle);

    uv_freeaddrinfo(response);
#endif
}

void AddressResolver::GotAddrInfo(int status, struct addrinfo* response)
{
    Endpoint endpoint;
    if (0 == status)
    {
        endpoint = Endpoint(response->ai_addr);
    }

    resolverCallbackFn(endpoint, status);
}

} // end of namespace Net
} // end of namespace DAVA

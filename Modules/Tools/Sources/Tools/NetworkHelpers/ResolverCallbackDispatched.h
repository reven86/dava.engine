#pragma once

#include <Network/Base/AddressResolver.h>
#include <Network/NetCore.h>

namespace DAVA
{
namespace Net
{
/**
    allows to call ResolverCallbackFn through dispatcher
*/

void ResolverCallbackWeak(std::weak_ptr<AddressResolver> resolverWeak, AddressResolver::ResolverCallbackFn resolverCallbackFn, const Endpoint& e, int32 i)
{
    std::shared_ptr<AddressResolver> resolver = resolverWeak.lock();
    if (resolver && !resolver->IsActive())
    {
        resolverCallbackFn(e, i);
    }
}

class ResolverCallbackDispatched : public AddressResolver::ResolverCallbackFn
{
public:
    explicit ResolverCallbackDispatched(Dispatcher<Function<void()>>* dispatcher, std::shared_ptr<AddressResolver>& resolverShared, AddressResolver::ResolverCallbackFn callbackFn)
        : dispatcher(dispatcher)
        , resolverWeak(resolverShared)
        , callbackFn(callbackFn)
    {
    }

    void operator()(const Endpoint& e, int32 i)
    {
        auto f = Bind(&ResolverCallbackWeak, resolverWeak, callbackFn, e, i);
        dispatcher->PostEvent(f);
    }

private:
    Dispatcher<Function<void()>>* dispatcher = nullptr;
    std::weak_ptr<AddressResolver> resolverWeak;
    AddressResolver::ResolverCallbackFn callbackFn;
};
}
}
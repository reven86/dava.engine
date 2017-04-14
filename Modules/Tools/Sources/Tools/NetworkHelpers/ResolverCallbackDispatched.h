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
class ResolverCallbackDispatched : public AddressResolver::ResolverCallbackFn
{
public:
    explicit ResolverCallbackDispatched(Dispatcher<Function<void()>>* dispatcher, AddressResolver::ResolverCallbackFn fn)
        : dispatcher(dispatcher)
        , fn(fn)
    {
    }

    void operator()(const Endpoint& e, int32 i)
    {
        auto f = Bind(fn, e, i);
        dispatcher->PostEvent(f);
    }

private:
    Dispatcher<Function<void()>>* dispatcher = nullptr;
    AddressResolver::ResolverCallbackFn fn;
};
}
}
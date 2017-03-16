#pragma once

#include <Network/ServiceRegistrar.h>
#include <Network/IChannel.h>
#include <Network/NetEventsDispatcher.h>
#include "Tools/NetworkHelpers/Private/ServicesDeleterExecutor.h"

namespace DAVA
{
namespace Net
{
/** ServiceDeleterAsync allows to pass call of ServiceDeleter functor in another thread

    ServiceCreatorAsync works in same manner.

    Example:

    NetEventsDispatcher dispatcher; // supposed that event will be placed in network thread and dispatched in user logic thread

    class A
    {
    public:
        A() : serviceDeleterAsync(MakeFunction(this, &A::Deleter), dispatcher) {}

        void InitNetwork()
        {
        ServiceCreator creator = MakeFunction(this, &A::Creator); // old way: creator will be invoked and executed in network thread
        ServiceDeleter deleter = MakeFunction(&serviceDeleterAsync, &ServiceDeleterAsync::ServiceDeleterCall);
        Net::NetCore::Instance()->RegisterService(myServiceID, creator, deleter);
        ....
        ....
        }

        IChannelListener* Creator(uint32, void*)
        {
            // some code that may be executed in network thread
        }

        void Deleter(IChannelListener*, void*)
        {
            // some code that should be executed in user logic thread
        }

    private:
        ServiceDeleterAsync serviceDeleterAsync;
    }
*/
class ServiceDeleterAsync
{
public:
    explicit ServiceDeleterAsync(ServiceDeleter serviceDeleter, NetEventsDispatcher* dispatcher);
    void ServiceDeleterCall(IChannelListener* obj, void* context);

private:
    std::shared_ptr<ServiceDeleterExecutor> serviceDeleterExecutor;
    NetEventsDispatcher* dispatcher = nullptr;
};
}
}

#pragma once

#include <Network/ServiceRegistrar.h>
#include <Network/IChannel.h>
#include <Network/NetCore.h>
#include "Tools/NetworkHelpers/Private/ServicesDeleterExecutor.h"

namespace DAVA
{
namespace Net
{
/** ServiceDeleterDispatched allows to pass call of ServiceDeleter functor in another thread

    ServiceCreatorDispatched works in same manner.

    Example:

    \code
    Dispatcher<Function<void()>> dispatcher; // supposed that event will be placed in network thread and dispatched in user logic thread

    class A
    {
    public:
        A() : serviceDeleterDispatched(MakeFunction(this, &A::Deleter), dispatcher) {}

        void InitNetwork()
        {
        ServiceCreator creator = MakeFunction(this, &A::Creator); // old way: creator will be invoked and executed in network thread
        ServiceDeleter deleter = MakeFunction(&serviceDeleterDispatched, &ServiceDeleterDispatched::ServiceDeleterCall);
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
        ServiceDeleterDispatched serviceDeleterDispatched;
    }
    \endcode
*/
class ServiceDeleterDispatched
{
public:
    explicit ServiceDeleterDispatched(ServiceDeleter serviceDeleter, Dispatcher<Function<void()>>* dispatcher);
    void ServiceDeleterCall(IChannelListener* obj, void* context);

private:
    std::shared_ptr<ServiceDeleterExecutor> serviceDeleterExecutor;
    Dispatcher<Function<void()>>* dispatcher = nullptr;
};
}
}

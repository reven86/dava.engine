#pragma once

#include <Network/ServiceRegistrar.h>
#include <Network/IChannel.h>
#include <Network/NetCore.h>
#include "Tools/NetworkHelpers/Private/ServicesCreatorExecutor.h"

namespace DAVA
{
namespace Net
{
/** ServiceCreatorDispatched allows to pass call of ServiceCreator functor in another thread

    ServiceDeleterDispatched works in same manner.
    
    Example:

    \code
    Dispatcher<Function<void()>> dispatcher; // supposed that event will be placed in network thread and dispatched in user logic thread

    class A
    {
    public:
        A() : serviceCreatorDispatched(MakeFunction(this, &A::Creator), dispatcher) {}

        void InitNetwork()
        {
            ServiceCreator creator = MakeFunction(&serviceCreatorDispatched, &ServiceCreatorDispatched::ServiceCreatorCall);
            ServiceDeleter deleter = MakeFunction(this, &A::Deleter); // old way: deleter will be invoked and executed in network thread
            Net::NetCore::Instance()->RegisterService(myServiceID, creator, deleter);
            ....
            ....
        }

        IChannelListener* Creator(uint32, void*)
        {
            // some code that should be executed in user logic thread
        }

        void Deleter(IChannelListener*, void*)
        {
            // some code that may be executed in network thread
        }

    private:
        ServiceCreatorDispatched serviceCreatorDispatched;
    }
    \endcode
*/
class ServiceCreatorDispatched
{
public:
    explicit ServiceCreatorDispatched(ServiceCreator serviceCreator, Dispatcher<Function<void()>>* dispatcher);
    IChannelListener* ServiceCreatorCall(uint32 serviceId, void* context);

private:
    Dispatcher<Function<void()>>* dispatcher = nullptr;
    std::shared_ptr<ServiceCreatorExecutor> serviceCreatorExecutor;
};
}
}

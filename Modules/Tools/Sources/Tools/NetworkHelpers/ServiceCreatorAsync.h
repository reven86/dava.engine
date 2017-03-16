#pragma once

#include <Network/ServiceRegistrar.h>
#include <Network/IChannel.h>
#include <Network/NetEventsDispatcher.h>
#include "Tools/NetworkHelpers/Private/ServicesCreatorExecutor.h"

namespace DAVA
{
namespace Net
{
/** ServiceCreatorAsync allows to pass call of ServiceCreator functor in another thread

    ServiceDeleterAsync works in same manner.
    
    Example:

    NetEventsDispatcher dispatcher; // supposed that event will be placed in network thread and dispatched in user logic thread

    class A
    {
    public:
        A() : serviceCreatorAsync(MakeFunction(this, &A::Creator), dispatcher) {}

        void InitNetwork()
        {
            ServiceCreator creator = MakeFunction(&serviceCreatorAsync, &ServiceCreatorAsync::ServiceCreatorCall);
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
        ServiceCreatorAsync serviceCreatorAsync;
    }
*/
class ServiceCreatorAsync
{
public:
    explicit ServiceCreatorAsync(ServiceCreator serviceCreator, NetEventsDispatcher* dispatcher);
    IChannelListener* ServiceCreatorCall(uint32 serviceId, void* context);

private:
    NetEventsDispatcher* dispatcher = nullptr;
    std::shared_ptr<ServiceCreatorExecutor> serviceCreatorExecutor;
};
}
}

#pragma once

#include <Network/ServiceRegistrar.h>
#include <Network/IChannel.h>

namespace DAVA
{
namespace Net
{
class ServiceCreatorExecutor
{
public:
    explicit ServiceCreatorExecutor(ServiceCreator fn)
        : serviceCreator(fn)
    {
    }

    void ServiceCreatorCall(uint32 serviceId, void* context)
    {
        targetFnResult = serviceCreator(serviceId, context);
    }

    ServiceCreator serviceCreator;
    IChannelListener* targetFnResult = nullptr;
};
}
}
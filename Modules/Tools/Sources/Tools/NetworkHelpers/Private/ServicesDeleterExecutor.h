#pragma once

#include <Network/ServiceRegistrar.h>
#include <Network/IChannel.h>

namespace DAVA
{
namespace Net
{
class ServiceDeleterExecutor
{
public:
    ServiceDeleterExecutor(ServiceDeleter serviceDeleter)
        : serviceDeleter(serviceDeleter)
    {
    }
    void ServiceDeleterCall(IChannelListener* obj, void* context)
    {
        serviceDeleter(obj, context);
    }

private:
    ServiceDeleter serviceDeleter;
};
}
}
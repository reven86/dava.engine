#include "Tools/NetworkHelpers/ServiceCreatorAsync.h"
#include <Utils/SafeMemberFnCaller.h>

namespace DAVA
{
namespace Net
{
ServiceCreatorAsync::ServiceCreatorAsync(ServiceCreator serviceCreator, NetEventsDispatcher* dispatcher)
    : dispatcher(dispatcher)
    , serviceCreatorExecutor(new ServiceCreatorExecutor(serviceCreator))
{
}

IChannelListener* ServiceCreatorAsync::ServiceCreatorCall(uint32 serviceId, void* context)
{
    std::weak_ptr<ServiceCreatorExecutor> targetObjectWeak(serviceCreatorExecutor);
    Function<void(ServiceCreatorExecutor*)> targetFn(Bind(&ServiceCreatorExecutor::ServiceCreatorCall, std::placeholders::_1, serviceId, context));
    auto targetFnCaller = &SafeMemberFnCaller<ServiceCreatorExecutor>;
    auto callerWithParams = Bind(targetFnCaller, targetFn, targetObjectWeak);
    Function<void()> msg(callerWithParams);
    dispatcher->SendEvent(msg);

    std::shared_ptr<ServiceCreatorExecutor> executor = targetObjectWeak.lock();
    return (executor ? executor->targetFnResult : nullptr);
}
}
}

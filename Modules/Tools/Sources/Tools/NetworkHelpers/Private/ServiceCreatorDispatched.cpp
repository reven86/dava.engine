#include "Tools/NetworkHelpers/ServiceCreatorDispatched.h"
#include "Tools/NetworkHelpers/SafeMemberFnCaller.h"

namespace DAVA
{
namespace Net
{
ServiceCreatorDispatched::ServiceCreatorDispatched(ServiceCreator serviceCreator, Dispatcher<Function<void()>>* dispatcher)
    : dispatcher(dispatcher)
    , serviceCreatorExecutor(new ServiceCreatorExecutor(serviceCreator))
{
}

IChannelListener* ServiceCreatorDispatched::ServiceCreatorCall(uint32 serviceId, void* context)
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

#include "Tools/NetworkHelpers/ServiceDeleterDispatched.h"
#include "Tools/NetworkHelpers/SafeMemberFnCaller.h"

namespace DAVA
{
namespace Net
{
ServiceDeleterDispatched::ServiceDeleterDispatched(ServiceDeleter serviceDeleter, Dispatcher<Function<void()>>* dispatcher)
    : serviceDeleterExecutor(new ServiceDeleterExecutor(serviceDeleter))
    , dispatcher(dispatcher)
{
}

void ServiceDeleterDispatched::ServiceDeleterCall(IChannelListener* obj, void* context)
{
    std::weak_ptr<ServiceDeleterExecutor> targetObjectWeak(serviceDeleterExecutor);
    Function<void(ServiceDeleterExecutor*)> targetFn(Bind(&ServiceDeleterExecutor::ServiceDeleterCall, std::placeholders::_1, obj, context));
    auto targetFnCaller = &SafeMemberFnCaller<ServiceDeleterExecutor>;
    auto callerWithParams = Bind(targetFnCaller, targetFn, targetObjectWeak);
    Function<void()> msg(callerWithParams);
    dispatcher->SendEvent(msg);
}
}
}

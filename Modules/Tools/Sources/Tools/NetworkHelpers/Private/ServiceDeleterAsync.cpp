#include "Tools/NetworkHelpers/ServiceDeleterAsync.h"
#include <Utils/SafeMemberFnCaller.h>

namespace DAVA
{
namespace Net
{

ServiceDeleterAsync::ServiceDeleterAsync(ServiceDeleter serviceDeleter, NetEventsDispatcher* dispatcher)
: serviceDeleterExecutor(new ServiceDeleterExecutor(serviceDeleter))
, dispatcher(dispatcher)
{
}

void ServiceDeleterAsync::ServiceDeleterCall(IChannelListener* obj, void* context)
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

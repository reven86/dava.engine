#include "Tools/NetworkHelpers/ChannelListenerDispatched.h"
#include "Tools/NetworkHelpers/SafeMemberFnCaller.h"
#include <Functional/Function.h>

namespace DAVA
{
namespace Net
{
ChannelListenerDispatched::ChannelListenerDispatched(std::weak_ptr<IChannelListener> listenerWeak, Dispatcher<Function<void()>>* netEventsDispatcher)
    : netEventsDispatcher(netEventsDispatcher)
    , targetObjectWeak(listenerWeak)
{
}

void ChannelListenerDispatched::OnChannelOpen(IChannel* channel)
{
    Function<void(IChannelListener*)> targetFn(Bind(&IChannelListener::OnChannelOpen, std::placeholders::_1, channel));
    auto targetFnCaller = &SafeMemberFnCaller<IChannelListener>;
    auto callerWithParams = Bind(targetFnCaller, targetFn, targetObjectWeak);
    Function<void()> msg(callerWithParams);
    netEventsDispatcher->PostEvent(msg);
}

void ChannelListenerDispatched::OnChannelClosed(IChannel* channel, const char8* message)
{
    Function<void(IChannelListener*)> targetFn(Bind(&IChannelListener::OnChannelClosed, std::placeholders::_1, channel, message));
    auto targetFnCaller = &SafeMemberFnCaller<IChannelListener>;
    auto callerWithParams = Bind(targetFnCaller, targetFn, targetObjectWeak);
    Function<void()> msg(callerWithParams);
    netEventsDispatcher->PostEvent(msg);
}

void ChannelListenerDispatched::OnPacketReceived(IChannel* channel, const void* buffer, size_t length)
{
    Function<void(IChannelListener*)> targetFn(Bind(&IChannelListener::OnPacketReceived, std::placeholders::_1, channel, buffer, length));
    auto targetFnCaller = &SafeMemberFnCaller<IChannelListener>;
    auto callerWithParams = Bind(targetFnCaller, targetFn, targetObjectWeak);
    Function<void()> msg(callerWithParams);
    netEventsDispatcher->PostEvent(msg);
}

void ChannelListenerDispatched::OnPacketSent(IChannel* channel, const void* buffer, size_t length)
{
    Function<void(IChannelListener*)> targetFn(Bind(&IChannelListener::OnPacketSent, std::placeholders::_1, channel, buffer, length));
    auto targetFnCaller = &SafeMemberFnCaller<IChannelListener>;
    auto callerWithParams = Bind(targetFnCaller, targetFn, targetObjectWeak);
    Function<void()> msg(callerWithParams);
    netEventsDispatcher->PostEvent(msg);
}

void ChannelListenerDispatched::OnPacketDelivered(IChannel* channel, uint32 packetId)
{
    Function<void(IChannelListener*)> targetFn(Bind(&IChannelListener::OnPacketDelivered, std::placeholders::_1, channel, packetId));
    auto targetFnCaller = &SafeMemberFnCaller<IChannelListener>;
    auto callerWithParams = Bind(targetFnCaller, targetFn, targetObjectWeak);
    Function<void()> msg(callerWithParams);
    netEventsDispatcher->PostEvent(msg);
}
}
}

#include "Tools/NetworkHelpers/ChannelListenerAsync.h"
#include <Utils/SafeMemberFnCaller.h>
#include <Functional/Function.h>

namespace DAVA
{
namespace Net
{

ChannelListenerAsync::ChannelListenerAsync(std::weak_ptr<IChannelListener> listenerWeak, NetEventsDispatcher* netEventsDispatcher)
    : netEventsDispatcher(netEventsDispatcher)
    , targetObjectWeak(listenerWeak)
{
}

void ChannelListenerAsync::OnChannelOpen(IChannel* channel)
{
    Function<void(IChannelListener*)> targetFn(Bind(&IChannelListener::OnChannelOpen, std::placeholders::_1, channel));
    auto targetFnCaller = &SafeMemberFnCaller<IChannelListener>;
    auto callerWithParams = Bind(targetFnCaller, targetFn, targetObjectWeak);
    Function<void()> msg(callerWithParams);
    netEventsDispatcher->PostEvent(msg);
}

void ChannelListenerAsync::OnChannelClosed(IChannel* channel, const char8* message)
{
    Function<void(IChannelListener*)> targetFn(Bind(&IChannelListener::OnChannelClosed, std::placeholders::_1, channel, message));
    auto targetFnCaller = &SafeMemberFnCaller<IChannelListener>;
    auto callerWithParams = Bind(targetFnCaller, targetFn, targetObjectWeak);
    Function<void()> msg(callerWithParams);
    netEventsDispatcher->PostEvent(msg);
}

void ChannelListenerAsync::OnPacketReceived(IChannel* channel, const void* buffer, size_t length)
{
    Function<void(IChannelListener*)> targetFn(Bind(&IChannelListener::OnPacketReceived, std::placeholders::_1, channel, buffer, length));
    auto targetFnCaller = &SafeMemberFnCaller<IChannelListener>;
    auto callerWithParams = Bind(targetFnCaller, targetFn, targetObjectWeak);
    Function<void()> msg(callerWithParams);
    netEventsDispatcher->PostEvent(msg);
}

void ChannelListenerAsync::OnPacketSent(IChannel* channel, const void* buffer, size_t length)
{
    Function<void(IChannelListener*)> targetFn(Bind(&IChannelListener::OnPacketSent, std::placeholders::_1, channel, buffer, length));
    auto targetFnCaller = &SafeMemberFnCaller<IChannelListener>;
    auto callerWithParams = Bind(targetFnCaller, targetFn, targetObjectWeak);
    Function<void()> msg(callerWithParams);
    netEventsDispatcher->PostEvent(msg);
}

void ChannelListenerAsync::OnPacketDelivered(IChannel* channel, uint32 packetId)
{
    Function<void(IChannelListener*)> targetFn(Bind(&IChannelListener::OnPacketDelivered, std::placeholders::_1, channel, packetId));
    auto targetFnCaller = &SafeMemberFnCaller<IChannelListener>;
    auto callerWithParams = Bind(targetFnCaller, targetFn, targetObjectWeak);
    Function<void()> msg(callerWithParams);
    netEventsDispatcher->PostEvent(msg);
}

}
}

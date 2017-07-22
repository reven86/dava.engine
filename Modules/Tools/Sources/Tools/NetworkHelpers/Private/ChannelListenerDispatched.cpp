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

/**
    OnPacketReceivedUsingTempBuffer works together with OnPacketReceived.
    It performs manual deletion of tempbuffer, that was created in OnPacketReceived function.
    The reason of using of temp buffer is that buffer passed to OnPacketReceived
    is valid only during OnPacketReceived call. Since our processing of OnPackedReceived
    is delayed, we need to remain buffer valid until its delayed invokation.
*/
void OnPacketReceivedUsingTempBuffer(std::weak_ptr<IChannelListener> listenerWeak, IChannel* channel, Vector<uint8> tempBuffer)
{
    std::shared_ptr<IChannelListener> listenerShared = listenerWeak.lock();
    if (listenerShared)
    {
        listenerShared->OnPacketReceived(channel, tempBuffer.data(), tempBuffer.size());
    }
}

void ChannelListenerDispatched::OnPacketReceived(IChannel* channel, const void* buffer, size_t length)
{
    Vector<uint8> tempBuf(length);
    Memcpy(tempBuf.data(), buffer, length);
    Function<void()> msg(Bind(&OnPacketReceivedUsingTempBuffer, targetObjectWeak, channel, std::move(tempBuf)));
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

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
    The reason of using of temp buffer is that our network uses single buffer for all incoming packets
    and that creates problems when processing of incoming packets become asynchronous.
*/
void OnPacketReceivedUsingTempBuffer(std::weak_ptr<IChannelListener> listenerWeak, IChannel* channel, uint8* tempBuffer, size_t length)
{
    std::shared_ptr<IChannelListener> listenerShared = listenerWeak.lock();
    if (listenerShared)
    {
        listenerShared->OnPacketReceived(channel, tempBuffer, length);
    }

    delete[] tempBuffer;
}

void ChannelListenerDispatched::OnPacketReceived(IChannel* channel, const void* buffer, size_t length)
{
    uint8* tempBuf = new uint8[length];
    Memcpy(tempBuf, buffer, length);
    Function<void()> msg(Bind(&OnPacketReceivedUsingTempBuffer, targetObjectWeak, channel, tempBuf, length));
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

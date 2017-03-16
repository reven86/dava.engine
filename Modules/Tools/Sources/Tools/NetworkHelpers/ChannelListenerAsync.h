#pragma once

#include <Network/IChannel.h>
#include <Network/NetEventsDispatcher.h>

namespace DAVA
{
namespace Net
{

/**
    ChannelListenerAsync is proxy class that passes IChannelListener callbacks to any other thread.
    
    ChannelListenerAsync uses NetEventsDispatcher for passing callbacks.
    ChannelListenerAsync guarantees that callback will not be called if at the moment of executing
    of that callback ChannelListenerAsync object is no more existing.

    Makes sense when running network in separate thread and willing to execute IChannelListener callbacks in other thread:
    in main thread, user thread etc.

    Example:

    // say A is a class that wants to use network and to process callbacks from net
    class A : public IChannelListener
    {
    public:
        void OnChannelOpen(IChannel*) override
        {
            // some code that should be executed on main thread
        }
    }

    NetEventsDispatcher dispatcher;
    std::shared_ptr<IChannelListener> a(new A);
    ChannelListenerAsync proxy(weak_ptr<IChannelListener>(a), dispatcher);

    // say this function will be called when connection is established.
    // Nework will then be using IChannelListener* pointer to invoke callbacks
    // on each network event.
    IChannelListener* ServiceCreate(uint32 serviceId, void* context)
    {
        return &proxy; // there we are passing pointer to ChannelListenerAsync object to network code.
    }

    // somewhere in net thread
    IChannelListener* eventsReceiver = ServiceCreate(param1, param2);
    ....
    eventsReceiver->OnChannelOpen(newChannel); // there callback to A::OnChannelOpen will be prepared and added to dispatcher
    
    // somewhere in user thread
    void OnUpdate()
    {
    // processing events accumulated by dispatcher
    if (dispatcher->HasEvents())
    {
        dispatcher->ProcessEvents(); // there callback to A::OnChannelOpen
    }
    }
*/
class ChannelListenerAsync : public IChannelListener
{
public:
    explicit ChannelListenerAsync(std::weak_ptr<IChannelListener> targetChannelListener, NetEventsDispatcher* netEventsDispatcher);

    void OnChannelOpen(IChannel* channel) override;
    void OnChannelClosed(IChannel* channel, const char8* message) override;
    void OnPacketReceived(IChannel* channel, const void* buffer, size_t length) override;
    void OnPacketSent(IChannel* channel, const void* buffer, size_t length) override;
    void OnPacketDelivered(IChannel* channel, uint32 packetId) override;

private:
    NetEventsDispatcher* netEventsDispatcher = nullptr;
    std::weak_ptr<IChannelListener> targetObjectWeak;
};
}
}

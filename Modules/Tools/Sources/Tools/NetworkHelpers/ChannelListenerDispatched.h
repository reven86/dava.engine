#pragma once

#include <Network/IChannel.h>
#include <Network/NetCore.h>

namespace DAVA
{
namespace Net
{
/**
    ChannelListenerDispatched is proxy class that passes IChannelListener callbacks to any other thread.
    
    ChannelListenerDispatched uses Dispatcher<Function<void()>> for passing callbacks.
    ChannelListenerDispatched guarantees that callback will not be called if at the moment of executing
    of that callback ChannelListenerDispatched object is no more existing.

    Makes sense when running network in separate thread and willing to execute IChannelListener callbacks in other thread:
    in main thread, user thread etc.

    Example:

    \code
    // say A is a class that wants to use network and to process callbacks from net
    class A : public IChannelListener
    {
    public:
        void OnChannelOpen(IChannel*) override
        {
            // some code that should be executed on main thread
        }
    }

    Dispatcher<Function<void()>> dispatcher;
    std::shared_ptr<IChannelListener> a(new A);
    ChannelListenerDispatched proxy(weak_ptr<IChannelListener>(a), dispatcher);

    // say this function will be called when connection is established.
    // Nework will then be using IChannelListener* pointer to invoke callbacks
    // on each network event.
    IChannelListener* ServiceCreate(uint32 serviceId, void* context)
    {
        return &proxy; // there we are passing pointer to ChannelListenerDispatched object to network code.
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
    \endcode
*/
class ChannelListenerDispatched : public IChannelListener
{
public:
    explicit ChannelListenerDispatched(std::weak_ptr<IChannelListener> targetChannelListener, Dispatcher<Function<void()>>* netEventsDispatcher);

    void OnChannelOpen(IChannel* channel) override;
    void OnChannelClosed(IChannel* channel, const char8* message) override;
    void OnPacketReceived(IChannel* channel, const void* buffer, size_t length) override;
    void OnPacketSent(IChannel* channel, const void* buffer, size_t length) override;
    void OnPacketDelivered(IChannel* channel, uint32 packetId) override;

private:
    Dispatcher<Function<void()>>* netEventsDispatcher = nullptr;
    std::weak_ptr<IChannelListener> targetObjectWeak;
};
}
}

#pragma once

#include "Engine/Dispatcher.h"

namespace DAVA
{
namespace Net
{
/** Dispatcher is intended to pass events from network thread to any other thread.

    Using this dispatcher makes sense only when running network on a separate thread.
    In this case, there might be cases when user wants to get callback that is invoked
    in network thread to be executed on user logic thread (for ex. on main thread).
    
    For example:
    
    NetEventsDispatcher dispatcher;

    class A
    {
        // say this function is called in network thread
        // but we want to process this callback in main thread
        void OnChannelOpen_Network()
        {
            Function<void()> f = MakeFunction(&A::OnChannelOpen_UserLogic);
            dispatcher->PostEvent(f);
        }

        // must be called in main thread
        void OnChannelOpen_UserLogic()
        {
            // user logic code
        }
    }

    // somewhere in main thread
    void OnUpdate()
    {
        // processing events accumulated by dispatcher
        if (dispatcher->HasEvents())
        {
            dispatcher->ProcessEvents(); // there callback on OnChannelOpen_UserLogic will be called
        }
    }
*/
using NetEventsDispatcher = Dispatcher<Function<void()>>;
}
}

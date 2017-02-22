#include "Network/NetCallbacksHolder.h"

namespace DAVA
{
namespace Net
{
NetCallbacksHolder::NetCallbacksHolder(Mode mode)
    : mode(mode)
{
}

void NetCallbacksHolder::AddCallback(const Function<void()>& fn)
{
    callbackQueue.Push(fn);
}

void NetCallbacksHolder::ExecPendingCallbacks()
{
    while (callbackQueue.PopAndExec())
    {
    }
}
}
}
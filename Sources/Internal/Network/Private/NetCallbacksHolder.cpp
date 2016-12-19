#include "Network/NetCallbacksHolder.h"

namespace DAVA
{
namespace Net
{
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
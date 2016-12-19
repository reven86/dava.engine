#pragma once

#include "Functional/Function.h"
#include "Job/JobQueue.h"

namespace DAVA
{
namespace Net
{
class NetCallbacksHolder
{
public:
    void AddCallback(const Function<void()>& fn);
    void ExecPendingCallbacks();

private:
    DAVA::JobQueueWorker callbackQueue;
};
}
}

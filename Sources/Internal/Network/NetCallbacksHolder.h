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
    enum Mode
    {
        ExecuteImmediately,
        AddInQueue
    };

    NetCallbacksHolder(Mode mode = ExecuteImmediately);

    Mode GetMode() const;

    void AddCallback(const Function<void()>& fn);
    void ExecPendingCallbacks();

private:
    DAVA::JobQueueWorker callbackQueue;
    Mode mode = ExecuteImmediately;
};

inline NetCallbacksHolder::Mode NetCallbacksHolder::GetMode() const
{
    return mode;
}
}
}

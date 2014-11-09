#pragma once

#include <Network/DeadlineTimer.h>

namespace DAVA
{

class IOLoop;

class TimerTest
{
public:
    TimerTest(IOLoop* loop);
    ~TimerTest();

    void Start();

private:
    void IssueWaitRequest(uint32 timeout);

    void HandleClose(DeadlineTimer* timer);
    void HandleTimer(DeadlineTimer* timer);

private:
    DeadlineTimer timer;
    uint32        count;
};

}   // namespace DAVA

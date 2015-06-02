#include <Base/FunctionTraits.h>

#include "TimerTest.h"

namespace DAVA
{

TimerTest::TimerTest(IOLoop* loop) : timer(loop), count(0)
{
    timer.SetCloseHandler(MakeFunction(this, &TimerTest::HandleClose));
}

TimerTest::~TimerTest() {}

void TimerTest::Start()
{
    IssueWaitRequest(0);
}

void TimerTest::IssueWaitRequest(uint32 timeout)
{
    timer.AsyncStartWait(timeout, MakeFunction(this, &TimerTest::HandleTimer));
}

void TimerTest::HandleClose(DeadlineTimer* timer)
{
    printf("Timer closed\n");
}

void TimerTest::HandleTimer(DeadlineTimer* timer)
{
    printf("Tick - %u\n", ++count);
    if (count < 10)
        IssueWaitRequest(500);
    else
        timer->Close();
}

}   // namespace DAVA

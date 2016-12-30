#include "Timer/RawTimer.h"
#include "Time/SystemTimer.h"

namespace DAVA
{
void RawTimer::Start()
{
    timerStartTime = SystemTimer::GetAbsoluteMillis();
    isStarted = true;
}

void RawTimer::Stop()
{
    isStarted = false;
}

void RawTimer::Resume()
{
    isStarted = true;
}

bool RawTimer::IsStarted()
{
    return isStarted;
}

int64 RawTimer::GetElapsed()
{
    if (isStarted)
    {
        return SystemTimer::GetAbsoluteMillis() - timerStartTime;
    }
    else
    {
        return 0;
    }
}
}
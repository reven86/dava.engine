#include "Timer/RawTimer.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
void RawTimer::Start()
{
    timerStartTime = SystemTimer::Instance()->AbsoluteMS();
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

uint64 RawTimer::GetElapsed()
{
    if (isStarted)
    {
        return SystemTimer::Instance()->AbsoluteMS() - timerStartTime;
    }
    else
    {
        return 0;
    }
}
}
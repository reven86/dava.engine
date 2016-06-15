#include "WinSystemTimer.h"

#include "Base/Platform.h"
#if defined(__DAVAENGINE_WIN_UAP__)
#include "WinApiUAP.h"
#elif defined(__DAVAENGINE_WIN32__)
#include <TimeAPI.h>
#endif

namespace DAVA
{
void EnableHighResolutionTimer(bool enable)
{
    bool timerServiceAvailable = false;
#if defined(__DAVAENGINE_WIN32__)
    timerServiceAvailable = true;
#elif defined(__DAVAENGINE_WIN_UAP__)
    timerServiceAvailable = WinApiUAP::IsAvailable(WinApiUAP::SYSTEM_TIMER_SERVICE);
#endif

    if (timerServiceAvailable)
    {
        TIMECAPS sistemTimerCaps;
        timeGetDevCaps(&sistemTimerCaps, sizeof(TIMECAPS));
        if (enable)
        {
            timeBeginPeriod(static_cast<UINT>(sistemTimerCaps.wPeriodMin));
        }
        else
        {
            timeEndPeriod(static_cast<UINT>(sistemTimerCaps.wPeriodMin));
        }
    }
}
}
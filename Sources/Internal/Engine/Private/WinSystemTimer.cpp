#include "WinSystemTimer.h"
#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)
#include "UWP/DllImportUWP.h"
#elif defined(__DAVAENGINE_WIN32__)
#include <TimeAPI.h>
#endif

namespace DAVA
{
namespace Private
{
void EnableHighResolutionTimer(bool enable)
{
#if defined(__DAVAENGINE_WIN32__)
    const bool timerServiceAvailable = true;
#elif defined(__DAVAENGINE_WIN_UAP__)
    using namespace DllImportUWP;

    const bool timerServiceAvailable =
        timeBeginPeriod != nullptr &&
        timeEndPeriod != nullptr &&
        timeGetDevCaps != nullptr;
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
}
#ifndef __DAVAENGINE_WINAPI_UAP_H__
#define __DAVAENGINE_WINAPI_UAP_H__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

typedef UINT MMRESULT;

typedef struct timecaps_tag
{
    UINT wPeriodMin; /* minimum period supported  */
    UINT wPeriodMax; /* maximum period supported  */
} TIMECAPS, *LPTIMECAPS;

extern MMRESULT(WINAPI* timeGetDevCaps)(LPTIMECAPS ptc, UINT cbtc);
extern MMRESULT(WINAPI* timeBeginPeriod)(UINT uPeriod);
extern MMRESULT(WINAPI* timeEndPeriod)(UINT uPeriod);

namespace WinApiUAP
{
enum eWinApiPart
{
    SYSTEM_TIMER_SERVICE,
};

void Initialize();
bool IsAvailable(eWinApiPart);
}

#endif

#endif // __DAVAENGINE_WINAPI_UAP_H__
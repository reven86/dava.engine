#pragma once

#include "Base/Platform.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN_UAP__)

namespace DAVA
{
namespace Private
{
namespace DllImportUWP
{
typedef UINT MMRESULT;

typedef struct timecaps_tag
{
    UINT wPeriodMin; /* minimum period supported  */
    UINT wPeriodMax; /* maximum period supported  */
} TIMECAPS, *LPTIMECAPS;

void Initialize();

extern MMRESULT(WINAPI* timeGetDevCaps)(LPTIMECAPS ptc, UINT cbtc);
extern MMRESULT(WINAPI* timeBeginPeriod)(UINT uPeriod);
extern MMRESULT(WINAPI* timeEndPeriod)(UINT uPeriod);
} // namespace DllImportUWP
} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__

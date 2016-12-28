#pragma once

#include "Base/Platform.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN_UAP__)

// Types from Windows SDK headers
typedef UINT MMRESULT;

typedef struct timecaps_tag
{
    UINT wPeriodMin; /* minimum period supported  */
    UINT wPeriodMax; /* maximum period supported  */
} TIMECAPS, *PTIMECAPS, NEAR *NPTIMECAPS, FAR *LPTIMECAPS;

namespace DAVA
{
namespace Private
{
// Static class that holds pointers to functions exported from DLLs.
// Windows API provides some useful functions which are not available for Universal Windows Platfrom,
// but those functions are present in system DLLs.
struct DllImport
{
    static void Initialize();

    // Kernel functions
    static HMODULE(WINAPI* fnLoadLibraryW)(LPCWSTR lpLibFileName);

    // Time API functions
    static MMRESULT(WINAPI* fnTimeGetDevCaps)(LPTIMECAPS ptc, UINT cbtc);
    static MMRESULT(WINAPI* fnTimeBeginPeriod)(UINT uPeriod);
    static MMRESULT(WINAPI* fnTimeEndPeriod)(UINT uPeriod);
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__

#include "DllImportUWP.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN_UAP__)

namespace DAVA
{
namespace Private
{
namespace DllImportUWP
{
MMRESULT(WINAPI* timeGetDevCaps)
(LPTIMECAPS ptc, UINT cbtc) = nullptr;
MMRESULT(WINAPI* timeBeginPeriod)
(UINT uPeriod) = nullptr;
MMRESULT(WINAPI* timeEndPeriod)
(UINT uPeriod) = nullptr;

void Initialize()
{
    MEMORY_BASIC_INFORMATION bi;
    VirtualQuery(static_cast<void*>(&GetModuleFileNameA), &bi, sizeof(bi));
    HMODULE hkernel = reinterpret_cast<HMODULE>(bi.AllocationBase);

    HMODULE(WINAPI * LoadLibraryW)
    (LPCWSTR lpLibFileName);
    LoadLibraryW = reinterpret_cast<decltype(LoadLibraryW)>(GetProcAddress(hkernel, "LoadLibraryW"));

    if (LoadLibraryW)
    {
        HMODULE hWinmm = LoadLibraryW(L"winmm.dll");
        if (hWinmm)
        {
            timeGetDevCaps = reinterpret_cast<decltype(timeGetDevCaps)>(GetProcAddress(hWinmm, "timeGetDevCaps"));
            timeBeginPeriod = reinterpret_cast<decltype(timeBeginPeriod)>(GetProcAddress(hWinmm, "timeBeginPeriod"));
            timeEndPeriod = reinterpret_cast<decltype(timeEndPeriod)>(GetProcAddress(hWinmm, "timeEndPeriod"));
        }
    }
}
}
}
}

#endif
#endif
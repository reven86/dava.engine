/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "WinApiUAP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Debug/DVAssert.h"

typedef UINT MMRESULT;
typedef struct timecaps_tag
{
    UINT wPeriodMin; /* minimum period supported  */
    UINT wPeriodMax; /* maximum period supported  */
} TIMECAPS, *LPTIMECAPS;

namespace DAVA
{
namespace WinApiUAP
{
namespace //private variables
{
bool initialized = false;

MMRESULT(WINAPI* timeGetDevCaps)
(LPTIMECAPS ptc, UINT cbtc) = nullptr;
MMRESULT(WINAPI* timeBeginPeriod)
(UINT uPeriod) = nullptr;
MMRESULT(WINAPI* timeEndPeriod)
(UINT uPeriod) = nullptr;
};

void Initialize()
{
    if (!initialized)
    {
        // Here land of black magic and fire-spitting dragons begins
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

        initialized = true;
    }
}

void TimeBeginPeriod(uint32 msPeriod)
{
    if (timeBeginPeriod)
        timeBeginPeriod(static_cast<UINT>(msPeriod));
}

void TimeEndPeriod(uint32 msPeriod)
{
    if (timeEndPeriod)
        timeEndPeriod(static_cast<UINT>(msPeriod));
}

uint32 TimeGetMinPeriod()
{
    if (timeGetDevCaps)
    {
        TIMECAPS sistemTimerCaps;
        timeGetDevCaps(&sistemTimerCaps, sizeof(TIMECAPS));
        return static_cast<uint32>(sistemTimerCaps.wPeriodMin);
    }

    return 0;
}

uint32 TimeGetMaxPeriod()
{
    if (timeGetDevCaps)
    {
        TIMECAPS sistemTimerCaps;
        timeGetDevCaps(&sistemTimerCaps, sizeof(TIMECAPS));
        return static_cast<uint32>(sistemTimerCaps.wPeriodMax);
    }

    return 0;
}
};
};

#endif

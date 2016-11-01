#pragma once
#include "Base/Platform.h"
#include "Debug//DVAssert.h"
#include "Logger/Logger.h"

#ifdef DAVA_IMPLEMENT_DYNAMIC_MODULE
    #ifdef __DAVAENGINE_WIN32__
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    TCHAR dllName[1024];
    DWORD result = GetModuleFileName(hinstDLL, dllName, 1024);
    DVASSERT(result != 0);

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DAVA::Logger::FrameworkDebug("Attached dynamic module %S", dllName);
        break;
    case DLL_PROCESS_DETACH:
        DAVA::Logger::FrameworkDebug("Detached dynamic module %S", dllName);
        break;
    }

    return TRUE;
}
    #endif
#endif
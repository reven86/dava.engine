#pragma once
#include "Base/Platform.h"

#ifdef DAVA_IMPLEMENT_DYNAMIC_MODULE
    #ifdef __DAVAENGINE_WIN32__
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    int i = 0;
}
    #endif
#endif
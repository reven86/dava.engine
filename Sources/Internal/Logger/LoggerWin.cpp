#include "Logger/Logger.h"

#if defined(__DAVAENGINE_WINDOWS__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

namespace DAVA
{
void Logger::PlatformLog(eLogLevel ll, const char8* text) const
{
    OutputDebugStringA(text);
}
}

#endif

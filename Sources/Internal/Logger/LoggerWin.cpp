#include "Logger/Logger.h"

#if defined(__DAVAENGINE_WINDOWS__)

#include "Base/Platform.h"

namespace DAVA
{
void Logger::PlatformLog(eLogLevel ll, const char8* text)
{
    ::OutputDebugStringA(text);
}
}

#endif

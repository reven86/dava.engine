#include "FileSystem/Logger.h"

#if defined(__DAVAENGINE_NACL__)

namespace DAVA 
{

void Logger::PlatformLog(eLogLevel ll, const char8* text)
{
	fprintf(stderr, "%s",text);
}

void Logger::PlatformLog(eLogLevel ll, const char16* text)
{
	wprintf(L"%s", text);
}

}
#endif 

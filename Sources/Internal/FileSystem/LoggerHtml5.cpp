#include "FileSystem/Logger.h"

#if defined(__DAVAENGINE_HTML5__)

namespace DAVA 
{

void Logger::PlatformLog(eLogLevel ll, const char8* text)
{
	printf("%s", text);
}

void Logger::PlatformLog(eLogLevel ll, const char16* text)
{
	printf("%s", text);
}

}
#endif 

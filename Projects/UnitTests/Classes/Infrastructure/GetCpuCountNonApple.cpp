#include "Base/BaseTypes.h"

#ifndef __DAVAENGINE_APPLE__

using namespace DAVA;

#ifdef __DAVAENGINE_ANDROID__
#include <unistd.h>

int32 GetCpuCount()
{
    return sysconf(_SC_NPROCESSORS_CONF);
}
#endif

#ifdef __DAVAENGINE_WINDOWS__
int32 GetCpuCount()
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}
#endif

#endif // !__DAVAENGINE_APPLE__
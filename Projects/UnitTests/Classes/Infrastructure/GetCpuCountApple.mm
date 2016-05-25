#include "Base/BaseTypes.h"

#ifdef __DAVAENGINE_APPLE__

#import <Foundation/NSProcessInfo.h>

using namespace DAVA;

int32 GetCpuCount()
{
    NSUInteger processorCount = [[NSProcessInfo processInfo] processorCount];
    return static_cast<int32>(processorCount);
}

#endif // __DAVAENGINE_APPLE__
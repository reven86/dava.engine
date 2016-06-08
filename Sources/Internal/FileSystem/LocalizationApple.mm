#include "Base/Platform.h"

#if defined(__DAVAENGINE_APPLE__)

#import <Foundation/Foundation.h>

#include "FileSystem/LocalizationSystem.h"

namespace DAVA
{
String LocalizationSystem::GetDeviceLocale(void) const
{
    NSArray* ar = [NSLocale preferredLanguages];

    String ret([[ar objectAtIndex:0] UTF8String]);
    return ret;
}
}
#endif
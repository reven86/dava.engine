#include "FileSystem/FilePath.h"

#if defined(__DAVAENGINE_IPHONE__)

#import <Foundation/NSString.h>
#import <Foundation/NSBundle.h>

namespace DAVA
{
void FilePath::InitializeBundleName()
{
    NSString* bundlePath = [[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/Data/"];
    SetBundleName([bundlePath UTF8String]);
}
}

#endif //#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)

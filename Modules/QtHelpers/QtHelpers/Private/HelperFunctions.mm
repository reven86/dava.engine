#include "HelperFunctions.h"


#import <Foundation/Foundation.h>

namespace QtHelpers
{
//realisation for OS X which invokes given function inside autorelease pool
#if defined(__DAVAENGINE_MACOS__)
void InvokeInAutoreleasePool(std::function<void()> function);
{
    @autoreleasepool
    {
        function();
    }
}
#endif //__DAVAENGINE_MACOS__
}

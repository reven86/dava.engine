#include "QtHelpers/HelperFunctions.h"
#import <Foundation/Foundation.h>
#include <QtGlobal>

namespace QtHelpers
{
//realisation for OS X which invokes given function inside autorelease pool
#if defined(Q_OS_MAC)
void InvokeInAutoreleasePool(std::function<void()> function)
{
    @autoreleasepool
    {
        function();
    }
}
#endif //Q_OS_MAC
}

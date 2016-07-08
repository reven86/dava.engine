#include "Base/BaseTypes.h"
#include "Platform/DPIHelper.h"

#include <AppKit/NSScreen.h>
#include <ApplicationServices/ApplicationServices.h>
#import <Foundation/Foundation.h>

namespace DAVA
{
uint32 DPIHelper::GetScreenDPI()
{
    NSScreen* screen = [NSScreen mainScreen];
    NSDictionary* description = [screen deviceDescription];
    NSSize displayPixelSize = [[description objectForKey:NSDeviceSize] sizeValue];
    CGSize displayPhysicalSize =
    CGDisplayScreenSize([[description objectForKey:@"NSScreenNumber"] unsignedIntValue]);

    if (displayPhysicalSize.width == 0.0f)
    {
        return 0;
    }
    return (displayPixelSize.width / displayPhysicalSize.width) * 25.4f;
}

float64 DPIHelper::GetDpiScaleFactor(int32 screenId)
{
    NSArray* screens = [NSScreen screens];
    NSScreen* screen = [screens objectAtIndex:screenId];
    const float64 scale = [screen backingScaleFactor];

    return scale;
}

Size2i DPIHelper::GetScreenSize()
{
    Size2i screenSize;
    NSScreen* screen = [NSScreen mainScreen];
    NSRect screenRect = [screen frame];
    screenSize.dx = screenRect.size.width;
    screenSize.dy = screenRect.size.height;

    return screenSize;
}
}
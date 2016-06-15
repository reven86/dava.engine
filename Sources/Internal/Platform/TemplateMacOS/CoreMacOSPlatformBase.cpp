#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_MACOS__)

#include "CoreMacOSPlatformBase.h"
#include <ApplicationServices/ApplicationServices.h>

namespace DAVA
{
int GetBPPFromMode(CGDisplayModeRef displayMode)
{
    CFStringRef pixelEncoding = CGDisplayModeCopyPixelEncoding(displayMode);

    int depth = 0;
    if ((CFStringCompare(pixelEncoding, CFSTR(kIO30BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo))
    {
        depth = 30;
    }
    else if (CFStringCompare(pixelEncoding, CFSTR(IO32BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
    {
        depth = 32;
    }
    else if (CFStringCompare(pixelEncoding, CFSTR(IO16BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
    {
        depth = 16;
    }
    else if (CFStringCompare(pixelEncoding, CFSTR(IO8BitIndexedPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
    {
        depth = 8;
    }

    CFRelease(pixelEncoding);

    return depth;
}

void CoreMacOSPlatformBase::GetAvailableDisplayModes(List<DisplayMode>& availableModes)
{
    CFArrayRef availableModesSystem = CGDisplayCopyAllDisplayModes(kCGDirectMainDisplay, nullptr);
    int32 numberOfAvailableModes = CFArrayGetCount(availableModesSystem);

    for (int modeIndex = 0; modeIndex < numberOfAvailableModes; ++modeIndex)
    {
        // look at each mode in the available list
        CGDisplayModeRef modeSystem = static_cast<CGDisplayModeRef>(const_cast<void*>(CFArrayGetValueAtIndex(availableModesSystem, modeIndex)));

        DisplayMode mode;
        mode.width = CGDisplayModeGetWidth(modeSystem);
        mode.height = CGDisplayModeGetHeight(modeSystem);
        mode.refreshRate = CGDisplayModeGetRefreshRate(modeSystem);
        mode.bpp = GetBPPFromMode(modeSystem);
        availableModes.push_back(mode);
    }

    CFRelease(availableModesSystem);
}
}

#endif // #if defined(__DAVAENGINE_MACOS__)

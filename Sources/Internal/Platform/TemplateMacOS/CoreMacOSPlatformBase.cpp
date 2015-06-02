/*==================================================================================
 Copyright (c) 2008, binaryzebra
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the binaryzebra nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/


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
	
DisplayMode CoreMacOSPlatformBase::GetCurrentDisplayMode()
{
    CGDisplayModeRef currentMode = CGDisplayCopyDisplayMode(kCGDirectMainDisplay);
        
    DisplayMode mode;
    mode.width = CGDisplayModeGetWidth(currentMode);
    mode.height = CGDisplayModeGetHeight(currentMode);
    mode.refreshRate = CGDisplayModeGetRefreshRate(currentMode);
    mode.bpp = GetBPPFromMode(currentMode);
    
    CGDisplayModeRelease(currentMode);

    return mode;
}

void CoreMacOSPlatformBase::GetAvailableDisplayModes(List<DisplayMode> & availableModes)
{
    CFArrayRef availableModesSystem = CGDisplayCopyAllDisplayModes(kCGDirectMainDisplay, NULL);
    int32 numberOfAvailableModes = CFArrayGetCount(availableModesSystem);

    for (int modeIndex = 0; modeIndex < numberOfAvailableModes; ++modeIndex)
    {
        // look at each mode in the available list
        CGDisplayModeRef modeSystem = (CGDisplayModeRef)CFArrayGetValueAtIndex(availableModesSystem, modeIndex);

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

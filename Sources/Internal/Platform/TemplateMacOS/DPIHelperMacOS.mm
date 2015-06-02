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
#include "Platform/DPIHelper.h"

#include <AppKit/NSScreen.h>
#include <ApplicationServices/ApplicationServices.h>
#import <Foundation/Foundation.h>

namespace DAVA
{
    uint32 DPIHelper::GetScreenDPI()
    {
        
        NSScreen *screen = [NSScreen mainScreen];
        NSDictionary *description = [screen deviceDescription];
        NSSize displayPixelSize = [[description objectForKey:NSDeviceSize] sizeValue];
        CGSize displayPhysicalSize =
            CGDisplayScreenSize([[description objectForKey:@"NSScreenNumber"] unsignedIntValue]);
        
        if(displayPhysicalSize.width == 0)
        {
            return 0;
        }
        return  (displayPixelSize.width / displayPhysicalSize.width) * 25.4f;
    }
	
    float64 DPIHelper::GetDpiScaleFactor(int32 screenId)
    {
        NSArray *screens = [NSScreen screens];
        NSScreen *screen = [screens objectAtIndex: screenId];
        const float64 scale = [screen backingScaleFactor];
        
        return scale;
    }
    
    Size2i DPIHelper::GetScreenSize()
    {
        Size2i screenSize;
        NSScreen *screen = [NSScreen mainScreen];
        NSRect screenRect = [screen frame];
        screenSize.dx = screenRect.size.width;
        screenSize.dy = screenRect.size.height;
        
        return screenSize;
    }
}
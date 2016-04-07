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

#include <Cocoa/Cocoa.h>
#include "Core/Core.h"
#include "FileSystem/FileSystem.h"
#include "Input/InputSystem.h"
#include "MouseCaptureMacOS.h"

namespace DAVA
{
void MouseCapturePrivate::SetNativePining(const eMouseCaptureMode& newMode)
{
    switch (newMode)
    {
    case eMouseCaptureMode::OFF:
        OSXShowCursor();
        CGAssociateMouseAndMouseCursorPosition(true);
        break;
    case eMouseCaptureMode::PINING:
        OSXHideCursor();
        MovePointerToWindowCenter();
        CGAssociateMouseAndMouseCursorPosition(false);
        break;
    case eMouseCaptureMode::FRAME:
    // Unsupported yet
    default:
        DVASSERT_MSG(false, "Unsupported cursor capture mode");
    }
}

void MouseCapturePrivate::SetCursorInCenter()
{
}

void MouseCapturePrivate::MovePointerToWindowCenter()
{
    NSRect windowRect = [[static_cast<NSView*>(Core::Instance()->GetNativeView()) window] frame];
    NSRect screenRect = [[NSScreen mainScreen] frame];

    // Window origin is at bottom-left edge, but CGWarpMouseCursorPosition requires point in screen coordinates
    windowRect.origin.y = screenRect.size.height - (windowRect.origin.y + windowRect.size.height);
    float x = windowRect.origin.x + windowRect.size.width / 2.0f;
    float y = windowRect.origin.y + windowRect.size.height / 2.0f;
    CGWarpMouseCursorPosition(CGPointMake(x, y));
}

void MouseCapturePrivate::OSXShowCursor()
{
    if (!cursorVisible)
    {
        [NSCursor unhide];
        cursorVisible = true;
    }
}

void MouseCapturePrivate::OSXHideCursor()
{
    if (cursorVisible)
    {
        [NSCursor hide];
        cursorVisible = false;
    }
}

} //  namespace DAVA

#endif // __DAVAENGINE_MACOS__

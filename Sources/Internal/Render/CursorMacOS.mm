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
#include "Render/Cursor.h"
#include "FileSystem/FileSystem.h"

#if defined(__DAVAENGINE_MACOS__) 


#if defined(Q_OS_MAC)
#include "Platform/Qt/MacOS/CorePlatformMacOS.h"
#else //#if defined(Q_OS_MAC)
#include "Platform/TemplateMacOS/CorePlatformMacOS.h"
#endif //#if defined(Q_OS_MAC)
#include <Cocoa/Cocoa.h>

int mouseMoveSkipCounter = 0;

// This variable is used to control cursor hide/unhide balancing
// See Apple doc for NSCursor's hide and unhide methods
// https://developer.apple.com/library/mac/documentation/Cocoa/Reference/ApplicationKit/Classes/NSCursor_Class/
static bool cursorVisible = true;

void OSXShowCursor()
{
    if (!cursorVisible)
    {
        [NSCursor unhide];
        cursorVisible = true;
    }
}

void OSXHideCursor()
{
    if (cursorVisible)
    {
        [NSCursor hide];
        cursorVisible = false;
    }
}

void MovePointerToWindowCenter()
{
    NSRect windowRect = [[static_cast<NSView*>(DAVA::Core::Instance()->GetNativeView()) window] frame];
    NSRect screenRect = [[NSScreen mainScreen] frame];

    // Window origin is at bottom-left edge, but CGWarpMouseCursorPosition requires point in screen coordinates
    windowRect.origin.y = screenRect.size.height - (windowRect.origin.y + windowRect.size.height);
    float x = windowRect.origin.x + windowRect.size.width / 2.0f;
    float y = windowRect.origin.y + windowRect.size.height / 2.0f;
    CGWarpMouseCursorPosition(CGPointMake(x, y));
}

void OSXEnablePinning()
{
    // If mouse pointer was outside window rectangle when enabling pinning mode then
    // mouse clicks are forwarded to other windows and our application loses focus.
    // So move mouse pointer to window center before enabling pinning mode.
    // Secondly, after using CGWarpMouseCursorPosition function to center mouse pointer
    // mouse move events arrive with big delta which causes mouse hopping.
    // The best solution I have investigated is to skip first N mouse move events after enabling
    // pinning mode: global variable mouseMoveSkipCounter is set to some reasonable value
    // and is checked in OpenGLView's process method to skip mouse move events

    const int SKIP_N_MOUSE_MOVE_EVENTS = 4;

    mouseMoveSkipCounter = SKIP_N_MOUSE_MOVE_EVENTS;
    OSXHideCursor();
    MovePointerToWindowCenter();
    CGAssociateMouseAndMouseCursorPosition(false);
}

void OSXDisablePinning()
{
    mouseMoveSkipCounter = 0;
    OSXShowCursor();
    CGAssociateMouseAndMouseCursorPosition(true);
}

namespace DAVA
{
static InputSystem::eMouseCaptureMode systemCursorCaptureMode = InputSystem::eMouseCaptureMode::OFF;

bool Cursor::SetMouseCaptureMode(InputSystem::eMouseCaptureMode mode)
{
    switch (mode)
    {
    case InputSystem::eMouseCaptureMode::OFF:
        OSXDisablePinning();
        systemCursorCaptureMode = mode;
        return true;
    case InputSystem::eMouseCaptureMode::PINING:
        OSXEnablePinning();
        systemCursorCaptureMode = mode;
        return true;
    case InputSystem::eMouseCaptureMode::FRAME:
    // Unsupported yet
    default:
        DVASSERT_MSG(false, "Unsupported cursor capture mode");
        return false;
    }
}

InputSystem::eMouseCaptureMode Cursor::GetMouseCaptureMode()
{
    return systemCursorCaptureMode;
}

static bool systemCursorVisibility = false;

bool Cursor::SetSystemCursorVisibility(bool show)
{
    if(show)
    {
        OSXShowCursor();
    }
    else
    {
        OSXHideCursor();
    }
    systemCursorVisibility = show;
    return true;
}

bool Cursor::GetSystemCursorVisibility()
{
    return systemCursorVisibility;
}
	
};


#endif // __DAVAENGINE_MACOS__

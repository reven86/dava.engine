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

namespace DAVA
{
static InputSystem::eMouseCaptureMode systemCursorCaptureMode = InputSystem::eMouseCaptureMode::OFF;

bool Cursor::SetMouseCaptureMode(InputSystem::eMouseCaptureMode mode)
{
    switch (mode)
    {
    case InputSystem::eMouseCaptureMode::OFF:
        SetSystemCursorVisibility(true);
        CGAssociateMouseAndMouseCursorPosition(true);
        systemCursorCaptureMode = mode;
        return true;
    case InputSystem::eMouseCaptureMode::PINING:
        SetSystemCursorVisibility(false);
        CGAssociateMouseAndMouseCursorPosition(false);
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
#ifdef __DAVAENGINE_NPAPI__
    CGDirectDisplayID displayID = 0; //this parameter is ignored on MacOS.
    if (show)
    {
        CGDisplayShowCursor(displayID);
    }
    else
    {
        CGDisplayHideCursor(displayID);
    }
#else
    if (show)
        [NSCursor unhide];
    else
    {
        [NSCursor hide];
#endif
    systemCursorVisibility = show;
    return true;
}

bool Cursor::GetSystemCursorVisibility()
{
    return systemCursorVisibility;
}
};


#endif // __DAVAENGINE_MACOS__

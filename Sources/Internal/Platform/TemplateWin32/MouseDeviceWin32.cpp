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

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN32__)

#include "Platform/TemplateWin32/MouseDeviceWin32.h"
#include "Platform/TemplateWin32/CorePlatformWin32.h"

namespace DAVA
{
void MouseDeviceWin32::SetCursorInCenter()
{
    HWND hWnd = static_cast<HWND>(DAVA::Core::Instance()->GetNativeView());
    RECT wndRect;
    GetWindowRect(hWnd, &wndRect);
    int centerX = static_cast<int>((wndRect.left + wndRect.right) >> 1);
    int centerY = static_cast<int>((wndRect.bottom + wndRect.top) >> 1);
    SetCursorPos(centerX, centerY);
}

bool MouseDeviceWin32::SkipEvents()
{
    return false;
}

bool MouseDeviceWin32::SetSystemCursorVisibility(bool show)
{
    DAVA::int32 showCount = 0;
    showCount = ShowCursor(show); // No cursor info available, just call

    if (show && showCount >= 0)
    {
        // If system cursor is visible then showCount should be >= 0
        lastSystemCursorShowState = true;
    }
    else if (!show && showCount < 0)
    {
        // If system cursor is not visible then showCount should be -1
        lastSystemCursorShowState = false;
    }
    else
    {
        // Setup failure
        return false;
    }
    return true;
}

void MouseDeviceWin32::SetMode(eCaptureMode newMode)
{
    switch (newMode)
    {
    case eCaptureMode::OFF:
    case eCaptureMode::PINING:
    {
        SetSystemCursorVisibility(newMode != eCaptureMode::PINING);
        if (newMode == eCaptureMode::PINING)
        {
            POINT p;
            GetCursorPos(&p);
            lastCursorPosition.x = p.x;
            lastCursorPosition.y = p.y;

            SetCursorInCenter();
        }
        else
        {
            SetCursorPos(lastCursorPosition.x, lastCursorPosition.y);
        }
        break;
    }
    case eCaptureMode::FRAME:
        Logger::Error("Unsupported cursor capture mode");
        break;
    default:
        DVASSERT_MSG(false, "Incorrect cursor capture mode");
        Logger::Error("Incorrect cursor capture mode");
        break;
    }
}

} //  namespace DAVA

#endif //  __DAVAENGINE_WIN32__

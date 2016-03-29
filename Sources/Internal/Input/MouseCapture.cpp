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

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_WIN_UAP__)

#if defined(__DAVAENGINE_MACOS__)
#include "Platform/TemplateMacOS/MouseCaptureMacOS.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Platform/TemplateWin32/MouseCaptureWin32.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Platform/TemplateWin32/MouseCaptureWinUAP.h"
#endif

#include "Core/Core.h"

namespace DAVA
{
MouseCapturePrivate* MouseCapture::GetPrivateImpl()
{
    static MouseCapturePrivate privateImpl;
    return &privateImpl;
}

void MouseCapture::SetMouseCaptureMode(InputSystem::eMouseCaptureMode newMode)
{
    if (mode != newMode)
    {
        mode = newMode;
        if (InputSystem::eMouseCaptureMode::OFF == mode)
        {
            SetNativePining(mode);
            deferredCapture = false;
        }

        if (InputSystem::eMouseCaptureMode::PINING == mode)
        {
            if (focused && !focusChenged)
            {
                SetNativePining(mode);
            }
            else
            {
                deferredCapture = true;
            }
        }
    }
}

InputSystem::eMouseCaptureMode MouseCapture::GetMouseCaptureMode()
{
    return mode;
}

InputSystem::eMouseCaptureMode MouseCapture::GetMouseCaptureModeNative()
{
    return nativeMode;
}

void MouseCapture::SetApplicationFocus(bool isFocused)
{
    if (focused != isFocused)
    {
        focusChenged = true;
        if (firstEntered)
        {
            firstEntered = false;
            focusChenged = false;
        }

        focused = isFocused;
        if (InputSystem::eMouseCaptureMode::PINING == mode)
        {
            if (focused)
            {
                deferredCapture = true;
            }
            else
            {
                SetNativePining(InputSystem::eMouseCaptureMode::OFF);
            }
        }
    }
}

bool MouseCapture::SkipEvents(UIEvent* event)
{
    focusChenged = false;
    if (InputSystem::eMouseCaptureMode::PINING == mode && focused && !deferredCapture)
    {
        GetPrivateImpl()->SetCursorPosition();
    }
    if (!deferredCapture && focused)
    {
        return false;
    }
    if (event->phase == UIEvent::Phase::ENDED)
    {
        bool inRect = true;
        Vector2 windowSize = Core::Instance()->GetWindowSize();
        inRect &= (event->point.x >= 0.f && event->point.x <= windowSize.x);
        inRect &= (event->point.y >= 0.f && event->point.y <= windowSize.y);
        if (inRect)
        {
            deferredCapture = false;
            SetNativePining(InputSystem::eMouseCaptureMode::PINING);
        }
    }
    return true;
}

void MouseCapture::SetNativePining(InputSystem::eMouseCaptureMode newNativeMode)
{
    if (newNativeMode != nativeMode)
    {
        nativeMode = newNativeMode;
        GetPrivateImpl()->SetNativePining(nativeMode);
    }
}

} // namespace DAVA

#else //  __DAVAENGINE_IPHONE__ || __DAVAENGINE_ANDROID__

namespace DAVA
{
void MouseCapture::SetMouseCaptureMode(InputSystem::eMouseCaptureMode newMode)
{
}

InputSystem::eMouseCaptureMode MouseCapture::GetMouseCaptureMode()
{
    return InputSystem::eMouseCaptureMode::OFF;
}

InputSystem::eMouseCaptureMode MouseCapture::GetMouseCaptureModeNative()
{
    return InputSystem::eMouseCaptureMode::OFF;
}

void MouseCapture::SetApplicationFocus(bool isFocused)
{
}

bool MouseCapture::SkipEvents(UIEvent* event)
{
    return false;
}

void MouseCapture::SetNativePining(InputSystem::eMouseCaptureMode newativeMode)
{
}

MouseCapturePrivate* MouseCapture::GetPrivateImpl()
{
    return nullptr;
}

} //namespace DAVA

#endif // defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_WIN_UAP__)

DAVA::InputSystem::eMouseCaptureMode DAVA::MouseCapture::mode = DAVA::InputSystem::eMouseCaptureMode::OFF;
DAVA::InputSystem::eMouseCaptureMode DAVA::MouseCapture::nativeMode = DAVA::InputSystem::eMouseCaptureMode::OFF;
bool DAVA::MouseCapture::focused = false;
bool DAVA::MouseCapture::focusChenged = false;
bool DAVA::MouseCapture::firstEntered = true;
bool DAVA::MouseCapture::deferredCapture = false;
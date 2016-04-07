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
#include "Input/InputSystem.h"
#include "Input/MouseCapture.h"

namespace DAVA
{
struct MouseCaptureContext
{
    eMouseCaptureMode mode = eMouseCaptureMode::OFF;
    eMouseCaptureMode nativeMode = eMouseCaptureMode::OFF;
    bool focused = false;
    bool focusChanged = false;
    bool firstEntered = true;
    bool deferredCapture = false;
};

void SetNativePining(const eMouseCaptureMode& newNativeMode, eMouseCaptureMode& nativeMode, const std::unique_ptr<MouseCapturePrivate>& privateImpl)
{
    if (newNativeMode != nativeMode)
    {
        nativeMode = newNativeMode;
        privateImpl->SetNativePining(nativeMode);
    }
}

MouseCapture::MouseCapture()
{
    Core::Instance()->focusChanged.Connect(this, &MouseCapture::OnFocused);
    context = std::make_unique<MouseCaptureContext>();
    privateImpl = std::make_unique<MouseCapturePrivate>();
}

void MouseCapture::SetMode(const eMouseCaptureMode& newMode)
{
    if (context->mode != newMode)
    {
        context->mode = newMode;
        if (eMouseCaptureMode::OFF == context->mode)
        {
            SetNativePining(context->mode, context->nativeMode, privateImpl);
            context->deferredCapture = false;
        }

        if (eMouseCaptureMode::PINING == context->mode)
        {
            if (context->focused && !context->focusChanged)
            {
                SetNativePining(context->mode, context->nativeMode, privateImpl);
            }
            else
            {
                context->deferredCapture = true;
            }
        }
    }
}

eMouseCaptureMode MouseCapture::GetMode() const
{
    return context->mode;
}

void MouseCapture::OnFocused(const bool isFocused)
{
    if (context->focused != isFocused)
    {
        context->focusChanged = true;
        if (context->firstEntered)
        {
            context->firstEntered = false;
            context->focusChanged = false;
        }

        context->focused = isFocused;
        if (eMouseCaptureMode::PINING == context->mode)
        {
            if (context->focused)
            {
                context->deferredCapture = true;
            }
            else
            {
                SetNativePining(eMouseCaptureMode::OFF, context->nativeMode, privateImpl);
            }
        }
    }
}

bool MouseCapture::SkipEvents(const UIEvent* const event)
{
    context->focusChanged = false;
    if (eMouseCaptureMode::PINING == context->mode && context->focused && !context->deferredCapture)
    {
        privateImpl->SetCursorInCenter();
    }
    if (!context->deferredCapture && context->focused)
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
            context->deferredCapture = false;
            SetNativePining(eMouseCaptureMode::PINING, context->nativeMode, privateImpl);
        }
    }
    return true;
}

} // namespace DAVA

#else //  __DAVAENGINE_IPHONE__ || __DAVAENGINE_ANDROID__

namespace DAVA
{
MouseCapture::MouseCapture()
{
}

void MouseCapture::SetMode(const eMouseCaptureMode& newMode)
{
}

const eMouseCaptureMode& MouseCapture::GetMode() const
{
    eMouseCaptureMode res;
    return eMouseCaptureMode::OFF;
}

void MouseCapture::OnFocused(const bool isFocused)
{
}

bool MouseCapture::SkipEvents(const UIEvent* const event)
{
    return false;
}

} //namespace DAVA

#endif // defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_WIN_UAP__)
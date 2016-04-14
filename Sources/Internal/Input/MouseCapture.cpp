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

#include "Platform/TemplateMacOS/MouseCaptureMacOS.h"
#include "Platform/TemplateWin32/MouseCaptureWin32.h"
#include "Platform/TemplateWin32/MouseCaptureWinUAP.h"

#include "Core/Core.h"
#include "Input/InputSystem.h"

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
#include "Input/MouseCapture.h"

namespace DAVA
{
class MouseCapturePrivate : public MouseCaptureInterface
{
public:
    void SetNativePining(eMouseCaptureMode newMode) override
    {
    }
    void SetCursorInCenter() override
    {
    }
    bool SkipEvents() override
    {
        return false;
    }
};

} //  DAVA
#endif // defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)

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

void MouseCapture::SetNativePining(eMouseCaptureMode newNativeMode)
{
    if (newNativeMode != context->nativeMode)
    {
        context->nativeMode = newNativeMode;
        privateImpl->SetNativePining(context->nativeMode);
    }
}

MouseCapture::MouseCapture()
{
    auto focusChanged = [this](bool isFocused) -> void
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
                    SetNativePining(eMouseCaptureMode::OFF);
                }
            }
        }
    };
    context = new MouseCaptureContext();
    privateImpl = new MouseCapturePrivate();
    Core::Instance()->focusChanged.Connect(focusChanged);
    context->focused = Core::Instance()->IsFocused();
}

MouseCapture::~MouseCapture()
{
    delete context;
    delete privateImpl;
}

void MouseCapture::SetMode(const eMouseCaptureMode newMode)
{
    if (context->mode != newMode)
    {
        context->mode = newMode;
        if (eMouseCaptureMode::OFF == context->mode)
        {
            SetNativePining(context->mode);
            context->deferredCapture = false;
        }
        if (eMouseCaptureMode::PINING == context->mode)
        {
            if (context->focused && !context->focusChanged)
            {
                SetNativePining(context->mode);
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

bool MouseCapture::IsPinningEnabled() const
{
    return eMouseCaptureMode::PINING == context->nativeMode;
}

bool MouseCapture::SkipEvents(const UIEvent* event)
{
    context->focusChanged = false;
    if (privateImpl->SkipEvents())
    {
        return true;
    }
    if (IsPinningEnabled())
    {
        privateImpl->SetCursorInCenter();
    }
    if (context->deferredCapture)
    {
        if (event->phase == UIEvent::Phase::ENDED)
        {
            bool inRect = true;
            Vector2 windowSize = Core::Instance()->GetWindowSize();
            inRect &= (event->point.x >= 0.f && event->point.x <= windowSize.x);
            inRect &= (event->point.y >= 0.f && event->point.y <= windowSize.y);
            if (inRect && context->focused)
            {
                SetNativePining(eMouseCaptureMode::PINING);
                context->deferredCapture = false;
            }
        }
        return true;
    }
    return false;
}

} // namespace DAVA

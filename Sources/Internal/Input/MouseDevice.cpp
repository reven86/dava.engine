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

#include "Core/Core.h"
#include "Input/MouseDevice.h"
#include "UI/UIEvent.h"

#include "Platform/TemplateMacOS/MouseDeviceMacOS.h"
#include "Platform/TemplateWin32/MouseDeviceWin32.h"
#include "Platform/TemplateWin32/MouseDeviceWinUAP.h"

#include "Engine/Engine.h"

namespace DAVA
{
class MouseDeviceStub : public MouseDeviceInterface
{
public:
    void SetMode(eCaptureMode newMode) override
    {
    }
    void SetCursorInCenter() override
    {
    }
    bool SkipEvents(const UIEvent* event) override
    {
        return false;
    }
};

struct MouseDeviceContext
{
    eCaptureMode mode = eCaptureMode::OFF;
    eCaptureMode nativeMode = eCaptureMode::OFF;
    bool focused = false;
    bool focusChanged = false;
    bool firstEntered = true;
    bool deferredCapture = false;
};

MouseDevice::MouseDevice()
{
#if defined(__DAVAENGINE_MACOS__)
    privateImpl = new MouseDeviceMacOS();
#elif defined(__DAVAENGINE_WIN_UAP__)
    privateImpl = new MouseDeviceUWP();
#elif defined(__DAVAENGINE_WIN32__)
    privateImpl = new MouseDeviceWin32();
#elif defined(__DAVAENGINE_IPHONE__)
    privateImpl = new MouseDeviceStub();
#elif defined(__DAVAENGINE_ANDROID__)
    privateImpl = new MouseDeviceStub();
#endif
    context = new MouseDeviceContext();

#if defined(__DAVAENGINE_COREV2__)
    auto focusChanged = [this](Window*, bool isFocused) -> void
#else
    auto focusChanged = [this](bool isFocused) -> void
#endif
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
            if (eCaptureMode::PINING == context->mode)
            {
                if (context->focused)
                {
                    context->deferredCapture = true;
                }
                else
                {
                    SetSystemMode(eCaptureMode::OFF);
                }
            }
        }
    };
#if defined(__DAVAENGINE_COREV2__)
    Window* primaryWindow = Engine::Instance()->PrimaryWindow();
    primaryWindow->signalFocusChanged.Connect(focusChanged);
    context->focused = primaryWindow->HasFocus();
#else
    Core::Instance()->focusChanged.Connect(focusChanged);
    context->focused = Core::Instance()->IsFocused();
#endif
}

MouseDevice::~MouseDevice()
{
    delete context;
    delete privateImpl;
}

void MouseDevice::SetMode(eCaptureMode newMode)
{
    if (context->mode != newMode)
    {
        context->mode = newMode;
        if (eCaptureMode::OFF == context->mode)
        {
            SetSystemMode(context->mode);
            context->deferredCapture = false;
        }
        if (eCaptureMode::PINING == context->mode)
        {
            if (context->focused && !context->focusChanged)
            {
                SetSystemMode(context->mode);
            }
            else
            {
                context->deferredCapture = true;
            }
        }
    }
}

eCaptureMode MouseDevice::GetMode() const
{
    return context->mode;
}

bool MouseDevice::IsPinningEnabled() const
{
    return eCaptureMode::PINING == context->nativeMode;
}

bool MouseDevice::SkipEvents(const UIEvent* event)
{
    context->focusChanged = false;
    if (privateImpl->SkipEvents(event))
    {
        return true;
    }
    if (IsPinningEnabled())
    {
        privateImpl->SetCursorInCenter();
    }
    if (context->deferredCapture)
    {
        if (event->device != UIEvent::Device::MOUSE && context->focused)
        {
            SetSystemMode(eCaptureMode::PINING);
            context->deferredCapture = false;
            return false;
        }
        else if ((event->device == UIEvent::Device::MOUSE) && (event->phase == UIEvent::Phase::ENDED))
        {
            bool inRect = true;
#if defined(__DAVAENGINE_COREV2__)
            Vector2 windowSize;
            windowSize.dx = Engine::Instance()->PrimaryWindow()->Width();
            windowSize.dy = Engine::Instance()->PrimaryWindow()->Height();
#else
            Vector2 windowSize = Core::Instance()->GetWindowSize();
#endif
            inRect &= (event->point.x >= 0.f && event->point.x <= windowSize.x);
            inRect &= (event->point.y >= 0.f && event->point.y <= windowSize.y);
            if (inRect && context->focused)
            {
                SetSystemMode(eCaptureMode::PINING);
                context->deferredCapture = false;
            }
        }
        return true;
    }
    return false;
}

void MouseDevice::SetSystemMode(eCaptureMode sysMode)
{
    if (sysMode != context->nativeMode)
    {
        context->nativeMode = sysMode;
        privateImpl->SetMode(context->nativeMode);
    }
}

} // namespace DAVA

#include "Core/Core.h"
#include "Input/MouseDevice.h"
#include "UI/UIEvent.h"

#include "Platform/TemplateMacOS/MouseDeviceMacOS.h"
#include "Platform/TemplateWin32/MouseDeviceWin32.h"
#include "Platform/TemplateWin32/MouseDeviceWinUAP.h"

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
    Core::Instance()->focusChanged.Connect(focusChanged);
    context->focused = Core::Instance()->IsFocused();
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
            Vector2 windowSize = Core::Instance()->GetWindowSize();
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

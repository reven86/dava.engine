#include "Input/MouseCapture.h"

#include "Base/Platform.h"

class MouseCapturePrivate
{
public:
    void SetMouseCaptureMode(DAVA::InputSystem::eMouseCaptureMode newMode);
    DAVA::InputSystem::eMouseCaptureMode GetMouseCaptureMode();
    void SetApplicationFocus(bool isFocused);

    bool SkipEvents(DAVA::UIEvent* event);

private:
    void NativePining(DAVA::InputSystem::eMouseCaptureMode newMode);

    DAVA::InputSystem::eMouseCaptureMode mode = DAVA::InputSystem::eMouseCaptureMode::OFF;
    bool modeChanged = false;
    bool focused = false;
    bool focusChanged = false;
    bool deferredCapture = false;

    std::chrono::high_resolution_clock timer;
    DAVA::float32 timeStart = 0.f;
    DAVA::float32 timeStop = 0.f;
};

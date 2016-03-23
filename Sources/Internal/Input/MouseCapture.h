#include "Input/InputSystem.h"

class MouseCapturePrivate;

class MouseCapture
{
public:
    static void SetMouseCaptureMode(DAVA::InputSystem::eMouseCaptureMode mode);
    static DAVA::InputSystem::eMouseCaptureMode GetMouseCaptureMode();
    static void SetApplicationFocus(bool isFocused);

    static bool SkipEvents(DAVA::UIEvent* event);

private:
    static MouseCapturePrivate* GetPrivateImpl();
};

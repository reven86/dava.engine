#include "Input/InputSystem.h"

class MouseCapturePrivate;

class MouseCapture
{
public:
    static void SetMouseCaptureMode(DAVA::InputSystem::eMouseCaptureMode newMode);
    static DAVA::InputSystem::eMouseCaptureMode GetMouseCaptureMode();
    static void SetApplicationFocus(bool isFocused);

    static bool SkipEvents(DAVA::UIEvent* event);

private:
    static void SetNativePining(DAVA::InputSystem::eMouseCaptureMode newativeMode);

    static MouseCapturePrivate* GetPrivateImpl();

    static DAVA::InputSystem::eMouseCaptureMode mode;
    static DAVA::InputSystem::eMouseCaptureMode nativeMode;
    static bool focused;
    static bool deferredCapture;
};

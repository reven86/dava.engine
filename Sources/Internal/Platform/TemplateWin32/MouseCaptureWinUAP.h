#include "Input/MouseCapture.h"

#include "Base/Platform.h"

class MouseCapturePrivate
{
public:
    void SetNativePining(DAVA::InputSystem::eMouseCaptureMode newMode);
    void SetCursorPosition();
};

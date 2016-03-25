#include "Input/MouseCapture.h"

#include "Base/Platform.h"

class MouseCapturePrivate
{
public:
    void SetNativePining(DAVA::InputSystem::eMouseCaptureMode newMode);
    void SetCursorPosition();

private:
    bool SetSystemCursorVisibility(bool show);
    bool SetMouseCaptureMode(DAVA::InputSystem::eMouseCaptureMode mode);

    bool lastSystemCursorShowState = true;
    DAVA::Point2i lastCursorPosition;
};

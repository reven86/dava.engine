#ifndef __FRAMEWORK__MOUSECAPTUREMACOS_H__
#define __FRAMEWORK__MOUSECAPTUREMACOS_H__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_MACOS__)

#include "Input/MouseDevice.h"

#if !defined(__DAVAENGINE_COREV2__)

namespace DAVA
{
class MouseDeviceMacOS : public MouseDeviceInterface
{
public:
    MouseDeviceMacOS();
    ~MouseDeviceMacOS();

    void SetMode(eCaptureMode newMode) override;
    void SetCursorInCenter() override;
    bool SkipEvents(const UIEvent* event) override;

private:
    bool cursorVisible = true;
    void* blankCursor = nullptr;

    // If mouse pointer was outside window rectangle when enabling pinning mode then
    // mouse clicks are forwarded to other windows and our application loses focus.
    // So move mouse pointer to window center before enabling pinning mode.
    // Secondly, after using CGWarpMouseCursorPosition function to center mouse pointer
    // mouse move events arrive with big delta which causes mouse hopping.
    // The best solution I have investigated is to skip first N mouse move events after enabling
    // pinning mode: global variable skipMouseMoveEvents is set to some reasonable value
    // and is checked in OpenGLView's process method to skip mouse move events
    uint32 skipMouseMoveEvents = 0;
    const uint32 SKIP_N_MOUSE_MOVE_EVENTS = 4;

    void MovePointerToWindowCenter();
    void OSXShowCursor();
    void OSXHideCursor();
    void* GetOrCreateBlankCursor();
    
#if defined(__DAVAENGINE_STEAM__)
    Token steamOverlayActivationConnId;
    void OnSteamActivation(bool active);
#endif
};

} //  namespace DAVA

#endif // !defined(__DAVAENGINE_COREV2__)

#endif //  __DAVAENGINE_MACOS__

#endif //  __FRAMEWORK__MOUSECAPTUREMACOS_H__

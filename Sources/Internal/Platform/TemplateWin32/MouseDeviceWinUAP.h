#ifndef __FRAMEWORK__MOUSECAPTUREWINUAP_H__
#define __FRAMEWORK__MOUSECAPTUREWINUAP_H__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Input/MouseDevice.h"

namespace DAVA
{
class MouseDeviceUWP : public MouseDeviceInterface
{
public:
    void SetMode(eCaptureMode newMode) override;
    void SetCursorInCenter() override;
    bool SkipEvents(const UIEvent* event) override;

private:
    uint32 skipMouseMoveEvents = 0;
    const uint32 SKIP_N_MOUSE_MOVE_EVENTS = 4;
};
}

#endif //  __DAVAENGINE_WIN_UAP__

#endif //  __FRAMEWORK__MOUSECAPTUREWINUAP_H__
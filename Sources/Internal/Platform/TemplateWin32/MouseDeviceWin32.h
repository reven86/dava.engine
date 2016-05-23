#ifndef __FRAMEWORK__MOUSECAPTUREWIN32_H__
#define __FRAMEWORK__MOUSECAPTUREWIN32_H__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN32__)

#include "Input/MouseDevice.h"
#include "Math/Math2D.h"

namespace DAVA
{
class MouseDeviceWin32 : public MouseDeviceInterface
{
public:
    void SetMode(eCaptureMode newMode) override;
    void SetCursorInCenter() override;
    bool SkipEvents(const UIEvent* event) override;

private:
    bool SetSystemCursorVisibility(bool show);

    bool lastSystemCursorShowState = true;
    Point2i lastCursorPosition;
};

} //  namespace DAVA

#endif //  __DAVAENGINE_WIN32__

#endif //  __FRAMEWORK__MOUSECAPTUREWIN32_H__
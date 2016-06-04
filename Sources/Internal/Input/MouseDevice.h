#ifndef __FRAMEWORK__MOUSECAPTURE_H__
#define __FRAMEWORK__MOUSECAPTURE_H__

#include <memory>
#include "Base/BaseObject.h"

namespace DAVA
{
class UIEvent;
struct MouseDeviceContext;
class MouseDeviceInterface;

enum class eCaptureMode
{
    OFF = 0, //!< Disable any capturing (send absolute xy)
    FRAME, //!< Capture system cursor into window rect (send absolute xy)
    PINING //!<< Capture system cursor on current position (send xy move delta)
};

class MouseDevice final : public BaseObject
{
public:
    MouseDevice();
    ~MouseDevice();
    MouseDevice(const MouseDevice&) = delete;
    MouseDevice& operator=(const MouseDevice&) = delete;

    void SetMode(eCaptureMode newMode);
    eCaptureMode GetMode() const;
    bool IsPinningEnabled() const;
    // Deprecated, only for UIControlSystem internal using
    DAVA_DEPRECATED(bool SkipEvents(const UIEvent* event));

private:
    void SetSystemMode(eCaptureMode sysMode);
    MouseDeviceContext* context;
    MouseDeviceInterface* privateImpl;
};

class MouseDeviceInterface
{
public:
    virtual void SetMode(eCaptureMode newMode) = 0;
    virtual void SetCursorInCenter() = 0;
    virtual bool SkipEvents(const UIEvent* event) = 0;
    virtual ~MouseDeviceInterface() = default;
};

} //  namespace DAVA

#endif //  __FRAMEWORK__MOUSECAPTURE_H__
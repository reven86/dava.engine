#ifndef __DAVAENGINE_CORE_MAC_OS_PLATFORM_BASE_H__
#define __DAVAENGINE_CORE_MAC_OS_PLATFORM_BASE_H__

#include "DAVAEngine.h"

namespace DAVA
{
class CoreMacOSPlatformBase : public Core
{
public:
    void GetAvailableDisplayModes(List<DisplayMode>& availableModes) override;

    // Signal is emitted when window has been miniaturized/deminiaturized or
    // when application has been hidden/unhidden.
    // Signal parameter meaning:
    //  - when true - application/window has been hidden/minimized
    //  - when false - application/window has been unhidden/restored
    Signal<bool> signalAppMinimizedRestored;
};
};

#endif // __DAVAENGINE_CORE_MAC_OS_PLATFORM_BASE_H__
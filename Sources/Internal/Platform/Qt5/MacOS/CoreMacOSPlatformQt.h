#ifndef __DAVAENGINE_CORE_MACOS_PLATFORM_QT__
#define __DAVAENGINE_CORE_MACOS_PLATFORM_QT__


#include "DAVAEngine.h"
#include "Platform/TemplateMacOS/CoreMacOSPlatformBase.h"

#if defined(__DAVAENGINE_MACOS__)

namespace DAVA
{
class CoreMacOSPlatformQt : public CoreMacOSPlatformBase
{
public:
    eScreenMode GetScreenMode() override;
    bool SetScreenMode(eScreenMode screenMode) override;
    void Quit() override;
};
};

#endif //#if defined(__DAVAENGINE_MACOS__)


#endif // __DAVAENGINE_CORE_MACOS_PLATFORM_QT__

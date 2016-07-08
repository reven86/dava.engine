#ifndef __DAVAENGINE_CORE_PLATFORM_MAC_OS_H__
#define __DAVAENGINE_CORE_PLATFORM_MAC_OS_H__

#include "DAVAEngine.h"
#include "Platform/TemplateMacOS/CoreMacOSPlatformBase.h"
#include "Functional/Signal.h"

namespace DAVA
{
class CoreMacOSPlatform : public CoreMacOSPlatformBase
{
public:
    eScreenMode GetScreenMode() override;
    bool SetScreenMode(eScreenMode screenMode) override;
    void Quit() override;

    void SetWindowMinimumSize(float32 width, float32 height) override;
    Vector2 GetWindowMinimumSize() const override;
    void SetScreenScaleMultiplier(float32 multiplier) override;

private:
    float32 minWindowWidth = 0.0f;
    float32 minWindowHeight = 0.0f;
};
};

#endif // __DAVAENGINE_CORE_PLATFORM_MAC_OS_H__
#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
struct IGame
{
    virtual void OnGameLoopStarted() = 0;
    virtual void OnGameLoopStopped() = 0;

    virtual void OnUpdate(float32 timeElapsed) = 0;
};

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__

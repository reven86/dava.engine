#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
class NativeService final
{
public:
private:
    NativeService(Private::PlatformCore* c);

private:
    Private::PlatformCore* core = nullptr;

    // Friends
    friend Private::PlatformCore;
};

} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__

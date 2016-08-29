#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
class WindowNativeService final
{
public:
private:
    WindowNativeService(Private::WindowBackend* wbackend);

private:
    Private::WindowBackend* windowBackend = nullptr;

    // friends
    friend class Private::WindowBackend;
};

} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__

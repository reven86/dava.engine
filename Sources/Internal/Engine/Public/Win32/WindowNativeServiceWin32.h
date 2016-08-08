#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#include "Engine/Private/EnginePrivateFwd.h"

namespace rhi
{
struct InitParam;
}

namespace DAVA
{
class WindowNativeService final
{
public:
    void InitRenderParams(rhi::InitParam& params);

private:
    WindowNativeService(Private::WindowBackend* wbackend);

private:
    Private::WindowBackend* windowBackend = nullptr;

    // friends
    friend class Private::WindowBackend;
};

} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__

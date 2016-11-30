#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
class RenderWidget;

class WindowNativeService final
{
public:
    void AcqureContext();
    void ReleaseContext();

private:
    WindowNativeService(Private::WindowBackend* wbackend);

private:
    Private::WindowBackend* windowBackend = nullptr;

    // friends
    friend class Private::WindowBackend;
};

} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
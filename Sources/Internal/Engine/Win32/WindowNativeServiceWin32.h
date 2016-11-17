#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"
#include "Base/Platform.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
class WindowNativeService final
{
private:
    WindowNativeService(Private::WindowBackend* windowBackend);

public:
    HWND GetHWND() const;

private:
    Private::WindowBackend* windowBackend = nullptr;

    // friends
    friend class Private::WindowBackend;
};

} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__

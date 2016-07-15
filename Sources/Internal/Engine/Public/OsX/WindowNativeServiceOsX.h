#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/EnginePrivateFwd.h"

@class NSView;

namespace DAVA
{
class WindowNativeService final
{
public:
    void AddNSView(NSView* nsview);
    void RemoveNSView(NSView* nsview);

private:
    WindowNativeService(Private::WindowNativeBridge* nativeBridge);

private:
    Private::WindowNativeBridge* bridge = nullptr;

    // Friends
    friend class Private::WindowBackend;
};

} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__

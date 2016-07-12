#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/WindowNativeBridgeiOS.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#include "Engine/Public/Window.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/iOS/WindowBackendiOS.h"
#include "Engine/Private/iOS/WindowDelegateiOS.h"

#include "Platform/SystemTimer.h"
#include "Logger/Logger.h"

namespace DAVA
{
namespace Private
{
WindowNativeBridgeiOS::WindowNativeBridgeiOS(WindowBackend* wbackend)
    : windowBackend(wbackend)
{
}

WindowNativeBridgeiOS::~WindowNativeBridgeiOS() = default;

bool WindowNativeBridgeiOS::DoCreateWindow(float32 x, float32 y, float32 width, float32 height)
{
    return false;
}

void WindowNativeBridgeiOS::DoResizeWindow(float32 width, float32 height)
{
}

void WindowNativeBridgeiOS::DoCloseWindow()
{
}

void WindowNativeBridgeiOS::TriggerPlatformEvents()
{
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__

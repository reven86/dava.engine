#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/WindowOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include <AppKit/NSScreen.h>

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Private/Dispatcher/Dispatcher.h"
#include "Engine/Private/OsX/CoreOsX.h"
#include "Engine/Private/OsX/WindowOsXObjcBridge.h"

#include "Concurrency/LockGuard.h"
#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
WindowOsX::WindowOsX(EngineBackend* engine_, WindowBackend* window_)
    : engine(engine_)
    , dispatcher(engine->dispatcher)
    , window(window_)
    , bridge(new WindowOsXObjcBridge(this))
{
    hideUnhideSignalId = engine->platformCore->didHideUnhide.Connect(bridge, &WindowOsXObjcBridge::ApplicationDidHideUnhide);
}

WindowOsX::~WindowOsX()
{
    engine->platformCore->didHideUnhide.Disconnect(hideUnhideSignalId);
    delete bridge;
}

void WindowOsX::Resize(float32 width, float32 height)
{
}

void* WindowOsX::GetHandle() const
{
    return bridge->openGLView;
}

void WindowOsX::RunAsyncOnUIThread(const Function<void()>& task)
{
}

bool WindowOsX::CreateWindow(float32 width, float32 height)
{
    NSSize screenSize = [[NSScreen mainScreen] frame].size;
    float32 x = (screenSize.width - width) / 2.0f;
    float32 y = (screenSize.height - height) / 2.0f;
    return bridge->CreateNSWindow(x, y, width, height);
}

void WindowOsX::DestroyWindow()
{
    bridge->DestroyNSWindow();
}

void WindowOsX::ResizeWindow(float32 width, float32 height)
{
    bridge->ResizeNSWindow(width, height);
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__

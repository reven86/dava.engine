#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/WindowOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Private/Dispatcher/Dispatcher.h"
#include "Engine/Private/OsX/CoreOsX.h"

#include "Concurrency/LockGuard.h"
#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
WindowOsX* WindowOsX::Create(Dispatcher* dispatcher, WindowBackend* window)
{
    return nullptr;
}

void WindowOsX::Destroy(WindowOsX* nativeWindow)
{
}

WindowOsX::WindowOsX(Dispatcher* dispatcher_, WindowBackend* window_)
    : dispatcher(dispatcher_)
    , window(window_)
{
}

WindowOsX::~WindowOsX() = default;

void WindowOsX::Resize(float32 width, float32 height)
{
}

void* WindowOsX::GetHandle() const
{
    return nullptr;
}

void WindowOsX::RunAsyncOnUIThread(const Function<void()>& task)
{
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__

#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Qt/WindowBackendQt.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Public/Window.h"

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/Qt/WindowBackendQt.h"

namespace DAVA
{
namespace Private
{
WindowBackend* WindowBackend::Create(Window* w)
{
    return nullptr;
}

void WindowBackend::Destroy(WindowBackend* nativeWindow)
{
}

WindowBackend::WindowBackend(Window* w)
    : dispatcher(EngineBackend::instance->dispatcher)
    , window(w)
{
}

WindowBackend::~WindowBackend() = default;

void WindowBackend::Resize(float32 width, float32 height)
{
}

void* WindowBackend::Handle() const
{
    return nullptr;
}

void WindowBackend::RunAsyncOnUIThread(const Function<void()>& task)
{
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__

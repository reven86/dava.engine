#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Qt/WindowQt.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Public/Window.h"

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/Dispatcher.h"
#include "Engine/Private/Qt/WindowQt.h"

namespace DAVA
{
namespace Private
{
WindowQt* WindowQt::Create(Window* w)
{
    return nullptr;
}

void WindowQt::Destroy(WindowQt* nativeWindow)
{
}

WindowQt::WindowQt(Window* w)
    : dispatcher(EngineBackend::instance->dispatcher)
    , window(w)
{
}

WindowQt::~WindowQt() = default;

void WindowQt::Resize(float32 width, float32 height)
{
}

void* WindowQt::Handle() const
{
    return nullptr;
}

void WindowQt::RunAsyncOnUIThread(const Function<void()>& task)
{
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__

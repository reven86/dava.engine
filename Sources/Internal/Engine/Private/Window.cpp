#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/Window.h"
#include "Engine/Private/WindowBackend.h"

namespace DAVA
{
Window::Window(Private::WindowBackend* backend)
    : windowBackend(backend)
{
}

Window::~Window()
{
    windowBackend = nullptr;
}

bool Window::IsPrimary() const
{
    return windowBackend->IsPrimary();
}

bool Window::IsVisible() const
{
    return windowBackend->IsVisible();
}

bool Window::HasFocus() const
{
    return windowBackend->HasFocus();
}

float32 Window::GetWidth() const
{
    return windowBackend->GetWidth();
}

float32 Window::GetHeight() const
{
    return windowBackend->GetHeight();
}

float32 Window::GetRenderSurfaceWidth() const
{
    return windowBackend->GetRenderSurfaceWidth();
}

float32 Window::GetRenderSurfaceHeight() const
{
    return windowBackend->GetRenderSurfaceHeight();
}

float32 Window::GetScaleX() const
{
    return windowBackend->GetScaleX();
}

float32 Window::GetScaleY() const
{
    return windowBackend->GetScaleY();
}

float32 Window::GetUserScale() const
{
    return windowBackend->GetUserScale();
}

float32 Window::GetRenderSurfaceScaleX() const
{
    return windowBackend->GetRenderSurfaceScaleX();
}

float32 Window::GetRenderSurfaceScaleY() const
{
    return windowBackend->GetRenderSurfaceScaleY();
}

void Window::Resize(float32 w, float32 h)
{
    return windowBackend->Resize(w, h);
}

void* Window::GetNativeHandle() const
{
    return windowBackend->GetNativeHandle();
}

void Window::RunAsyncOnUIThread(const Function<void()>& task)
{
    windowBackend->RunAsyncOnUIThread(task);
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__

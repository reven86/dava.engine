#if defined(__DAVAENGINE_COREV2__)

#include "Engine/UWP/WindowNativeServiceUWP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/UWP/Window/WindowNativeBridgeUWP.h"

namespace DAVA
{
WindowNativeService::WindowNativeService(Private::WindowNativeBridge ^ cxxBridge)
    : bridge(cxxBridge)
{
}

void WindowNativeService::AddXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl)
{
    bridge->AddXamlControl(xamlControl);
}

void WindowNativeService::RemoveXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl)
{
    bridge->RemoveXamlControl(xamlControl);
}

void WindowNativeService::PositionXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl, float32 x, float32 y)
{
    bridge->PositionXamlControl(xamlControl, x, y);
}

void WindowNativeService::UnfocusXamlControl()
{
    bridge->UnfocusXamlControl();
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__

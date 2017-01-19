#include "Base/Platform.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/UWP/PlatformCoreUWP.h"
#include "Engine/Private/UWP/Window/WindowBackendUWP.h"
#include "Engine/Private/UWP/Window/WindowNativeBridgeUWP.h"

namespace DAVA
{
namespace PlatformApi
{
namespace Win10
{
void AddXamlControl(Window* targetWindow, ::Windows::UI::Xaml::UIElement ^ xamlControl)
{
    using namespace DAVA::Private;
    WindowBackend* wb = EngineBackend::GetWindowBackend(targetWindow);
    wb->bridge->AddXamlControl(xamlControl);
}

void RemoveXamlControl(Window* targetWindow, ::Windows::UI::Xaml::UIElement ^ xamlControl)
{
    using namespace DAVA::Private;
    WindowBackend* wb = EngineBackend::GetWindowBackend(targetWindow);
    wb->bridge->RemoveXamlControl(xamlControl);
}

void PositionXamlControl(Window* targetWindow, ::Windows::UI::Xaml::UIElement ^ xamlControl, float32 x, float32 y)
{
    using namespace DAVA::Private;
    WindowBackend* wb = EngineBackend::GetWindowBackend(targetWindow);
    wb->bridge->PositionXamlControl(xamlControl, x, y);
}

void UnfocusXamlControl(Window* targetWindow, ::Windows::UI::Xaml::UIElement ^ xamlControl)
{
    using namespace DAVA::Private;
    WindowBackend* wb = EngineBackend::GetWindowBackend(targetWindow);
    wb->bridge->UnfocusXamlControl();
}

::Windows::UI::Xaml::Input::Pointer ^ GetLastPressedPointer(Window* targetWindow)
{
    using namespace DAVA::Private;
    WindowBackend* wb = EngineBackend::GetWindowBackend(targetWindow);
    return wb->bridge->GetLastPressedPointer();
}

void RegisterXamlApplicationListener(XamlApplicationListener* listener)
{
    using namespace DAVA::Private;
    PlatformCore* core = EngineBackend::Instance()->GetPlatformCore();
    core->RegisterXamlApplicationListener(listener);
}

void UnregisterXamlApplicationListener(XamlApplicationListener* listener)
{
    using namespace DAVA::Private;
    PlatformCore* core = EngineBackend::Instance()->GetPlatformCore();
    core->UnregisterXamlApplicationListener(listener);
}

} // namespace Win10
} // namespace PlatformApi
} // namespace DAVA

#endif // defined(__DAVAENGINE_WIN_UAP__)
#endif // defined(__DAVAENGINE_COREV2__)

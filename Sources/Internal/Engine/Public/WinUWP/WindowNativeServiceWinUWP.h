#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineFwd.h"

namespace DAVA
{
class WindowNativeService final
{
public:
    void AddXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl);
    void RemoveXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl);
    void PositionXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl, float32 x, float32 y);
    void UnfocusXamlControl();

private:
    WindowNativeService(Private::WindowWinUWPBridge ^ cxxBridge);

private:
    Private::WindowWinUWPBridge ^ bridge = nullptr;

    // Friends
    friend Private::WindowWinUWP;
};

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__

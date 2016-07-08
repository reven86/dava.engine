#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineFwd.h"
#include "Engine/Private/Dispatcher/PlatformDispatcher.h"

#include "Functional/Function.h"

namespace DAVA
{
namespace Private
{
class WindowWinUWP final
{
public:
    WindowWinUWP(EngineBackend* e, Window* w);
    ~WindowWinUWP();

    void* GetHandle() const;
    Dispatcher* GetDispatcher() const;
    Window* GetWindow() const;

    void Resize(float32 width, float32 height);
    void Close();

    void RunAsyncOnUIThread(const Function<void()>& task);

    void TriggerPlatformEvents();
    void ProcessPlatformEvents();

    void BindXamlWindow(::Windows::UI::Xaml::Window ^ xamlWindow);

    void AddXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl);
    void RemoveXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl);
    void PositionXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl, float32 x, float32 y);
    void UnfocusXamlControl();
    //void SetTextBoxCustomStyle(Windows::UI::Xaml::Controls::TextBox^ textBox);
    //void SetPasswordBoxCustomStyle(Windows::UI::Xaml::Controls::PasswordBox^ passwordBox);
    //void CaptureTextBox(Windows::UI::Xaml::Controls::Control^ text);

private:
    void PlatformEventHandler(const PlatformEvent& e);

private:
    EngineBackend* engine = nullptr;
    Dispatcher* dispatcher = nullptr;
    Window* window = nullptr;

    PlatformDispatcher platformDispatcher;

    ref struct WindowWinUWPBridge ^ bridge = nullptr;
};

inline Dispatcher* WindowWinUWP::GetDispatcher() const
{
    return dispatcher;
}

inline Window* WindowWinUWP::GetWindow() const
{
    return window;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__

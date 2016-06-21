#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineFwd.h"

#include "Functional/Function.h"

namespace DAVA
{
namespace Private
{
// clang-format off

class WindowWinUWP final
{
public:
    WindowWinUWP(EngineBackend* engine_, WindowBackend* window_);
    ~WindowWinUWP();

    void Resize(float32 width, float32 height);
    void* GetHandle() const;

    void RunAsyncOnUIThread(const Function<void()>& task);

private:
    void BindXamlWindow(::Windows::UI::Xaml::Window^ xamlWindow);
    void DestroyNWindow();

private:
    EngineBackend* engine = nullptr;
    Dispatcher* dispatcher = nullptr;
    WindowBackend* window = nullptr;

    ref struct WindowWinUWPCxxBridge^ bridge = nullptr;

    // Friends
    friend class CoreWinUWP;
    friend ref struct WindowWinUWPCxxBridge;
};

// clang-format on

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__

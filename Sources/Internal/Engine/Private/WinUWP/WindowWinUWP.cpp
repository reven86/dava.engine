#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/WinUWP/WindowWinUWP.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Private/Dispatcher/Dispatcher.h"
#include "Engine/Private/WinUWP/CoreWinUWP.h"
#include "Engine/Private/WinUWP/WindowWinUWPCxxBridge.h"

namespace DAVA
{
namespace Private
{
// clang-format off

WindowWinUWP::WindowWinUWP(EngineBackend* engine_, WindowBackend* window_)
    : engine(engine_)
    , dispatcher(engine->dispatcher)
    , window(window_)
    , bridge(ref new WindowWinUWPCxxBridge(this))
{

}

WindowWinUWP::~WindowWinUWP() = default;

void WindowWinUWP::Resize(float32 width, float32 height)
{
}

void* WindowWinUWP::GetHandle() const
{
    return reinterpret_cast<void*>(bridge->xamlSwapChainPanel);
}

void WindowWinUWP::RunAsyncOnUIThread(const Function<void()>& task)
{
}

void WindowWinUWP::BindXamlWindow(::Windows::UI::Xaml::Window^ xamlWindow)
{
    bridge->BindToXamlWindow(xamlWindow);
}

void WindowWinUWP::DestroyNWindow()
{

}

// clang-format on

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__

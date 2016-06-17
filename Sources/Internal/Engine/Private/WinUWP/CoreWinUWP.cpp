#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/WinUWP/CoreWinUWP.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Private/WinUWP/WindowWinUWP.h"

namespace DAVA
{
namespace Private
{
CoreWinUWP::CoreWinUWP(EngineBackend* e)
    : engineBackend(e)
{
}

CoreWinUWP::~CoreWinUWP() = default;

Vector<String> CoreWinUWP::GetCommandLine(int argc, char* argv[])
{
    return Vector<String>();
}

void CoreWinUWP::Init()
{
}

void CoreWinUWP::Run()
{
}

void CoreWinUWP::Quit()
{
}

WindowWinUWP* CoreWinUWP::CreateNativeWindow(WindowBackend* w, float32 width, float32 height)
{
    return nullptr;
}

void CoreWinUWP::DestroyNativeWindow(WindowBackend* w)
{
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__

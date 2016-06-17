#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineFwd.h"

namespace DAVA
{
namespace Private
{
class CoreWinUWP final
{
public:
    CoreWinUWP(EngineBackend* e);
    ~CoreWinUWP();

    Vector<String> GetCommandLine(int argc, char* argv[]);

    void Init();
    void Run();
    void Quit();

    WindowWinUWP* CreateNativeWindow(WindowBackend* w, float32 width, float32 height);
    void DestroyNativeWindow(WindowBackend* w);

private:
    EngineBackend* engineBackend = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__

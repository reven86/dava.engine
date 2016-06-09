#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/EngineFwd.h"

namespace DAVA
{
namespace Private
{
class CoreOsX final
{
public:
    CoreOsX();
    ~CoreOsX();

    CoreOsX(const CoreOsX&) = delete;
    CoreOsX& operator=(const CoreOsX&) = delete;

    Vector<String> GetCommandLine(int argc, char* argv[]);

    void Init();
    void Run();
    void Quit();

    WindowOsX* CreateNativeWindow(WindowBackend* w);
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__

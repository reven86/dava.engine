#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Functional/Signal.h"

#include "Engine/Private/EngineFwd.h"
#include "Engine/Private/OsX/OsXFwd.h"

namespace DAVA
{
namespace Private
{
class CoreOsX final
{
public:
    CoreOsX(EngineBackend* e);
    ~CoreOsX();

    Vector<String> GetCommandLine(int argc, char* argv[]);

    void Init();
    void Run();
    void Quit();

    // WindowOsX gets notified about application hidden/unhidden state changing
    // to update its visibility state
    Signal<bool> didHideUnhide;

private:
    int OnFrame();

    WindowOsX* CreateNativeWindow(WindowBackend* w, float32 width, float32 height);

private:
    EngineBackend* engineBackend = nullptr;
    // TODO: std::unique_ptr
    CoreOsXObjcBridge* objcBridge = nullptr;

    // Friends
    friend struct CoreOsXObjcBridge;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__

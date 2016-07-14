#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/PlatformCoreiOS.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#include "Engine/Public/iOS/NativeServiceiOS.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/iOS/Window/WindowBackendiOS.h"
#include "Engine/Private/iOS/CoreNativeBridgeiOS.h"

namespace DAVA
{
namespace Private
{
PlatformCore::PlatformCore(EngineBackend* e)
    : engineBackend(e)
    , dispatcher(e->GetDispatcher())
    , bridge(new CoreNativeBridgeiOS(this))
    , nativeService(new NativeService(this))
{
}

PlatformCore::~PlatformCore()
{
    delete bridge;
}

void PlatformCore::Init()
{
}

void PlatformCore::Run()
{
    bridge->Run();
}

void PlatformCore::Quit()
{
    // Quit is not supported on iOS
}

int32 PlatformCore::OnFrame()
{
    return engineBackend->OnFrame();
}

WindowBackend* PlatformCore::CreateNativeWindow(Window* w, float32 width, float32 height)
{
    WindowBackend* wbackend = new WindowBackend(engineBackend, w);
    if (!wbackend->Create(width, height))
    {
        delete wbackend;
        wbackend = nullptr;
    }
    return wbackend;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__

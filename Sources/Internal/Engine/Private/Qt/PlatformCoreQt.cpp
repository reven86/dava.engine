#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Qt/PlatformCoreQt.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Private/EngineBackend.h"

namespace DAVA
{
namespace Private
{
PlatformCore::PlatformCore() = default;
PlatformCore::~PlatformCore() = default;

void PlatformCore::Init()
{
}

void PlatformCore::Run()
{
}

void PlatformCore::Quit()
{
}

WindowBackend* PlatformCore::CreateNativeWindow(Window* w)
{
    return WindowBackend::Create(w);
}

void (*PlatformCore::AcqureContext())()
{
    return acqureContext;
}

void (*PlatformCore::ReleaseContext())()
{
    return releaseContext;
}

void PlatformCore::Prepare(void (*acqureContextFunc)(), void (*releaseContextFunc)())
{
    acqureContext = acqureContextFunc;
    releaseContext = releaseContextFunc;

    EngineBackend::instance->OnGameLoopStarted();
}

void PlatformCore::OnFrame()
{
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__

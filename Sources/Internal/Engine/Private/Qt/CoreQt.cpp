#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Qt/CoreQt.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Private/EngineBackend.h"

namespace DAVA
{
namespace Private
{
CoreQt::CoreQt() = default;
CoreQt::~CoreQt() = default;

void CoreQt::Init()
{
}

void CoreQt::Run()
{
}

void CoreQt::Quit()
{
}

WindowQt* CoreQt::CreateNativeWindow(Window* w)
{
    return WindowQt::Create(w);
}

void (*CoreQt::AcqureContext())()
{
    return acqureContext;
}

void (*CoreQt::ReleaseContext())()
{
    return releaseContext;
}

void CoreQt::Prepare(void (*acqureContextFunc)(), void (*releaseContextFunc)())
{
    acqureContext = acqureContextFunc;
    releaseContext = releaseContextFunc;

    EngineBackend::instance->OnGameLoopStarted();
}

void CoreQt::OnFrame()
{
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__

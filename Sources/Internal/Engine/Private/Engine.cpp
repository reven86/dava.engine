#include "Engine/Engine.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"

namespace DAVA
{
namespace EngineSingletonNamespace
{
Engine* engineSingleton = nullptr;
}

Engine* Engine::Instance()
{
    return EngineSingletonNamespace::engineSingleton;
}

Engine::Engine()
{
    DVASSERT(EngineSingletonNamespace::engineSingleton == nullptr);

    EngineSingletonNamespace::engineSingleton = this;

    engineBackend = Private::EngineBackend::Instance();
    engineBackend->EngineCreated(this);
}

Engine::~Engine()
{
    engineBackend->EngineDestroyed();
    engineBackend = nullptr;

    EngineSingletonNamespace::engineSingleton = nullptr;
}

const EngineContext* Engine::GetContext() const
{
    return engineBackend->GetContext();
}

Window* Engine::PrimaryWindow() const
{
    return engineBackend->GetPrimaryWindow();
}

eEngineRunMode Engine::GetRunMode() const
{
    return engineBackend->GetRunMode();
}

bool Engine::IsStandaloneGUIMode() const
{
    return engineBackend->IsStandaloneGUIMode();
}

bool Engine::IsEmbeddedGUIMode() const
{
    return engineBackend->IsEmbeddedGUIMode();
}

bool Engine::IsConsoleMode() const
{
    return engineBackend->IsConsoleMode();
}

void Engine::Init(eEngineRunMode runMode, const Vector<String>& modules, KeyedArchive* options)
{
    engineBackend->Init(runMode, modules, options);
}

int Engine::Run()
{
    return engineBackend->Run();
}

void Engine::QuitAsync(int exitCode)
{
    engineBackend->Quit(exitCode);
}

void Engine::SetCloseRequestHandler(const Function<bool(Window*)>& handler)
{
    engineBackend->SetCloseRequestHandler(handler);
}

void Engine::RunOnMainThreadAsync(const Function<void()>& task)
{
    engineBackend->DispatchOnMainThread(task, false);
}

void Engine::RunOnMainThread(const Function<void()>& task)
{
    engineBackend->DispatchOnMainThread(task, true);
}

uint32 Engine::GetGlobalFrameIndex() const
{
    return engineBackend->GetGlobalFrameIndex();
}

const Vector<String>& Engine::GetCommandLine() const
{
    return engineBackend->GetCommandLine();
}

Vector<char*> Engine::GetCommandLineAsArgv() const
{
    return engineBackend->GetCommandLineAsArgv();
}

const KeyedArchive* Engine::GetOptions() const
{
    return engineBackend->GetOptions();
}

bool Engine::IsSuspended() const
{
    return engineBackend->IsSuspended();
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__

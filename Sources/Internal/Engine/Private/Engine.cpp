#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Engine.h"

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

EngineContext* Engine::GetContext() const
{
    return engineBackend->GetEngineContext();
}

NativeService* Engine::GetNativeService() const
{
    return engineBackend->GetNativeService();
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

void Engine::Init(eEngineRunMode runMode, const Vector<String>& modules)
{
    engineBackend->Init(runMode, modules);
}

int Engine::Run()
{
    return engineBackend->Run();
}

void Engine::Quit(int exitCode)
{
    engineBackend->Quit(exitCode);
}

void Engine::SetCloseRequestHandler(const Function<bool(Window*)>& handler)
{
    engineBackend->SetCloseRequestHandler(handler);
}

void Engine::RunAsyncOnMainThread(const Function<void()>& task)
{
    engineBackend->DispatchOnMainThread(task, false);
}

void Engine::RunAndWaitOnMainThread(const Function<void()>& task)
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

void Engine::SetOptions(KeyedArchive* options)
{
    engineBackend->SetOptions(options);
}

KeyedArchive* Engine::GetOptions()
{
    return engineBackend->GetOptions();
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__

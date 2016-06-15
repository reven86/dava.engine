#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/Engine.h"

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Private/Dispatcher/Dispatcher.h"

namespace DAVA
{
namespace
{
Engine* engineSingleton = nullptr;
}

Engine* Engine::Instance()
{
    return engineSingleton;
}

Engine::Engine()
{
    DVASSERT(engineSingleton == nullptr);

    engineSingleton = this;
    engineBackend = Private::EngineBackend::instance;
    engineBackend->engine = this;
}

Engine::~Engine()
{
    engineBackend = nullptr;
    engineSingleton = nullptr;
}

AppContext* Engine::Context() const
{
    return engineBackend->context;
}

Window* Engine::PrimaryWindow() const
{
    return engineBackend->primaryWindow->GetWindow();
}

void Engine::Init(bool consoleMode, const Vector<String>& modules)
{
    engineBackend->Init(consoleMode, modules);
}

int Engine::Run()
{
    return engineBackend->Run();
}

void Engine::Quit(int exitCode)
{
    engineBackend->Quit(exitCode);
}

void Engine::RunAsyncOnMainThread(const Function<void()>& task)
{
    engineBackend->RunAsyncOnMainThread(task);
}

uint32 Engine::GetGlobalFrameIndex() const
{
    return engineBackend->globalFrameIndex;
}

const Vector<String>& Engine::GetCommandLine() const
{
    return engineBackend->GetCommandLine();
}

bool Engine::IsConsoleMode() const
{
    return engineBackend->consoleMode;
}

void Engine::SetOptions(KeyedArchive* options)
{
    engineBackend->options = options;
}

KeyedArchive* Engine::GetOptions()
{
    return engineBackend->options;
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__

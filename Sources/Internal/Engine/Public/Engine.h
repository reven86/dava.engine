#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Functional.h"

#include "Engine/Public/AppContext.h"
#include "Engine/Private/EngineFwd.h"

namespace DAVA
{
class Window;

class Engine final
{
    friend class Private::EngineBackend;

public:
    // TODO: remove this method after all modules and systems will take Engine instance on creation
    static Engine* Instance();

    Engine();
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    AppContext* Context() const;
    Window* PrimaryWindow() const;

    void Init(bool consoleMode, const Vector<String>& modules);
    int Run();
    void Quit(int exitCode = 0);

    void RunAsyncOnMainThread(const Function<void()>& task);

    // Methods taken from class Core
    void SetOptions(KeyedArchive* options_);
    KeyedArchive* GetOptions();

    uint32 GetGlobalFrameIndex() const;
    const Vector<String>& GetCommandLine() const;
    bool IsConsoleMode() const;

public:
    // Signals
    Signal<> gameLoopStarted;
    Signal<> gameLoopStopped;
    Signal<> beforeTerminate;
    Signal<Window*> windowCreated;
    Signal<Window*> windowDestroyed;
    Signal<> beginFrame;
    Signal<float32> update;
    Signal<> draw;
    Signal<> endFrame;

private:
    Private::EngineBackend* engineBackend = nullptr;
};

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__

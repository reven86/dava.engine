#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Functional.h"

#include "Engine/Private/EngineFwd.h"

namespace DAVA
{
struct IGame;
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

    Window* PrimaryWindow() const;

    void Init(bool consoleMode, const Vector<String>& modules);
    int Run();
    void Quit();

    // Dummy methods for now
    uint32 GetGlobalFrameIndex() const;
    const Vector<String>& GetCommandLine() const;
    bool IsConsoleMode() const;

    void SetOptions(KeyedArchive* options_);
    KeyedArchive* GetOptions();

    void RunAsyncOnMainThread(const Function<void()>& task);

public:
    // Signals
    Signal<> signalGameLoopStarted;
    Signal<> signalGameLoopStopped;
    Signal<> signalBeginFrame;
    Signal<float32> signalPreUpdate;
    Signal<float32> signalUpdate;
    Signal<float32> signalPostUpdate;
    Signal<> signalDraw;
    Signal<> signalEndFrame;

private:
    Private::EngineBackend* engineBackend = nullptr;
    KeyedArchive* options = nullptr;
};

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__

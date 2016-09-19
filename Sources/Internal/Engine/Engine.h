#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Functional.h"

#include "Engine/EngineTypes.h"
#include "Engine/EngineContext.h"
#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
class KeyedArchive;
class Engine final
{
public:
    // TODO: remove this method after all modules and systems will take Engine instance on creation
    static Engine* Instance();

    Engine();
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    EngineContext* GetContext() const;
    NativeService* GetNativeService() const;
    Window* PrimaryWindow() const;

    eEngineRunMode GetRunMode() const;
    bool IsStandaloneGUIMode() const;
    bool IsEmbeddedGUIMode() const;
    bool IsConsoleMode() const;

    void Init(eEngineRunMode runMode, const Vector<String>& modules);
    int Run();
    void Quit(int exitCode = 0);

    /// \brief Set handler which is invoked when user is trying to close window.
    ///
    /// Handler can prevent window closing by returning false. This ability is
    /// supported only on some desktop platforms: win32 and macOS.
    /// Typical usage is to return false in handler to prevent immediate window closing
    /// and show dialog asking user whether she wants to close window. If she chooses to
    /// close window then application should call Window::Close.
    /// Handler is only invoked if window is closing by user request: by pressing Alt+F4 or
    /// by clicking mouse on window close button.
    ///
    /// \param handler - function object which takes Window& as parameter and returns bool
    void SetShouldWindowCloseHandler(const Function<bool(Window&)>& handler);
    void RunAsyncOnMainThread(const Function<void()>& task);
    void RunAndWaitOnMainThread(const Function<void()>& task);

    // Methods taken from class Core
    void SetOptions(KeyedArchive* options_);
    KeyedArchive* GetOptions();

    uint32 GetGlobalFrameIndex() const;
    const Vector<String>& GetCommandLine() const;

public:
    // Signals
    Signal<> gameLoopStarted;
    Signal<> gameLoopStopped;
    Signal<> beforeTerminate;
    Signal<Window&> windowCreated;
    Signal<Window&> windowDestroyed;
    Signal<> beginFrame;
    Signal<float32> update;
    Signal<> draw;
    Signal<> endFrame;
    Signal<> suspended;
    Signal<> resumed;

private:
    Private::EngineBackend* engineBackend = nullptr;

    // Friends
    friend class Private::EngineBackend;
};

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__

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

    /// \brief Set handler which is invoked when user is trying to close window or application.
    ///
    /// Handler can prevent window/application closing by returning false. This ability is
    /// supported only on desktop platforms: win32 and macOS.
    /// Typical usage is to return false in handler to prevent immediate window/app closing
    /// and show dialog asking user whether she wants to close window/app. If she chooses to
    /// close window/app then application should call Window::Close or Engine::Quit.
    /// Handler is only invoked if window/app is closing by user request: by pressing Alt+F4 or
    /// by clicking mouse on window close button or by pressing Cmd+Q on macOS.
    ///
    /// \param handler Function object which takes Window* as parameter and returns bool.
    ///                If pointer to Window is null then user is trying to close whole application,
    ///                otherwise user is trying to close specified window.
    void SetCloseRequestHandler(const Function<bool(Window*)>& handler);
    void RunAsyncOnMainThread(const Function<void()>& task);
    void RunAndWaitOnMainThread(const Function<void()>& task);

    // Methods taken from class Core
    void SetOptions(KeyedArchive* options);
    KeyedArchive* GetOptions();

    uint32 GetGlobalFrameIndex() const;
    const Vector<String>& GetCommandLine() const;
    DAVA::Vector<char*> GetCommandLineAsArgv();

public:
    // Signals
    Signal<> gameLoopStarted;
    Signal<> gameLoopStopped;
    Signal<> cleanup;
    Signal<Window*> windowCreated;
    Signal<Window*> windowDestroyed;
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

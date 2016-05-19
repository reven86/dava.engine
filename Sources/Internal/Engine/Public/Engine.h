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
    static Engine* Instance();

    Engine();
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    Window* PrimaryWindow() const;

    void Init(bool consoleMode, const Vector<String>& modules);
    int Run(IGame* gameObject);
    void Quit();

    // Dummy methods for now
    uint32 GetGlobalFrameIndex() const;
    const Vector<String>& GetCommandLine() const;
    bool IsConsoleMode() const;

    void SetOptions(KeyedArchive* archiveOfOptions);
    KeyedArchive* GetOptions();

    void RunAsyncOnMainThread(const Function<void()>& task);

private:
    Private::EngineBackend* engineBackend = nullptr;
    KeyedArchive* options = nullptr;
};

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__

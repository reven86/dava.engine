#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Concurrency/Mutex.h"
#include "Functional/Function.h"

#include "Engine/Private/EngineFwd.h"
#include "Engine/Private/OsX/OsXFwd.h"
#include "Engine/Private/Dispatcher/PlatformDispatcher.h"

namespace DAVA
{
namespace Private
{
class WindowOsX final
{
public:
    WindowOsX(EngineBackend* engine_, WindowBackend* window_);
    ~WindowOsX();

    void* GetHandle() const;

    bool Create(float32 width, float32 height);
    void Resize(float32 width, float32 height);
    void Close();

    void RunAsyncOnUIThread(const Function<void()>& task);

    void TriggerPlatformEvents();
    void ProcessPlatformEvents();

private:
    void EventHandler(const PlatformEvent& e);

private:
    EngineBackend* engine = nullptr;
    Dispatcher* dispatcher = nullptr;
    WindowBackend* window = nullptr;

    PlatformDispatcher platformDispatcher;

    WindowOsXObjcBridge* bridge = nullptr;

    bool isMinimized = false;
    size_t hideUnhideSignalId = 0;

    // Friends
    friend class CoreOsX;
    friend struct WindowOsXObjcBridge;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__

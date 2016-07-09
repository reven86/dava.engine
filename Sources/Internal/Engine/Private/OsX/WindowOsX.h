#pragma once

#if defined(__DAVAENGINE_COREV2__)

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
class WindowInteropService;
class WindowOsX final
{
public:
    WindowOsX(EngineBackend* e, Window* w);
    ~WindowOsX();

    void* GetHandle() const;
    Dispatcher* GetDispatcher() const;
    Window* GetWindow() const;

    WindowInteropService* GetInteropService() const;

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
    Window* window = nullptr;

    PlatformDispatcher platformDispatcher;

    WindowOsXObjcBridge* bridge = nullptr;

    bool isMinimized = false;
    size_t hideUnhideSignalId = 0;

    // Friends
    friend class CoreOsX;
    friend struct WindowOsXObjcBridge;
};

inline Dispatcher* WindowOsX::GetDispatcher() const
{
    return dispatcher;
}

inline Window* WindowOsX::GetWindow() const
{
    return window;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__

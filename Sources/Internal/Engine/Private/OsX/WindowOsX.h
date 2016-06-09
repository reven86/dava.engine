#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Concurrency/Mutex.h"
#include "Functional/Function.h"

#include "Engine/Private/EngineFwd.h"

namespace DAVA
{
namespace Private
{
class WindowOsX final
{
public:
    static WindowOsX* Create(Dispatcher* dispatcher, WindowBackend* window);
    static void Destroy(WindowOsX* nativeWindow);

    void Resize(float32 width, float32 height);
    void* GetHandle() const;

    void RunAsyncOnUIThread(const Function<void()>& task);

private:
    WindowOsX(Dispatcher* dispatcher_, WindowBackend* window_);
    ~WindowOsX();

    WindowOsX(const WindowOsX&) = delete;
    WindowOsX& operator=(const WindowOsX&) = delete;

    bool CreateNativeWindow();
    void ResizeNativeWindow(int32 width, int32 height);

    //void PostCustomMessage(const EventWin32& e);
    //void ProcessCustomEvents();

private:
    Dispatcher* dispatcher = nullptr;
    WindowBackend* window = nullptr;

    bool isMinimized = false;

    //Mutex mutex;
    //Vector<EventWin32> events;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__

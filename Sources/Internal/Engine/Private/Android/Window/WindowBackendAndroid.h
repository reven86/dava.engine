#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Private/Dispatcher/UIDispatcher.h"

#include "Functional/Function.h"

#include <jni.h>
#include <android/native_window_jni.h>

namespace DAVA
{
namespace Private
{
class WindowBackend final
{
public:
    WindowBackend(EngineBackend* e, Window* w);
    ~WindowBackend();

    WindowBackend(const WindowBackend&) = delete;
    WindowBackend& operator=(const WindowBackend&) = delete;

    void* GetHandle() const;
    WindowNativeService* GetNativeService() const;

    bool Create(float32 width, float32 height);
    void Resize(float32 width, float32 height);
    void Close();
    bool IsWindowReadyForRender() const;

    void RunAsyncOnUIThread(const Function<void()>& task);

    void TriggerPlatformEvents();

private:
    void DoResizeWindow(float32 width, float32 height);
    void DoCloseWindow();

    void EventHandler(const UIDispatcherEvent& e);

    void OnResume();
    void OnPause();
    void SurfaceChanged(JNIEnv* env, jobject surface, int32 width, int32 height);
    void SurfaceDestroyed();
    void OnTouch(int32 action, int32 touchId, float32 x, float32 y);

private:
    ANativeWindow* androidWindow = nullptr;
    EngineBackend* engine = nullptr;
    MainDispatcher* dispatcher = nullptr;
    Window* window = nullptr;

    UIDispatcher platformDispatcher;
    std::unique_ptr<WindowNativeService> nativeService;

    bool firstTimeSurfaceChanged = true;

    // Friends
    friend struct AndroidBridge;
};

inline void* WindowBackend::GetHandle() const
{
    return androidWindow;
}

inline WindowNativeService* WindowBackend::GetNativeService() const
{
    return nativeService.get();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__

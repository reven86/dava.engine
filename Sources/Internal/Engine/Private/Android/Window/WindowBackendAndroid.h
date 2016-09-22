#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Android/JNIBridge.h"
#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Private/Dispatcher/UIDispatcher.h"

#include "Functional/Function.h"

#include <android/native_window_jni.h>

namespace rhi
{
struct InitParam;
}

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
    void InitCustomRenderParams(rhi::InitParam& params);

    void RunAsyncOnUIThread(const Function<void()>& task);

    void TriggerPlatformEvents();

    jobject CreateNativeControl(const char8* controlClassName, void* backendPointer);

    void SetMouseMode(eMouseMode mode);
    eMouseMode GetMouseMode() const;

private:
    void DoResizeWindow(float32 width, float32 height);
    void DoCloseWindow();

    void EventHandler(const UIDispatcherEvent& e);

    void OnResume();
    void OnPause();
    void SurfaceCreated(JNIEnv* env, jobject surfaceViewInstance);
    void SurfaceChanged(JNIEnv* env, jobject surface, int32 width, int32 height);
    void SurfaceDestroyed();
    void ProcessProperties();
    void OnTouch(int32 action, int32 touchId, float32 x, float32 y);

private:
    jobject surfaceView = nullptr;
    ANativeWindow* androidWindow = nullptr;
    EngineBackend* engine = nullptr;
    MainDispatcher* dispatcher = nullptr;
    Window* window = nullptr;

    std::unique_ptr<JNI::JavaClass> surfaceViewJavaClass;
    Function<void(jobject)> triggerPlatformEvents;
    Function<jobject(jobject, jstring, jlong)> createNativeControl;

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

inline void WindowBackend::InitCustomRenderParams(rhi::InitParam& /*params*/)
{
    // No custom render params
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__

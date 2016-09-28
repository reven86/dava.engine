#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Android/JNIBridge.h"
#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Private/WindowBackendBase.h"

#include <android/native_window_jni.h>

namespace rhi
{
struct InitParam;
}

namespace DAVA
{
namespace Private
{
class WindowBackend final : public WindowBackendBase
{
public:
    WindowBackend(EngineBackend* engineBackend, Window* window);
    ~WindowBackend();

    WindowBackend(const WindowBackend&) = delete;
    WindowBackend& operator=(const WindowBackend&) = delete;

    void Resize(float32 width, float32 height);
    void Close(bool appIsTerminating);
    void SetTitle(const String& title);

    void* GetHandle() const;
    WindowNativeService* GetNativeService() const;

    bool IsWindowReadyForRender() const;
    void InitCustomRenderParams(rhi::InitParam& params);

    void TriggerPlatformEvents();

    jobject CreateNativeControl(const char8* controlClassName, void* backendPointer);

private:
    void UIEventHandler(const UIDispatcherEvent& e);
    void ReplaceAndroidNativeWindow(ANativeWindow* newAndroidWindow);

    void OnResume();
    void OnPause();
    void SurfaceCreated(JNIEnv* env, jobject surfaceViewInstance);
    void SurfaceChanged(JNIEnv* env, jobject surface, int32 width, int32 height, int32 dpi);
    void SurfaceDestroyed();
    void ProcessProperties();
    void OnTouch(int32 action, int32 touchId, float32 x, float32 y);

private:
    jobject surfaceView = nullptr;
    ANativeWindow* androidWindow = nullptr;
    std::unique_ptr<WindowNativeService> nativeService;

    std::unique_ptr<JNI::JavaClass> surfaceViewJavaClass;
    Function<void(jobject)> triggerPlatformEvents;
    Function<jobject(jobject, jstring, jlong)> createNativeControl;

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

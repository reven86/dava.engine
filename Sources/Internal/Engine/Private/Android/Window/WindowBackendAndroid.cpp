#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Android/Window/WindowBackendAndroid.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Window.h"
#include "Engine/Android/WindowNativeServiceAndroid.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/Android/AndroidBridge.h"
#include "Engine/Private/Android/PlatformCoreAndroid.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
WindowBackend::WindowBackend(EngineBackend* engineBackend, Window* window)
    : WindowBackendBase(*window,
                        *engineBackend->GetDispatcher(),
                        MakeFunction(this, &WindowBackend::UIEventHandler))
    , nativeService(new WindowNativeService(this))
{
}

WindowBackend::~WindowBackend() = default;

void WindowBackend::Detach()
{
    if (surfaceView != nullptr)
    {
        JNIEnv* env = JNI::GetEnv();
        env->DeleteGlobalRef(surfaceView);
        surfaceView = nullptr;

        DispatchWindowDestroyed(false);
    }
}

void WindowBackend::Resize(float32 width, float32 height)
{
    // Android windows are always stretched to screen size
}

void WindowBackend::Close()
{
    // For now android windows cannot be closed. Later add ability to close non-primary windows
}

bool WindowBackend::IsWindowReadyForRender() const
{
    return GetHandle() != nullptr;
}

void WindowBackend::TriggerPlatformEvents()
{
    if (uiDispatcher.HasEvents())
    {
        try
        {
            triggerPlatformEvents(surfaceView);
        }
        catch (const JNI::Exception& e)
        {
            Logger::Error("WindowBackend::TriggerPlatformEvents failed: %s", e.what());
            DVASSERT_MSG(false, e.what());
        }
    }
}

jobject WindowBackend::CreateNativeControl(const char8* controlClassName, void* backendPointer)
{
    jobject object = nullptr;
    try
    {
        jstring className = JNI::CStrToJavaString(controlClassName);
        object = createNativeControl(surfaceView, className, reinterpret_cast<jlong>(backendPointer));
    }
    catch (const JNI::Exception& e)
    {
        Logger::Error("[WindowBackend::CreateNativeControl] failed to create native control %s: %s", controlClassName, e.what());
    }
    return object;
}

void WindowBackend::UIEventHandler(const UIDispatcherEvent& e)
{
    switch (e.type)
    {
    case UIDispatcherEvent::FUNCTOR:
        e.functor();
        break;
    default:
        break;
    }
}

void WindowBackend::ReplaceAndroidNativeWindow(ANativeWindow* newAndroidWindow)
{
    if (androidWindow != nullptr)
    {
        ANativeWindow_release(androidWindow);
    }
    androidWindow = newAndroidWindow;
}

void WindowBackend::OnResume()
{
    PostVisibilityChanged(true);
    PostFocusChanged(true);
}

void WindowBackend::OnPause()
{
    PostFocusChanged(false);
    PostVisibilityChanged(false);
}

void WindowBackend::SurfaceCreated(JNIEnv* env, jobject surfaceViewInstance)
{
    // Here reference to java DavaSurfaceView instance is obtained
    if (surfaceView == nullptr)
    {
        surfaceView = env->NewGlobalRef(surfaceViewInstance);
    }
}

void WindowBackend::SurfaceChanged(JNIEnv* env, jobject surface, int32 width, int32 height)
{
    {
        ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env, surface);

        MainDispatcherEvent e(MainDispatcherEvent::FUNCTOR);
        e.functor = [this, nativeWindow]() {
            ReplaceAndroidNativeWindow(nativeWindow);
        };
        mainDispatcher.PostEvent(e);
    }

    float32 w = static_cast<float32>(width);
    float32 h = static_cast<float32>(height);
    if (firstTimeSurfaceChanged)
    {
        uiDispatcher.LinkToCurrentThread();

        try
        {
            surfaceViewJavaClass.reset(new JNI::JavaClass("com/dava/engine/DavaSurfaceView"));
            triggerPlatformEvents = surfaceViewJavaClass->GetMethod<void>("triggerPlatformEvents");
            createNativeControl = surfaceViewJavaClass->GetMethod<jobject, jstring, jlong>("createNativeControl");
        }
        catch (const JNI::Exception& e)
        {
            Logger::Error("[WindowBackend] failed to init java bridge: %s", e.what());
            DVASSERT_MSG(false, e.what());
        }

        PostWindowCreated(w, h, 1.0f, 1.0f);

        firstTimeSurfaceChanged = false;
    }
    else
    {
        PostSizeChanged(w, h, 1.0f, 1.0f);
    }
}

void WindowBackend::SurfaceDestroyed()
{
    MainDispatcherEvent e(MainDispatcherEvent::FUNCTOR);
    e.functor = [this]() {
        ReplaceAndroidNativeWindow(nullptr);
    };
    mainDispatcher.PostEvent(e);
}

void WindowBackend::ProcessProperties()
{
    uiDispatcher.ProcessEvents();
}

void WindowBackend::OnTouch(int32 action, int32 touchId, float32 x, float32 y)
{
    enum
    {
        ACTION_DOWN = 0,
        ACTION_UP = 1,
        ACTION_MOVE = 2,
        ACTION_POINTER_DOWN = 5,
        ACTION_POINTER_UP = 6
    };

    if (action == ACTION_POINTER_DOWN)
        action = ACTION_DOWN;
    else if (action == ACTION_POINTER_UP)
        action = ACTION_UP;

    MainDispatcherEvent e(window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();

    switch (action)
    {
    case ACTION_MOVE:
        e.type = MainDispatcherEvent::TOUCH_MOVE;
        e.tmoveEvent.touchId = touchId;
        e.tmoveEvent.x = x;
        e.tmoveEvent.y = y;
        break;
    case ACTION_UP:
    case ACTION_DOWN:
        e.type = action == ACTION_UP ? MainDispatcherEvent::TOUCH_UP : MainDispatcherEvent::TOUCH_DOWN;
        e.tclickEvent.touchId = touchId;
        e.tclickEvent.x = x;
        e.tclickEvent.y = y;
        break;
    default:
        return;
    }
    mainDispatcher.PostEvent(e);
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__

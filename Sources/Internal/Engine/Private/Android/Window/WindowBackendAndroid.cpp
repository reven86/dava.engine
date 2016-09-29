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
WindowBackend::WindowBackend(EngineBackend* e, Window* w)
    : engine(e)
    , dispatcher(engine->GetDispatcher())
    , window(w)
    , platformDispatcher(MakeFunction(this, &WindowBackend::EventHandler))
    , nativeService(new WindowNativeService(this))
{
}

WindowBackend::~WindowBackend() = default;

bool WindowBackend::Create(float32 width, float32 height)
{
    // For now primary window is created in java, later add ability to create secondary (presentation) windows
    return false;
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

void WindowBackend::RunAsyncOnUIThread(const Function<void()>& task)
{
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::FUNCTOR;
    e.functor = task;
    platformDispatcher.PostEvent(e);
}

void WindowBackend::TriggerPlatformEvents()
{
    if (platformDispatcher.HasEvents())
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

bool WindowBackend::SetCaptureMode(eCaptureMode mode);
{
    // not implemented
    return false;
}

bool WindowBackend::SetMouseVisibility(bool visible);
{
    // not implemented
    return false;
}

void WindowBackend::DoResizeWindow(float32 width, float32 height)
{
}

void WindowBackend::DoCloseWindow()
{
}

void WindowBackend::EventHandler(const UIDispatcherEvent& e)
{
    switch (e.type)
    {
    case UIDispatcherEvent::RESIZE_WINDOW:
        break;
    case UIDispatcherEvent::CLOSE_WINDOW:
        break;
    case UIDispatcherEvent::FUNCTOR:
        e.functor();
        break;
    // not implemented
    // case UIDispatcherEvent::CHANGE_CAPTURE_MODE:
    // case UIDispatcherEvent::CHANGE_MOUSE_VISIBILITY:
    default:
        break;
    }
}

void WindowBackend::OnResume()
{
    window->PostVisibilityChanged(true);
    window->PostFocusChanged(true);
}

void WindowBackend::OnPause()
{
    window->PostFocusChanged(false);
    window->PostVisibilityChanged(false);
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
            if (androidWindow != nullptr)
            {
                ANativeWindow_release(androidWindow);
            }
            androidWindow = nativeWindow;
        };
        dispatcher->PostEvent(e);
    }

    if (firstTimeSurfaceChanged)
    {
        platformDispatcher.LinkToCurrentThread();

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

        MainDispatcherEvent e(MainDispatcherEvent::WINDOW_CREATED, window);
        e.windowCreatedEvent.windowBackend = this;
        e.windowCreatedEvent.size.width = static_cast<float32>(width);
        e.windowCreatedEvent.size.height = static_cast<float32>(height);
        e.windowCreatedEvent.size.scaleX = 1.0f;
        e.windowCreatedEvent.size.scaleY = 1.0f;
        dispatcher->PostEvent(e);

        firstTimeSurfaceChanged = false;
    }
    else
    {
        MainDispatcherEvent e(MainDispatcherEvent::WINDOW_SIZE_SCALE_CHANGED, window);
        e.sizeEvent.width = static_cast<float32>(width);
        e.sizeEvent.height = static_cast<float32>(height);
        e.sizeEvent.scaleX = 1.0f;
        e.sizeEvent.scaleY = 1.0f;
        dispatcher->PostEvent(e);
    }
}

void WindowBackend::SurfaceDestroyed()
{
    MainDispatcherEvent e(MainDispatcherEvent::FUNCTOR);
    e.functor = [this]()
    {
        if (androidWindow != nullptr)
        {
            ANativeWindow_release(androidWindow);
            androidWindow = nullptr;
        }
    };
    dispatcher->PostEvent(e);
}

void WindowBackend::ProcessProperties()
{
    platformDispatcher.ProcessEvents();
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
    dispatcher->PostEvent(e);
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__

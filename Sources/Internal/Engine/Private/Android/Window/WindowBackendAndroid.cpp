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

extern "C"
{

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnResume(JNIEnv* env, jclass jclazz, jlong windowBackendPointer)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    wbackend->OnResume();
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnPause(JNIEnv* env, jclass jclazz, jlong windowBackendPointer)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    wbackend->OnPause();
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnSurfaceCreated(JNIEnv* env, jclass jclazz, jlong windowBackendPointer, jobject jsurfaceView)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    wbackend->SurfaceCreated(env, jsurfaceView);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnSurfaceChanged(JNIEnv* env, jclass jclazz, jlong windowBackendPointer, jobject surface, jint width, jint height, jint surfaceWidth, jint surfaceHeight, jint dpi)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    wbackend->SurfaceChanged(env, surface, width, height, surfaceWidth, surfaceHeight, dpi);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnSurfaceDestroyed(JNIEnv* env, jclass jclazz, jlong windowBackendPointer)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    wbackend->SurfaceDestroyed();
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewProcessEvents(JNIEnv* env, jclass jclazz, jlong windowBackendPointer)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    wbackend->ProcessProperties();
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnTouch(JNIEnv* env, jclass jclazz, jlong windowBackendPointer, jint action, jint touchId, jfloat x, jfloat y)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    wbackend->OnTouch(action, touchId, x, y);
}
} // extern "C"

namespace DAVA
{
namespace Private
{
WindowBackend::WindowBackend(EngineBackend* engineBackend, Window* window)
    : engineBackend(engineBackend)
    , window(window)
    , mainDispatcher(engineBackend->GetDispatcher())
    , uiDispatcher(MakeFunction(this, &WindowBackend::UIEventHandler))
    , nativeService(new WindowNativeService(this))
{
}

WindowBackend::~WindowBackend()
{
    DVASSERT(surfaceView == nullptr);
}

void WindowBackend::Resize(float32 /*width*/, float32 /*height*/)
{
    // Android windows are always stretched to display size
}

void WindowBackend::Close(bool appIsTerminating)
{
    if (appIsTerminating)
    {
        // If application is terminating then free window resources on C++ side and send event
        // as if window has been destroyed. Engine ensures that Close with appIsTerminating with
        // true value is always called on termination.
        if (surfaceView != nullptr)
        {
            mainDispatcher->SendEvent(MainDispatcherEvent::CreateWindowDestroyedEvent(window));

            JNIEnv* env = JNI::GetEnv();
            env->DeleteGlobalRef(surfaceView);
            surfaceView = nullptr;
        }
    }
    else if (window->IsPrimary())
    {
        // Primary android window cannot be closed, instead quit application according to Engine rules.
        // TODO: later add ability to close secondary windows.
        engineBackend->Quit(0);
    }
}

void WindowBackend::SetTitle(const String& title)
{
    // Android window does not have title
}

void WindowBackend::RunAsyncOnUIThread(const Function<void()>& task)
{
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateFunctorEvent(task));
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

float32 WindowBackend::GetSurfaceScale() const
{
    return surfaceScale;
}

bool WindowBackend::SetSurfaceScale(float32 scale)
{
    DVASSERT(scale > 0.0f && scale <= 1.0f);

    try
    {
        setScale(surfaceView, scale);
        surfaceScale = scale;
        return true;
    }
    catch (const JNI::Exception& e)
    {
        Logger::Error("[WindowBackend::SetSurfaceScale] failed to set scale %f: %s", scale, e.what());
        return false;
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
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, true));
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowFocusChangedEvent(window, true));
}

void WindowBackend::OnPause()
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowFocusChangedEvent(window, false));
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, false));
}

void WindowBackend::SurfaceCreated(JNIEnv* env, jobject surfaceViewInstance)
{
    // Here reference to java DavaSurfaceView instance is obtained
    if (surfaceView == nullptr)
    {
        surfaceView = env->NewGlobalRef(surfaceViewInstance);
    }
}

void WindowBackend::SurfaceChanged(JNIEnv* env, jobject surface, int32 width, int32 height, int32 surfaceWidth, int32 surfaceHeight, int32 dpi)
{
    {
        ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env, surface);

        MainDispatcherEvent e(MainDispatcherEvent::FUNCTOR);
        e.functor = [this, nativeWindow]() {
            ReplaceAndroidNativeWindow(nativeWindow);
        };
        mainDispatcher->PostEvent(e);
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
            setScale = surfaceViewJavaClass->GetMethod<void, jfloat>("setScale");
        }
        catch (const JNI::Exception& e)
        {
            Logger::Error("[WindowBackend] failed to init java bridge: %s", e.what());
            DVASSERT_MSG(false, e.what());
        }

        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowCreatedEvent(window, w, h, surfaceWidth, surfaceHeight, dpi));
        firstTimeSurfaceChanged = false;
    }
    else
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window, w, h, surfaceWidth, surfaceHeight));
    }
}

void WindowBackend::SurfaceDestroyed()
{
    MainDispatcherEvent e(MainDispatcherEvent::FUNCTOR);
    e.functor = [this]() {
        ReplaceAndroidNativeWindow(nullptr);
    };
    mainDispatcher->PostEvent(e);
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

    MainDispatcherEvent::eType type = MainDispatcherEvent::TOUCH_DOWN;
    switch (action)
    {
    case ACTION_MOVE:
        type = MainDispatcherEvent::TOUCH_MOVE;
        break;
    case ACTION_UP:
    case ACTION_POINTER_UP:
        type = MainDispatcherEvent::TOUCH_UP;
        break;
    case ACTION_DOWN:
    case ACTION_POINTER_DOWN:
        type = MainDispatcherEvent::TOUCH_DOWN;
        break;
    default:
        return;
    }
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowTouchEvent(window, type, touchId, x, y));
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__

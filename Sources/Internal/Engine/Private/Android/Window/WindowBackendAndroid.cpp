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

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnSurfaceChanged(JNIEnv* env, jclass jclazz, jlong windowBackendPointer, jobject surface, jint width, jint height)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    wbackend->SurfaceChanged(env, surface, width, height);
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

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnMouseEvent(JNIEnv* env, jclass jclazz, jlong windowBackendPointer, jint action, jint buttonState, jfloat x, jfloat y, jfloat deltaX, jfloat deltaY, jint modifierKeys)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    wbackend->OnMouseEvent(action, buttonState, x, y, deltaX, deltaY, modifierKeys);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnTouchEvent(JNIEnv* env, jclass jclazz, jlong windowBackendPointer, jint action, jint touchId, jfloat x, jfloat y, jint modifierKeys)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    wbackend->OnTouchEvent(action, touchId, x, y, modifierKeys);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnKeyEvent(JNIEnv* env, jclass jclazz, jlong windowBackendPointer, jint action, jint keyCode, jint unicodeChar, jint modifierKeys, jboolean isRepeated)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    wbackend->OnKeyEvent(action, keyCode, unicodeChar, modifierKeys, isRepeated == JNI_TRUE);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnGamepadButton(JNIEnv* env, jclass jclazz, jlong windowBackendPointer, jint deviceId, jint action, jint keyCode)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    wbackend->OnGamepadButton(deviceId, action, keyCode);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaSurfaceView_nativeSurfaceViewOnGamepadMotion(JNIEnv* env, jclass jclazz, jlong windowBackendPointer, jint deviceId, jint axis, jfloat value)
{
    using DAVA::Private::WindowBackend;
    WindowBackend* wbackend = reinterpret_cast<WindowBackend*>(static_cast<uintptr_t>(windowBackendPointer));
    wbackend->OnGamepadMotion(deviceId, axis, value);
}

} // extern "C"

namespace DAVA
{
namespace Private
{
// Corresponding necessary constants from android.view.MotionEvent
struct AMotionEvent
{
    // Actions
    enum
    {
        ACTION_DOWN = 0,
        ACTION_UP = 1,
        ACTION_MOVE = 2,
        ACTION_POINTER_DOWN = 5,
        ACTION_POINTER_UP = 6,
        ACTION_HOVER_MOVE = 7,
        ACTION_SCROLL = 8,
    };

    // Mouse buttons
    enum
    {
        BUTTON_PRIMARY = 0x01,
        BUTTON_SECONDARY = 0x02,
        BUTTON_TERTIARY = 0x04,
    };

    // Joystick axes
    enum
    {
        AXIS_Y = 0x01,
        AXIS_RZ = 0x0E,
        AXIS_RY = 0x0D,
    };
};

// Corresponding necessary constants from android.view.KeyEvent
struct AKeyEvent
{
    // Actions
    enum
    {
        ACTION_DOWN = 0,
        ACTION_UP = 1,
    };

    // Keycodes
    enum
    {
        KEYCODE_BACK = 4,

        KEYCODE_DPAD_UP = 0x13,
        KEYCODE_DPAD_DOWN = 0x14,
        KEYCODE_DPAD_LEFT = 0x15,
        KEYCODE_DPAD_RIGHT = 0x16,

        // KEYCODE_DPAD_UP_LEFT = 0x10C,
        // KEYCODE_DPAD_DOWN_LEFT = 0x10D,
        // KEYCODE_DPAD_UP_RIGHT = 0x10E,
        // KEYCODE_DPAD_DOWN_RIGHT = 0x10F,

        META_SHIFT_ON = 0x01,
        META_ALT_ON = 0x02,
        META_CTRL_ON = 0x00001000,
    };
};

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

void WindowBackend::SurfaceChanged(JNIEnv* env, jobject surface, int32 width, int32 height)
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
        }
        catch (const JNI::Exception& e)
        {
            Logger::Error("[WindowBackend] failed to init java bridge: %s", e.what());
            DVASSERT_MSG(false, e.what());
        }

        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowCreatedEvent(window,
                                                                                w,
                                                                                h,
                                                                                1.0f,
                                                                                1.0f));
        firstTimeSurfaceChanged = false;
    }
    else
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window,
                                                                                    w,
                                                                                    h,
                                                                                    1.0f,
                                                                                    1.0f));
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

void WindowBackend::OnMouseEvent(int32 action, int32 nativeButtonState, float32 x, float32 y, float32 deltaX, float32 deltaY, int32 nativeModifierKeys)
{
    eModifierKeys modifierKeys = GetModifierKeys(nativeModifierKeys);
    switch (action)
    {
    case AMotionEvent::ACTION_MOVE:
    case AMotionEvent::ACTION_HOVER_MOVE:
        if (lastMouseMoveX != x && lastMouseMoveY != y)
        {
            mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseMoveEvent(window, x, y, modifierKeys, false));
            lastMouseMoveX = x;
            lastMouseMoveY = y;
        }
        break;
    case AMotionEvent::ACTION_SCROLL:
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseWheelEvent(window, x, y, deltaX, deltaY, modifierKeys, false));
        break;
    default:
        break;
    }

    // What you should know about mouse handling on android:
    //  - android does not send which mouse button generates up event (ACTION_UP)
    //  - android sometimes does not send mouse button down events (ACTION_DOWN)
    //  - android sends mouse button states in every mouse event
    // So we manually track mouse button states and send appropriate events into dava.engine.
    std::bitset<MOUSE_BUTTON_COUNT> state = GetMouseButtonState(nativeButtonState);
    std::bitset<MOUSE_BUTTON_COUNT> change = mouseButtonState ^ state;
    if (change.any())
    {
        MainDispatcherEvent e = MainDispatcherEvent::CreateWindowMouseClickEvent(window, MainDispatcherEvent::MOUSE_BUTTON_UP, eMouseButtons::LEFT, x, y, 1, modifierKeys, false);
        for (size_t i = 0, n = change.size(); i < n; ++i)
        {
            if (change[i])
            {
                e.type = state[i] ? MainDispatcherEvent::MOUSE_BUTTON_DOWN : MainDispatcherEvent::MOUSE_BUTTON_UP;
                e.mouseEvent.button = static_cast<eMouseButtons>(i + 1);
                mainDispatcher->PostEvent(e);
            }
        }
    }
    mouseButtonState = state;
}

void WindowBackend::OnTouchEvent(int32 action, int32 touchId, float32 x, float32 y, int32 nativeModifierKeys)
{
    MainDispatcherEvent::eType type = MainDispatcherEvent::TOUCH_DOWN;
    switch (action)
    {
    case AMotionEvent::ACTION_MOVE:
        type = MainDispatcherEvent::TOUCH_MOVE;
        break;
    case AMotionEvent::ACTION_UP:
    case AMotionEvent::ACTION_POINTER_UP:
        type = MainDispatcherEvent::TOUCH_UP;
        break;
    case AMotionEvent::ACTION_DOWN:
    case AMotionEvent::ACTION_POINTER_DOWN:
        type = MainDispatcherEvent::TOUCH_DOWN;
        break;
    default:
        return;
    }

    eModifierKeys modifierKeys = GetModifierKeys(nativeModifierKeys);
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowTouchEvent(window, type, touchId, x, y, modifierKeys));
}

void WindowBackend::OnKeyEvent(int32 action, int32 keyCode, int32 unicodeChar, int32 nativeModifierKeys, bool isRepeated)
{
    if (keyCode == AKeyEvent::KEYCODE_BACK)
    {
        if (action == AKeyEvent::ACTION_UP)
        {
            mainDispatcher->PostEvent(MainDispatcherEvent(MainDispatcherEvent::BACK_NAVIGATION));
        }
    }
    else
    {
        bool isPressed = action == AKeyEvent::ACTION_DOWN;
        eModifierKeys modifierKeys = GetModifierKeys(nativeModifierKeys);
        MainDispatcherEvent::eType type = isPressed ? MainDispatcherEvent::KEY_DOWN : MainDispatcherEvent::KEY_UP;
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(window, type, keyCode, modifierKeys, isRepeated));

        if (isPressed && unicodeChar != 0)
        {
            mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(window, MainDispatcherEvent::KEY_CHAR, unicodeChar, modifierKeys, isRepeated));
        }
    }
}

void WindowBackend::OnGamepadButton(int32 deviceId, int32 action, int32 keyCode)
{
    // In DAVA::GamepadDevice Dpad buttons are interpreted as range [-1, 1], other buttons are expected to have value
    // 1 for down state and 0 for up state.
    float32 value = 0.f;
    if (action == AKeyEvent::ACTION_DOWN)
    {
        switch (keyCode)
        {
        case AKeyEvent::KEYCODE_DPAD_DOWN:
            value = -1.f;
            break;
        case AKeyEvent::KEYCODE_DPAD_LEFT:
            value = -1.f;
            break;
        case AKeyEvent::KEYCODE_DPAD_UP:
        case AKeyEvent::KEYCODE_DPAD_RIGHT:
        default:
            value = 1.f;
            break;
        }
    }

    MainDispatcherEvent::eType type = action == AKeyEvent::ACTION_DOWN ? MainDispatcherEvent::GAMEPAD_BUTTON_DOWN : MainDispatcherEvent::GAMEPAD_BUTTON_UP;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateGamepadButtonEvent(deviceId, type, keyCode, value));
}

void WindowBackend::OnGamepadMotion(int32 deviceId, int32 axis, float32 value)
{
    // Invert joystick Y-axis
    // TODO: find out why Y-axis should be inverted
    if (axis == AMotionEvent::AXIS_Y || axis == AMotionEvent::AXIS_RZ || axis == AMotionEvent::AXIS_RY)
    {
        value = -value;
    }
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateGamepadMotionEvent(deviceId, axis, value));
}

std::bitset<WindowBackend::MOUSE_BUTTON_COUNT> WindowBackend::GetMouseButtonState(int32 nativeButtonState)
{
    std::bitset<MOUSE_BUTTON_COUNT> state;
    // Android supports only three mouse buttons
    state.set(0, (nativeButtonState & AMotionEvent::BUTTON_PRIMARY) == AMotionEvent::BUTTON_PRIMARY);
    state.set(1, (nativeButtonState & AMotionEvent::BUTTON_SECONDARY) == AMotionEvent::BUTTON_SECONDARY);
    state.set(2, (nativeButtonState & AMotionEvent::BUTTON_TERTIARY) == AMotionEvent::BUTTON_TERTIARY);
    return state;
}

eModifierKeys WindowBackend::GetModifierKeys(int32 nativeModifierKeys)
{
    eModifierKeys result = eModifierKeys::NONE;
    if (nativeModifierKeys & AKeyEvent::META_SHIFT_ON)
    {
        result |= eModifierKeys::SHIFT;
    }
    if (nativeModifierKeys & AKeyEvent::META_ALT_ON)
    {
        result |= eModifierKeys::ALT;
    }
    if (nativeModifierKeys & AKeyEvent::META_CTRL_ON)
    {
        result |= eModifierKeys::CONTROL;
    }
    return result;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__

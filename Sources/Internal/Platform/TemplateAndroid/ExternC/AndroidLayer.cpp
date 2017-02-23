#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Android/AndroidBridge.h"
#include "Logger/Logger.h"
#include "Platform/DeviceInfo.h"
#include "Time/SystemTimer.h"

namespace DAVA
{
namespace JNI
{
JavaVM* GetJVM()
{
    return Private::AndroidBridge::GetJavaVM();
}

} // namespace JNI
} // namespace DAVA

#else // __DAVAENGINE_COREV2__

#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "Logger/Logger.h"
#include "Utils/Utils.h"
#include "Input/AccelerometerAndroid.h"
#include "AndroidDelegate.h"
#include "Platform/TemplateAndroid/AndroidCrashReport.h"
#include "Platform/TemplateAndroid/JniExtensions.h"
#include "UI/Private/Android/WebViewControlAndroid.h"
#include "Debug/DVAssertMessage.h"
#include "Platform/TemplateAndroid/DeviceInfoAndroid.h"
#include "Utils/UtilsAndroid.h"
#include "UI/UITextFieldAndroid.h"
#include "UI/Private/Android/MovieViewControlAndroid.h"
#include "Platform/TemplateAndroid/DPIHelperAndroid.h"
#include "Platform/TemplateAndroid/AndroidCrashReport.h"
#include "Platform/TemplateAndroid/FileListAndroid.h"
#include "Utils/UTF8Utils.h"
#include "Engine/Android/JNIBridge.h"
#include <dirent.h>

#include "Render/Renderer.h"

extern "C"
{
jint JNI_OnLoad(JavaVM* vm, void* reserved);

//JNIApplication
JNIEXPORT void JNICALL Java_com_dava_framework_JNIApplication_OnCreateApplication(JNIEnv* env, jobject classthis, jstring externalPath, jstring internalPath, jstring apppath, jstring logTag, jstring packageName, jstring commandLineParams);
JNIEXPORT void JNICALL Java_com_dava_framework_JNIApplication_OnConfigurationChanged(JNIEnv* env, jobject classthis);
JNIEXPORT void JNICALL Java_com_dava_framework_JNIApplication_OnLowMemoryWarning(JNIEnv* env, jobject classthis);
JNIEXPORT void JNICALL Java_com_dava_framework_JNIApplication_OnTerminate(JNIEnv* env, jobject classthis);
JNIEXPORT void JNICALL Java_com_dava_framework_JNIApplication_SetAssetManager(JNIEnv* env, jobject classthis, jobject assetManager);

//JNIActivity
JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnCreate(JNIEnv* env, jobject classthis);
JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnStart(JNIEnv* env, jobject classthis);
JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnStop(JNIEnv* env, jobject classthis);
JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeFinishing(JNIEnv* env, jobject classthis);
JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnDestroy(JNIEnv* env, jobject classthis);
JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnAccelerometer(JNIEnv* env, jobject classthis, jfloat x, jfloat y, jfloat z);
JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnGamepadAvailable(JNIEnv* env, jobject classthis, jboolean isAvailable);
JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnGamepadTriggersAvailable(JNIEnv* env, jobject classthis, jboolean isAvailable);
JNIEXPORT bool JNICALL Java_com_dava_framework_JNIActivity_nativeIsMultitouchEnabled(JNIEnv* env, jobject classthis);
JNIEXPORT int JNICALL Java_com_dava_framework_JNIActivity_nativeGetDesiredFPS(JNIEnv* env, jobject classthis);
JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnResume(JNIEnv* env, jobject classthis);
JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnPause(JNIEnv* env, jobject classthis, jboolean isLock);

//JNISurfaceView
JNIEXPORT void JNICALL Java_com_dava_framework_JNISurfaceView_nativeOnInput(JNIEnv* env, jobject classthis, jint action, jint source, jint groupSize, jobject allInputs);
JNIEXPORT void JNICALL Java_com_dava_framework_JNISurfaceView_nativeOnKeyDown(JNIEnv* env, jobject classthis, jint keyCode, jint modifiers);
JNIEXPORT void JNICALL Java_com_dava_framework_JNISurfaceView_nativeOnKeyUp(JNIEnv* env, jobject classthis, jint keyCode, jint modifiers);
JNIEXPORT void JNICALL Java_com_dava_framework_JNISurfaceView_nativeOnGamepadElement(JNIEnv* env, jobject classthis, jint elementKey, jfloat value, jboolean isKeycode, jint modifiers);
JNIEXPORT void JNICALL Java_com_dava_framework_JNISurfaceView_nativeSurfaceCreated(JNIEnv* env, jobject classthis, jobject surface);
JNIEXPORT void JNICALL Java_com_dava_framework_JNISurfaceView_nativeSurfaceChanged(JNIEnv* env, jobject classthis, jobject surface, jint width, jint height);
JNIEXPORT void JNICALL Java_com_dava_framework_JNISurfaceView_nativeSurfaceDestroyed(JNIEnv* env, jobject classthis);

JNIEXPORT void JNICALL Java_com_dava_framework_JNISurfaceView_nativeProcessFrame(JNIEnv* env, jobject classthis);
//DeviceInfo
JNIEXPORT void JNICALL Java_com_dava_framework_DataConnectionStateListener_OnCarrierNameChanged(JNIEnv* env, jobject classthis);
};

namespace
{
DAVA::CorePlatformAndroid* core = nullptr;

DAVA::String documentsFolderPathEx;
DAVA::String documentsFolderPathIn;
DAVA::String folderDocuments;
DAVA::String assetsFolderPath;
DAVA::String androidLogTag;
DAVA::String androidPackageName;

DAVA::JNI::JavaClass* gArrayListClass = nullptr;
DAVA::JNI::JavaClass* gInputEventClass = nullptr;

DAVA::Function<jobject(jobject, jint)> gArrayListGetMethod;
DAVA::Function<jint(jobject)> gArrayListSizeMethod;

jfieldID gInputEventTidField;
jfieldID gInputEventXField;
jfieldID gInputEventYField;
jfieldID gInputEventTimeField;
jfieldID gInputEventModifiersField;

AndroidDelegate* androidDelegate = nullptr;
ANativeWindow* nativeWindow = nullptr;
}
namespace DAVA
{
namespace JNI
{
JavaVM* GetJVM()
{
    if (androidDelegate == nullptr)
    {
        LOGE("androidDelegate == nullptr file %s(%d)", __FILE__, __LINE__);
        return nullptr;
    }
    JavaVM* jvm = androidDelegate->GetVM();
    if (jvm == nullptr)
    {
        LOGE("jvm == nullptr file %s(%d)", __FILE__, __LINE__);
        return nullptr;
    }
    return jvm;
}
} // end namespace JNI
} // end namespace DAVA

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6))
    {
        LOGE("Failed get java environment");
        return -1;
    }

    androidDelegate = new AndroidDelegate(vm);

    DAVA::JNI::JavaClass::Initialize();

    DAVA::AndroidCrashReport::Init(env);
    LOGI("finished JNI_OnLoad");

    return JNI_VERSION_1_6;
}

void InitApplication(JNIEnv* env, const DAVA::String& commandLineParams)
{
    if (!core)
    {
        core = new DAVA::CorePlatformAndroid(commandLineParams);
        if (core)
        {
            core->CreateAndroidWindow(documentsFolderPathEx.c_str(),
                                      documentsFolderPathIn.c_str(),
                                      assetsFolderPath.c_str(),
                                      androidLogTag.c_str(), androidDelegate);
        }
        else
        {
            LOGE("[InitApplication] Can't allocate space for CoreAndroidPlatform");
        }
    }
    else
    {
        DAVA::Logger::Warning("[InitApplication] CoreAndroidPlatform has been created");
    }
}

void DeinitApplication()
{
    if (core)
    {
        core->Quit();
        core->ReleaseSingletons();
        core = nullptr;
    }

    SafeDelete(androidDelegate);
}

// CALLED FROM JNIApplication
// private static native void OnCreateApplication();
// private static native void OnConfigurationChanged();
// private static native void OnLowMemory();
// private static native void OnTerminate()

void Java_com_dava_framework_JNIApplication_OnCreateApplication(JNIEnv* env, jobject classthis, jstring externalPath, jstring internalPath, jstring apppath, jstring logTag, jstring packageName, jstring commandLineParams)
{
    LOGE("start OnCreateApplication");
    androidLogTag = DAVA::JNI::ToString(logTag);

    LOGE("next logTag OnCreateApplication");

    documentsFolderPathEx = DAVA::JNI::ToString(externalPath);
    documentsFolderPathIn = DAVA::JNI::ToString(internalPath);
    assetsFolderPath = DAVA::JNI::ToString(apppath);
    androidPackageName = DAVA::JNI::ToString(packageName);
    DAVA::String commandLine = DAVA::JNI::ToString(commandLineParams);

    DAVA::Thread::InitMainThread();

    InitApplication(env, commandLine);

    gArrayListClass = new DAVA::JNI::JavaClass("java/util/ArrayList");
    gInputEventClass = new DAVA::JNI::JavaClass("com/dava/framework/JNISurfaceView$InputRunnable$InputEvent");

    gArrayListGetMethod = gArrayListClass->GetMethod<jobject, jint>("get");
    gArrayListSizeMethod = gArrayListClass->GetMethod<jint>("size");

    gInputEventTidField = env->GetFieldID(*gInputEventClass, "tid", DAVA::JNI::TypeMetrics<jint>());
    gInputEventXField = env->GetFieldID(*gInputEventClass, "x", DAVA::JNI::TypeMetrics<jfloat>());
    gInputEventYField = env->GetFieldID(*gInputEventClass, "y", DAVA::JNI::TypeMetrics<jfloat>());
    gInputEventTimeField = env->GetFieldID(*gInputEventClass, "time", DAVA::JNI::TypeMetrics<jdouble>());
    gInputEventModifiersField = env->GetFieldID(*gInputEventClass, "modifiers", DAVA::JNI::TypeMetrics<jint>());

    DAVA::Logger::Info("finish OnCreateApplication");
}

void Java_com_dava_framework_JNIApplication_OnConfigurationChanged(JNIEnv* env, jobject classthis)
{
}

void Java_com_dava_framework_JNIApplication_OnLowMemoryWarning(JNIEnv* env, jobject classthis)
{
    if (core)
    {
        DAVA::Logger::Error("__ LOW MEMORY ___  %p", env);
    }
}
void Java_com_dava_framework_JNIApplication_OnTerminate(JNIEnv* env, jobject classthis)
{
}
// END OF JNIApplication

// CALLED FROM JNIActivity
void Java_com_dava_framework_JNIActivity_nativeOnCreate(JNIEnv* env, jobject classthis)
{
    //	LOGI("___ ON CREATE ___ %p, %d;  isFirstRun = %d", env, classthis, isFirstRun);
    if (core)
    {
        core->OnCreateActivity();
    }
}

void Java_com_dava_framework_JNIActivity_nativeOnStart(JNIEnv* env, jobject classthis)
{
    if (core)
    {
        core->StartVisible();
    }
}

void Java_com_dava_framework_JNIActivity_nativeOnStop(JNIEnv* env, jobject classthis)
{
    if (core)
    {
        core->StopVisible();
    }
}

void Java_com_dava_framework_JNIActivity_nativeFinishing(JNIEnv* env, jobject classthis)
{
    DeinitApplication();
}

void Java_com_dava_framework_JNIActivity_nativeOnDestroy(JNIEnv* env, jobject classthis)
{
    if (core)
    {
        core->OnDestroyActivity();
    }

    DAVA::SafeDelete(gArrayListClass);
    DAVA::SafeDelete(gInputEventClass);
}

void Java_com_dava_framework_JNIActivity_nativeOnAccelerometer(JNIEnv* env, jobject classthis, jfloat x, jfloat y, jfloat z)
{
    DAVA::AccelerometerAndroidImpl* accelerometer = static_cast<DAVA::AccelerometerAndroidImpl*>(DAVA::Accelerometer::Instance());
    if (accelerometer)
    {
        accelerometer->SetAccelerationData(x, y, z);
    }
}

void Java_com_dava_framework_JNIActivity_nativeOnGamepadAvailable(JNIEnv* env, jobject classthis, jboolean isAvailable)
{
    if (core)
    {
        core->OnGamepadAvailable(isAvailable);
    }
}

void Java_com_dava_framework_JNIActivity_nativeOnGamepadTriggersAvailable(JNIEnv* env, jobject classthis, jboolean isAvailable)
{
    if (core)
    {
        core->OnGamepadTriggersAvailable(isAvailable);
    }
}

bool Java_com_dava_framework_JNIActivity_nativeIsMultitouchEnabled(JNIEnv* env, jobject classthis)
{
    if (core)
    {
        return core->IsMultitouchEnabled();
    }
    return true;
}

int Java_com_dava_framework_JNIActivity_nativeGetDesiredFPS(JNIEnv* env, jobject classthis)
{
    return DAVA::Renderer::GetDesiredFPS();
}

namespace android_details
{
DAVA::UIEvent::Phase GetPhase(DAVA::int32 action, DAVA::int32 source)
{
    DAVA::UIEvent::Phase phase = DAVA::UIEvent::Phase::DRAG;
    switch (action)
    {
    case 5: //ACTION_POINTER_DOWN
    case 0: //ACTION_DOWN
        phase = DAVA::UIEvent::Phase::BEGAN;
        break;

    case 6: //ACTION_POINTER_UP
    case 1: //ACTION_UP
        phase = DAVA::UIEvent::Phase::ENDED;
        break;

    case 3: //ACTION_CANCEL
        phase = DAVA::UIEvent::Phase::CANCELLED;
        break;

    case 4: //ACTION_OUTSIDE
        break;
    }

    return phase;
}

DAVA::UIEvent CreateUIEventFromJavaEvent(JNIEnv* env, jobject input,
                                         jint action, jint source)
{
    DAVA::UIEvent event;
    event.touchId = static_cast<DAVA::uint32>(env->GetIntField(input, gInputEventTidField));
    event.point.x = event.physPoint.x = env->GetFloatField(input,
                                                           gInputEventXField);
    event.point.y = event.physPoint.y = env->GetFloatField(input,
                                                           gInputEventYField);
    // timestamp in seconds, JNIEnv retern in milliseconds
    event.timestamp = env->GetDoubleField(input, gInputEventTimeField) / 1000.0;
    event.modifiers = env->GetIntField(input, gInputEventModifiersField);
    event.phase = GetPhase(action, source);

    if (event.phase == DAVA::UIEvent::Phase::JOYSTICK)
    {
        event.device = DAVA::eInputDevices::GAMEPAD;
    }
    else if (event.phase >= DAVA::UIEvent::Phase::CHAR && event.phase <= DAVA::UIEvent::Phase::KEY_UP)
    {
        event.device = DAVA::eInputDevices::KEYBOARD;
    }
    else
    {
        event.device = DAVA::eInputDevices::TOUCH_SURFACE;
    }

    return event;
}
} // end namespace android_details

// CALLED FROM JNIGLSurfaceView

void Java_com_dava_framework_JNISurfaceView_nativeOnInput(JNIEnv* env, jobject classthis, jint action, jint source, jint groupSize, jobject javaAllInputs)
{
    if (core)
    {
        DAVA::Vector<DAVA::UIEvent> allInputs;

        int allInputsCount = gArrayListSizeMethod(javaAllInputs);
        int inputsCount = allInputsCount;

        for (int groupStartIndex = 0; groupStartIndex < inputsCount; groupStartIndex += groupSize)
        {
            int groupEndIndex = groupStartIndex + groupSize;

            allInputs.clear();

            for (int touchIndex = groupStartIndex; touchIndex < groupEndIndex; ++touchIndex)
            {
                if (touchIndex < allInputsCount)
                {
                    jobject jInput = gArrayListGetMethod(javaAllInputs, touchIndex);

                    DAVA::UIEvent event = android_details::CreateUIEventFromJavaEvent(env, jInput, action, source);
                    allInputs.push_back(event);

                    env->DeleteLocalRef(jInput);
                }
            }
            core->OnInput(allInputs);
        }
    }
}

void Java_com_dava_framework_JNISurfaceView_nativeOnKeyDown(JNIEnv* env, jobject classthis, jint keyCode, jint modifiers)
{
    if (core)
    {
        core->KeyDown(keyCode, static_cast<DAVA::uint32>(modifiers));
    }
}

void Java_com_dava_framework_JNISurfaceView_nativeOnKeyUp(JNIEnv* env, jobject classthis, jint keyCode, jint modifiers)
{
    if (core)
    {
        core->KeyUp(keyCode, static_cast<DAVA::uint32>(modifiers));
    }
}

void Java_com_dava_framework_JNISurfaceView_nativeOnGamepadElement(JNIEnv* env, jobject classthis, jint elementKey, jfloat value, jboolean isKeycode, jint modifiers)
{
    if (core)
    {
        core->OnGamepadElement(elementKey, value, isKeycode, static_cast<DAVA::uint32>(modifiers));
    }
}

void Java_com_dava_framework_JNISurfaceView_nativeSurfaceCreated(JNIEnv* env, jobject classthis, jobject surface)
{
    if (nativeWindow)
    {
        ANativeWindow_release(nativeWindow);
    }

    nativeWindow = ANativeWindow_fromSurface(env, surface);

    if (core)
    {
        core->SetNativeWindow(nativeWindow);
    }
}

void Java_com_dava_framework_JNISurfaceView_nativeSurfaceChanged(JNIEnv* env, jobject classthis, jobject surface, jint width, jint height)
{
    if (nativeWindow)
    {
        ANativeWindow_release(nativeWindow);
    }

    nativeWindow = ANativeWindow_fromSurface(env, surface);

    if (core)
    {
        core->SetNativeWindow(nativeWindow);
        core->RenderReset(width, height);
    }
}

void Java_com_dava_framework_JNISurfaceView_nativeSurfaceDestroyed(JNIEnv* env, jobject classthis)
{
    if (nativeWindow)
    {
        ANativeWindow_release(nativeWindow);
        nativeWindow = nullptr;

        if (core)
        {
            core->SetNativeWindow(nullptr);

            rhi::ResetParam params;
            params.width = 0;
            params.height = 0;
            params.window = nullptr;
            DAVA::Renderer::Reset(params);
        }
    }
}

void Java_com_dava_framework_DataConnectionStateListener_OnCarrierNameChanged(JNIEnv* env, jobject classthis)
{
    // TODO: add callback in Core V2
    DAVA::DeviceInfo::carrierNameChanged.Emit(DAVA::DeviceInfo::GetCarrierName());
}

void Java_com_dava_framework_JNISurfaceView_nativeProcessFrame(JNIEnv* env, jobject classthis)
{
    if (core)
    {
        core->ProcessFrame();
    }
}

static DAVA::int64 goBackgroundTimeRelativeToBoot = 0;
static DAVA::int64 goBackgroundTime = 0;

void Java_com_dava_framework_JNIActivity_nativeOnResume(JNIEnv* env, jobject classthis)
{
    if (goBackgroundTimeRelativeToBoot > 0)
    {
        DAVA::int64 timeSpentInBackground1 = DAVA::SystemTimer::GetSystemUptimeUs() - goBackgroundTimeRelativeToBoot;
        DAVA::int64 timeSpentInBackground2 = DAVA::SystemTimer::GetUs() - goBackgroundTime;

        DAVA::Logger::Debug("Time spent in background %lld us (reported by SystemTimer %lld us)", timeSpentInBackground1, timeSpentInBackground2);
        if (timeSpentInBackground1 - timeSpentInBackground2 > 500000l)
        {
            DAVA::Core::AdjustSystemTimer(timeSpentInBackground1 - timeSpentInBackground2);
        }
    }

    if (core)
    {
        core->StartForeground();
    }
}

void Java_com_dava_framework_JNIActivity_nativeOnPause(JNIEnv* env, jobject classthis, jboolean isLock)
{
    goBackgroundTimeRelativeToBoot = DAVA::SystemTimer::GetSystemUptimeUs();
    goBackgroundTime = DAVA::SystemTimer::GetUs();

    if (core)
    {
        core->StopForeground(isLock);
    }
}

// END OF JNISurfaceView

#endif // !__DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"
#include "Notification/LocalNotificationController.h"
#include "Notification/Private/Android/LocalNotificationAndroid.h"
#include "Notification/Private/Android/NativeDelegateAndroid.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Android/JNIBridge.h"
#include "Engine/Engine.h"
#include "Notification/LocalNotificationController.h"

#include "Logger/Logger.h"

extern "C"
{
JNIEXPORT void JNICALL Java_com_dava_engine_NativeDelegate_nativeNewIntent(JNIEnv* env, jclass jclazz, jstring uid)
{
    DAVA::String uidStr = DAVA::JNI::JavaStringToString(uid);
    auto function = [uidStr]()
    {
        DAVA::Engine::Instance()->GetContext()->localNotificationController->OnNotificationPressed(uidStr);
    };
    DAVA::Engine::Instance()->RunAsyncOnMainThread(function);
}
} // extern "C"

namespace DAVA
{
NativeDelegate::NativeDelegate(LocalNotificationController& controller)
{
    try
    {
        instance = nullptr;
        JNIEnv* env = JNI::GetEnv();
        JNI::JavaClass clazz("com/dava/engine/NativeDelegate");
        release = clazz.GetMethod<void>("release");
        jmethodID classConstructor = env->GetMethodID(clazz, "<init>", "()V");
        jobject obj = env->NewObject(clazz, classConstructor);
        instance = env->NewGlobalRef(obj);
    }
    catch (const JNI::Exception& e)
    {
        Logger::Error("[NativeDelegate] failed to init java bridge: %s", e.what());
        DVASSERT_MSG(false, e.what());
        return;
    }
}

NativeDelegate::~NativeDelegate()
{
    if (instance != nullptr)
    {
        release(instance);
    }
}
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__

#include "Notification/Private/Android/NativeListenerAndroid.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

#include "Notification/LocalNotificationController.h"
#include "Notification/Private/Android/LocalNotificationAndroid.h"

#include "Engine/Android/JNIBridge.h"
#include "Engine/Engine.h"
#include "Notification/LocalNotificationController.h"

#include "Logger/Logger.h"

extern "C"
{
JNIEXPORT void JNICALL Java_com_dava_engine_notification_NativeListener_nativeNewIntent(JNIEnv* env, jclass jclazz, jstring uid)
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
namespace Private
{
NativeListener::NativeListener(LocalNotificationController& controller)
{
    try
    {
        instance = nullptr;
        JNIEnv* env = JNI::GetEnv();
        JNI::JavaClass clazz("com/dava/engine/notification/NativeListener");
        release = clazz.GetMethod<void>("release");
        jmethodID classConstructor = env->GetMethodID(clazz, "<init>", "()V");
        jobject obj = env->NewObject(clazz, classConstructor);
        instance = env->NewGlobalRef(obj);
        env->DeleteLocalRef(obj);
    }
    catch (const JNI::Exception& e)
    {
        Logger::Error("[NativeListener] failed to init java bridge: %s", e.what());
        DVASSERT_MSG(false, e.what());
        return;
    }
}

NativeListener::~NativeListener()
{
    if (instance != nullptr)
    {
        release(instance);
    }
}
} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__

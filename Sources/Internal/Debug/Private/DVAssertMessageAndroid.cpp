#include "Debug/DVAssertMessage.h"

#if !defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Android/JNIBridge.h"
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"

namespace DAVA
{
namespace DVAssertMessage
{
bool InnerShow(eModalType modalType, const char* message)
{
    JNI::JavaClass msg("com/dava/framework/JNIAssert");
    auto showMessage = msg.GetStaticMethod<jboolean, jboolean, jstring>("Assert");

    JNIEnv* env = JNI::GetEnv();
    jstring jStrMessage = env->NewStringUTF(message);
    bool waitUserInput = (ALWAYS_MODAL == modalType);
    jboolean breakExecution = showMessage(waitUserInput, jStrMessage);
    env->DeleteLocalRef(jStrMessage);

    return breakExecution == JNI_FALSE ? false : true;
}

} // namespace DVAssertMessage
} // namespace DAVA

#endif
#endif // !defined(__DAVAENGINE_COREV2__)

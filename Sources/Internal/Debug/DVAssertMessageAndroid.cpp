#include "DVAssertMessage.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "Platform/TemplateAndroid/JniHelpers.h"
#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"

namespace DAVA
{
namespace DVAssertMessage
{
bool InnerShow(eModalType modalType, const char* message)
{
    const JNI::JavaClass* jni_assert_class = JNI::JavaClass::Get("com/dava/framework/JNIAssert");
    if (!jni_assert_class) // com/dava/framework/JNIAssert wasn't registered
    {
        Logger::Error("com/dava/framework/JNIAssert wasn't registered");
        return false;
    }

    auto showMessage = jni_assert_class->GetStaticMethod<jboolean, jboolean, jstring>("Assert");
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

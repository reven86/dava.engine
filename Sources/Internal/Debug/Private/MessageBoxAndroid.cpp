#include "Base/Platform.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_ANDROID__)

#include "Base/BaseTypes.h"
#include "Engine/Engine.h"
#include "Engine/Android/JNIBridge.h"
#include "Engine/Private/Android/AndroidBridge.h"
#include "Logger/Logger.h"

extern DAVA::Private::AndroidBridge* androidBridge;

namespace DAVA
{
namespace Debug
{
int MessageBox(const String& title, const String& message, const Vector<String>& buttons, int /*defaultButton*/)
{
    DVASSERT(0 < buttons.size() && buttons.size() <= 3);

    try
    {
        // TODO: make use JNI::ObjectRef after merging corev2_android branch to avoid jobject leaks on exception
        JNI::JavaClass msgboxClass("com/dava/engine/MessageBox");
        Function<jint(jstring, jstring, jstringArray)> showModal = msgboxClass.GetStaticMethod<jint, jstring, jstring, jstringArray>("messageBox");

        jstring jtitle = JNI::StringToJavaString(title);
        jstring jmessage = JNI::StringToJavaString(message);

        JNIEnv* env = JNI::GetEnv();
        jclass stringClass = JNI::LoadJavaClass("java/lang/String", true, env);
        jsize n = static_cast<jsize>(buttons.size());
        jstringArray jbuttons = static_cast<jstringArray>(env->NewObjectArray(n, stringClass, nullptr));
        for (jsize i = 0; i < n; ++i)
        {
            jstring jbuttonName = JNI::StringToJavaString(buttons[i]);
            env->SetObjectArrayElement(jbuttons, i, jbuttonName);
            JNI::CheckJavaException(env, true);
            env->DeleteLocalRef(jbuttonName);
        }

        jint r = showModal(jtitle, jmessage, jbuttons);

        env->DeleteLocalRef(jtitle);
        env->DeleteLocalRef(jmessage);
        env->DeleteLocalRef(jbuttons); // TODO: should I delete array elements explicitly?
        return static_cast<int>(r);
    }
    catch (const JNI::Exception& e)
    {
        Logger::Error("MessageBox failed: %s", e.what());
        return -1;
    }
}

} // namespace Debug
} // namespace DAVA

#endif // defined(__DAVAENGINE_ANDROID__)
#endif // defined(__DAVAENGINE_COREV2__)

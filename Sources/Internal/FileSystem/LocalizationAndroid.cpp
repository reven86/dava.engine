#include "Base/Platform.h"

#if defined(__DAVAENGINE_ANDROID__) 

#include "Engine/Android/JNIBridge.h"
#include "FileSystem/LocalizationSystem.h"
#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"

namespace DAVA
{
String LocalizationSystem::GetDeviceLocale(void) const
{
    JNI::JavaClass jniLocalisation("com/dava/framework/JNILocalization");
    Function<jstring()> getLocale = jniLocalisation.GetStaticMethod<jstring>("GetLocale");

    return JNI::ToString(getLocale());
}
};

#endif

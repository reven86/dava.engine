#include "Platform/DPIHelper.h"
#include "DPIHelperAndroid.h"

namespace DAVA
{
JniDpiHelper::JniDpiHelper()
    : jniDpiHelper("com/dava/framework/JNIDpiHelper")
{
    getScreenDPI = jniDpiHelper.GetStaticMethod<jint>("GetScreenDPI");
}

uint32 JniDpiHelper::GetScreenDPI()
{
    uint32 dpi = 0;
    dpi = getScreenDPI();
    return dpi;
}

uint32 DPIHelper::GetScreenDPI()
{
    JniDpiHelper helper;
    return helper.GetScreenDPI();
}

} //namespace DAVA

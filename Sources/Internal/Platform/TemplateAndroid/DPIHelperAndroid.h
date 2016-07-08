#ifndef __FRAMEWORK__DPIHELPERANDROID__
#define __FRAMEWORK__DPIHELPERANDROID__

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/JniHelpers.h"

namespace DAVA
{
class JniDpiHelper
{
public:
    JniDpiHelper();
    uint32 GetScreenDPI();

private:
    JNI::JavaClass jniDpiHelper;
    Function<jint()> getScreenDPI;
};
};

#endif //__DAVAENGINE_ANDROID__

#endif /* defined(__FRAMEWORK__DPIHELPERANDROID__) */

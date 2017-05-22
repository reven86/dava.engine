#ifndef __DAVAENGINE_SCREENMANAGER_H__
#define __DAVAENGINE_SCREENMANAGER_H__

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WINDOWS__) || defined(__DAVAENGINE_LINUX__)
    #include "UI/UIScreenManagerDefault.h"
#elif defined(__DAVAENGINE_IPHONE__)
    #include "UI/UIScreenManageriPhone.h"
#elif defined(__DAVAENGINE_ANDROID__)
    #include "UI/UIScreenManagerAndroid.h"
#else //PLATFORMS
//other platforms
#endif //PLATFORMS

#endif // __DAVAENGINE_SCREENMANAGER_C_H__
#pragma once

//#define MEMPROF_ENABLE  1

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN32__)
#   define MEMPROF_WIN32    1
#elif defined(__DAVAENGINE_ANDROID__)
#   define MEMPROF_ANDROID
#elif defined(__DAVAENGINE_MACOS__)
#   define MEMPROF_MACOS    1
#elif defined(__DAVAENGINE_IPHONE__)
#   define MEMPROF_IOS      1
#else
#   error "Unsupported platform"
#endif

/*
#if defined(WIN32) || defined(_WIN32)
#   define MEMPROF_WIN32   1
#elif defined(__ANDROID__)
#   define MEMPROF_ANDROID
#elif defined(__APPLE__)
#   include <TargetConditionals.h>
#   if TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR
#       define MEMPROF_IOS      1
#   elif TARGET_OS_IPHONE
#       define MEMPROF_IOS      1
#   elif TARGET_OS_MAC
#       define MEMPROF_MACOS    1
#   endif
#endif
*/

#pragma once

#define MEMPROF_ENABLE  1

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

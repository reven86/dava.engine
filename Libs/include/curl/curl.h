#pragma once
#include "Base/Platform.h"

#if defined (__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#    include "iOS_MacOS/curl.h"
#else
#    include "Others/curl.h"
#endif

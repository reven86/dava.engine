/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

/*
*/

#ifndef __DAVAENGINE_PLATFORM_DETECTION__
#define __DAVAENGINE_PLATFORM_DETECTION__

#include "DAVAConfig.h"

//Detections of iPhone
#if defined(TARGET_OS_IPHONE)

#if !defined(__DAVAENGINE_IPHONE__) // for old projects we check if users defined it
#define __DAVAENGINE_IPHONE__
#endif

//Detection of MacOS
#elif defined(__GNUC__) && \
       (defined(__APPLE_CPP__) || defined(__APPLE_CC__) || defined(__MACOS_CLASSIC__))
#define __DAVAENGINE_MACOS__
#endif

//Detaction of Apple platform
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)

#define __DAVAENGINE_APPLE__
#include "PlatformApple.h"

//Detection of Windows
#elif defined(_WIN32)

#define __DAVAENGINE_WINDOWS__
#include "PlatformWindows.h"

//Detection of Android
#elif defined(__ANDROID__) || defined(ANDROID)

#define __DAVAENGINE_ANDROID__
#include "PlatformAndroid.h"

#else
#error Unsupported platform detected
#endif

//detecting of compiler features definitions
#if !defined(DAVA_NOINLINE)  || \
    !defined(DAVA_ALIGNOF)   || \
    !defined(DAVA_NOEXCEPT)  || \
    !defined(DAVA_CONSTEXPR) || \
    !defined(DAVA_DEPRECATED)
#error Some compiler features is not detected for current platform
#endif

//suppressing of deprecated functions
#ifdef DAVAENGINE_HIDE_DEPRECATED
#undef  DAVA_DEPRECATED
#define DAVA_DEPRECATED(func) func
#endif

#endif // __DAVAENGINE_PLATFORM_DETECTION__
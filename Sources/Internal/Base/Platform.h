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

#ifndef __DAVAENGINE_PLATFORM__
#define __DAVAENGINE_PLATFORM__

#include "DAVAConfig.h"

//-------------------------------------------------------------------------------------
//Compiler features
//-------------------------------------------------------------------------------------
//GCC && Clang
#if defined(__GNUC__)

#   define DAVA_NOINLINE    __attribute__((noinline))
#   define DAVA_FORCEINLINE __attribute__((always_inline))
#   define DAVA_ALIGNOF(x)  alignof(x)
#   define DAVA_CONSTEXPR   constexpr
#   define DAVA_DEPRECATED(func) func __attribute__ ((deprecated))
#   define DAVA_ALIGNED(Var, Len) Var __attribute__((aligned(Len)))
#   define DAVA_NOEXCEPT    noexcept

//Microsoft Visual C++
#elif defined(_MSC_VER)

#   define DAVA_NOINLINE    __declspec(noinline)
#   define DAVA_FORCEINLINE __forceinline
#   define DAVA_ALIGNOF(x)  __alignof(x)

#   if _MSC_VER >= 1900 //msvs 2015 RC or later
//Constexpr is not supported even in VS2013 (partially supported in 2015 CTP)
#       define DAVA_CONSTEXPR constexpr
#       define DAVA_NOEXCEPT noexcept
#   else
#       define DAVA_CONSTEXPR
#       define DAVA_NOEXCEPT throw()
#   endif

#   define DAVA_DEPRECATED(func) __declspec(deprecated) func
#   define DAVA_ALIGNED(Var, Len) __declspec(align(Len)) Var

#endif

//detecting of compiler features definitions
#if !defined(DAVA_NOINLINE)    || \
    !defined(DAVA_FORCEINLINE) || \
    !defined(DAVA_ALIGNOF)     || \
    !defined(DAVA_NOEXCEPT)    || \
    !defined(DAVA_CONSTEXPR)   || \
    !defined(DAVA_DEPRECATED)  || \
    !defined(DAVA_ALIGNED)
#   error Some compiler features is not defined for current platform
#endif

//suppressing of deprecated functions
#ifdef DAVAENGINE_HIDE_DEPRECATED
#   undef  DAVA_DEPRECATED
#   define DAVA_DEPRECATED(func) func
#endif

//-------------------------------------------------------------------------------------
//Platform detection
//-------------------------------------------------------------------------------------
//Detection of Apple
#if defined(__GNUC__) && \
       (defined(__APPLE_CPP__) || defined(__APPLE_CC__) || defined(__MACOS_CLASSIC__))

#   define __DAVAENGINE_APPLE__

#   include <AvailabilityMacros.h>
#   include <TargetConditionals.h>
#   include <mach/mach.h>
#   include <mach/mach_time.h>
#   include <unistd.h>

//Detection of iPhone
#   if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE

#       if !defined(__DAVAENGINE_IPHONE__) // for old projects we check if users defined it
#           define __DAVAENGINE_IPHONE__
#       endif

//Detection of MacOS
#   else
#       define __DAVAENGINE_MACOS__
#   endif

#   if MAC_OS_X_VERSION_10_6 <= MAC_OS_X_VERSION_MAX_ALLOWED
#       define __DAVAENGINE_MACOS_VERSION_10_6__
#   endif

#   define __DAVASOUND_AL__

//Detection of Windows
#elif defined(_WIN32) || defined(_WIN64)

#   define __DAVAENGINE_WINDOWS__

//Platform defines
#   define __DAVASOUND_AL__
#   define WIN32_LEAN_AND_MEAN
#   ifndef NOMINMAX
#       define NOMINMAX        // undef macro min and max from windows headers
#   endif

#   include <Windows.h>
#   include <Windowsx.h>

#   undef DrawState
#   undef GetCommandLine
#   undef GetClassName
#   undef Yield
#undef ERROR

//Detection of windows platform type
#   if !defined(WINAPI_FAMILY_PARTITION) || WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#       define __DAVAENGINE_WIN32__
#   elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#       define __DAVAENGINE_WIN_UAP__
#       define __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__
#       define __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__ DVASSERT_MSG(false, "Feature has no implementation or partly implemented")
#   endif

//Using C++11 concurrency as default
#   if defined(__DAVAENGINE_WIN_UAP__) && !defined(USE_CPP11_CONCURRENCY)
#       define USE_CPP11_CONCURRENCY
#   endif

//Detection of Android
#elif defined(__ANDROID__) || defined(ANDROID)

#   define __DAVAENGINE_ANDROID__
#   undef __DAVASOUND_AL__

#endif

#endif  // __DAVAENGINE_PLATFORM__
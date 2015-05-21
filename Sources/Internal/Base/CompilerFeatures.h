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

#ifndef __DAVAENGINE_COMPILER_FEATURES__
#define __DAVAENGINE_COMPILER_FEATURES__

#include "DavaConfig.h"

#if defined(__GNUC__)

#   define CC_NOINLINE    __attribute__((noinline))
#   define CC_FORCEINLINE __attribute__((always_inline))
#   define CC_ALIGNOF(x)  alignof(x)
#   define CC_NOEXCEPT    noexcept
#   define CC_CONSTEXPR   constexpr
#   define CC_DEPRECATED(func) func __attribute__ ((deprecated))

#elif defined(_MSC_VER)

#   define CC_NOINLINE    __declspec(noinline)
#   define CC_FORCEINLINE __forceinline
#   define CC_ALIGNOF(x)  __alignof(x)

#   if _MSC_VER >= 1900 //msvs 2015 RC or later
//Constexpr is not supported even in VS2013 (partially supported in 2015 CTP)
#       define CC_CONSTEXPR constexpr
#       define CC_NOEXCEPT noexcept
#   else
#       define CC_CONSTEXPR
#       define CC_NOEXCEPT throw()
#   endif

#   define CC_DEPRECATED(func) __declspec(deprecated) func

#endif

//detecting of compiler features definitions
#if !defined(CC_NOINLINE)    || \
    !defined(CC_FORCEINLINE) || \
    !defined(CC_ALIGNOF)     || \
    !defined(CC_NOEXCEPT)    || \
    !defined(CC_CONSTEXPR)   || \
    !defined(CC_DEPRECATED)
#   error Some compiler features is not detected for current platform
#endif

//suppressing of deprecated functions
#ifdef DAVAENGINE_HIDE_DEPRECATED
#   undef  CC_DEPRECATED
#   define CC_DEPRECATED(func) func
#endif

//Temporary client build fix
#define DAVA_DEPRECATED CC_DEPRECATED

#endif  // __DAVAENGINE_COMPILER_FEATURES__
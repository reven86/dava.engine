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

#ifndef __DAVAENGINE_PLATFORM_WINDOWS__
#define __DAVAENGINE_PLATFORM_WINDOWS__

#ifndef __DAVAENGINE_WINDOWS__
#   error Invalid direct including of this header! Use PlatformDetection.h instead
#endif

//Platform alias
#define __DAVAENGINE_WIN32__ __DAVAENGINE_WINDOWS__

//Platform defines
#define __DAVASOUND_AL__
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#   define NOMINMAX        // undef macro min and max from windows headers
#endif

#include <Windows.h>
#include <Windowsx.h>

#undef DrawState
#undef GetCommandLine
#undef GetClassName
#undef Yield

//Detection of windows platform type
#if !defined(WINAPI_FAMILY_PARTITION) || WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#   define __DAVAENGINE_WINDOWS_DESKTOP__
#elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#   define __DAVAENGINE_WINDOWS_STORE__
#   define __DAVAENGINE_WINDOWS_STORE_INCOMPLETE_IMPLEMENTATION__MARKER__
#   define __DAVAENGINE_WINDOWS_STORE_INCOMPLETE_IMPLEMENTATION__ DVASSERT_MSG(false, "Feature has no implementation or partly implemented")
#endif

#endif // __DAVAENGINE_PLATFORM_WINDOWS__
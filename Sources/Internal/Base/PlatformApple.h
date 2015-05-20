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

#ifndef __DAVAENGINE_PLATFORM_APPLE__
#define __DAVAENGINE_PLATFORM_APPLE__

#ifndef __DAVAENGINE_APPLE__
#    error Invalid direct including of this header! Use PlatformDetection.h instead
#endif

#include <AvailabilityMacros.h>
#include <TargetConditionals.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>

//Detections of iPhone
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE

#   if !defined(__DAVAENGINE_IPHONE__) // for old projects we check if users defined it
#       define __DAVAENGINE_IPHONE__
#   endif

//Detection of MacOS
#else
#   define __DAVAENGINE_MACOS__
#endif

#if MAC_OS_X_VERSION_10_6 <= MAC_OS_X_VERSION_MAX_ALLOWED
#   define __DAVAENGINE_MACOS_VERSION_10_6__
#endif

#define __DAVASOUND_AL__

#endif // __DAVAENGINE_PLATFORM_APPLE__
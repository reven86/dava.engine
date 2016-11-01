#pragma once

#include "DAVAConfig.h"

#if defined(__clang__)
#define DAVA_SWITCH_CASE_FALLTHROUGH [[clang::fallthrough]]
#else
#define DAVA_SWITCH_CASE_FALLTHROUGH
#endif

//suppressing of deprecated functions
#ifdef DAVAENGINE_HIDE_DEPRECATED
#undef DAVA_DEPRECATED
#define DAVA_DEPRECATED(func) func
#endif

//-------------------------------------------------------------------------------------
//Platform detection
//-------------------------------------------------------------------------------------
//Detection of Apple
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#include <AvailabilityMacros.h>
#include <TargetConditionals.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>
#endif

//Detection of Windows
#if defined(__DAVAENGINE_WINDOWS__)

//Platform defines
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX // undef macro min and max from windows headers
#endif

#include <Windows.h>
#include <Windowsx.h>

#undef DrawState
#undef GetCommandLine
#undef GetClassName
#undef Yield
#undef ERROR
#undef DELETE

//Detection of windows platform type
#if !defined(WINAPI_FAMILY_PARTITION) || WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#define __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__
#define __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__ DVASSERT_MSG(false, "Feature has no implementation or partly implemented")
#endif

//Using C++11 concurrency as default
#if defined(__DAVAENGINE_WIN_UAP__) && !defined(USE_CPP11_CONCURRENCY)
#define USE_CPP11_CONCURRENCY
#endif

#endif

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


#ifndef __DAVAENGINE_TYPES_H__
#define __DAVAENGINE_TYPES_H__

#include "DAVAConfig.h"
#include "Base/TemplateHelpers.h"

// Platform detection:

#if defined(__GNUC__) && ( defined(__APPLE_CPP__) || defined(__APPLE_CC__) || defined(__MACOS_CLASSIC__) )
//#if !defined(_WIN32) // fix for mac os platforms
#include <TargetConditionals.h>
#endif


#if defined(TARGET_OS_IPHONE)
#if TARGET_OS_IPHONE
	#if !defined(__DAVAENGINE_IPHONE__) // for old projects we check if users defined it
		#define __DAVAENGINE_IPHONE__
	#endif
#endif
#endif


#ifndef __DAVAENGINE_IPHONE__
#if defined(_WIN32)
#define __DAVAENGINE_WIN32__
//#elif defined(__APPLE__) || defined(MACOSX)
#elif defined(__GNUC__) && ( defined(__APPLE_CPP__) || defined(__APPLE_CC__) || defined(__MACOS_CLASSIC__) )
#define __DAVAENGINE_MACOS__

#if MAC_OS_X_VERSION_10_6 <= MAC_OS_X_VERSION_MAX_ALLOWED
#define __DAVAENGINE_MACOS_VERSION_10_6__
#endif //#if MAC_OS_X_VERSION_10_6 <= MAC_OS_X_VERSION_MAX_ALLOWED

#endif
#endif


// add some other platform detection here...
#if !defined (__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_WIN32__) && !defined(__DAVAENGINE_MACOS__)
#if defined(__ANDROID__) || defined(ANDROID) 
	#define __DAVAENGINE_ANDROID__
#endif //#if defined(__ANDROID__) || defined(ANDROID) 
#endif //#if !defined (__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_WIN32__) && !defined(__DAVAENGINE_MACOS__)


/////////
// Default headers per platform:

// Introduce useful defines:
//  DAVA_NOINLINE to tell compiler not no inline function
//  DAVA_ALIGNOF to get alignment of type
//  DAVA_NOEXCEPT to point that function doesn't throw
#if defined(__DAVAENGINE_WIN32__)
#define DAVA_NOINLINE       __declspec(noinline)
#define DAVA_ALIGNOF(x)     __alignof(x)
#define DAVA_NOEXCEPT       throw()
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#define DAVA_NOINLINE       __attribute__((noinline))
#define DAVA_ALIGNOF(x)     alignof(x)
#define DAVA_NOEXCEPT       noexcept
#elif defined(__DAVAENGINE_ANDROID__)
#define DAVA_NOINLINE       __attribute__((noinline))
#define DAVA_ALIGNOF(x)     alignof(x)
#define DAVA_NOEXCEPT       noexcept
#endif

#if defined(__DAVAENGINE_WIN32__)
#define __DAVASOUND_AL__
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX        // undef macro min and max from windows headers
#endif
#include <windows.h>
#include <windowsx.h>
#undef DrawState
#undef GetCommandLine
#undef GetClassName
#undef Yield

#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__) // Mac & iPhone
#define __DAVASOUND_AL__

#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>

#elif defined(__DAVAENGINE_ANDROID__)
//TODO: specific includes
//#define __DAVASOUND_AL__
#undef __DAVASOUND_AL__

#else
// some other platform...

#endif 


//#define _HAS_ITERATOR_DEBUGGING 0
//#define _SECURE_SCL 0

// MSVS: conversion from type1 to type2, possible loss of data

#if defined(__DAVAENGINE_WIN32__)
#pragma warning( push )
#pragma warning( disable : 4244)
#endif 

#include <cstdint>
#include <memory>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <set>
#include <stack>
#include <queue>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <cerrno>

#if defined(__DAVAENGINE_WIN32__)
#pragma warning( pop )
#endif 

//#if TARGET_OS_IPHONE_SIMULATOR //) || defined(TARGET_IPHONE)
//#if defined(__APPLE__) || defined(MACOSX)
//#define __IPHONE__
//#endif 
//#define __DAVAENGINE_IPHONE__

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
#include "MemoryManager/AllocPools.h"
#include "MemoryManager/MemoryManagerAllocator.h"
#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)

namespace DAVA
{

typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;

typedef signed char     int8;
typedef signed short    int16;
typedef signed int      int32;

#if defined(__DAVAENGINE_WIN32__)
    typedef unsigned __int64    uint64;
    typedef signed __int64      int64;
#else
    typedef unsigned long long  uint64;
    typedef signed long long    int64;
#endif 

// TODO: maybe replace pointer_size with using pointer_size = uintptr_t?
typedef Select<sizeof(void*) == 4, uint32, uint64>::Result pointer_size;

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

typedef char        char8;
typedef wchar_t     char16;

typedef float       float32;
typedef double      float64;

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
// FIX: replace DefaultSTLAllocator with MemoryManagerAllocator after fixing framework and game codebases
template<typename T>
using DefaultSTLAllocator = std::allocator<T>;
//using DefaultSTLAllocator = MemoryManagerAllocator<T, ALLOC_POOL_APP>;
#else
template<typename T>
using DefaultSTLAllocator = std::allocator<T>;
#endif

template<typename CharT>
using BasicString = std::basic_string<CharT, std::char_traits<CharT>, DefaultSTLAllocator<CharT>>;

using String = BasicString<char8>;
using WideString = BasicString<wchar_t>;

template<typename CharT>
using BasicStringStream = std::basic_stringstream<CharT, std::char_traits<CharT>, DefaultSTLAllocator<CharT>>;

using StringStream = BasicStringStream<char8>;

template<typename T>
using List = std::list<T, DefaultSTLAllocator<T>>;

template<typename T>
using Vector = std::vector<T, DefaultSTLAllocator<T>>;

template<typename T>
using Deque = std::deque<T, DefaultSTLAllocator<T>>;

template <class _Key,
          class _Compare = std::less<_Key>>
using Set = std::set< _Key, _Compare, DefaultSTLAllocator<_Key>>;
    
template<class _Kty,
         class _Ty,
         class _Pr = std::less<_Kty>>
using Map = std::map<_Kty, _Ty, _Pr, DefaultSTLAllocator<std::pair<const _Kty, _Ty>>>;

template<class _Kty,
         class _Ty,
         class _Pr = std::less<_Kty>>
using MultiMap = std::multimap<_Kty, _Ty, _Pr, DefaultSTLAllocator<std::pair<const _Kty, _Ty>>>;

template<class T,
         class Container = Deque<T>>
using Stack = std::stack<T, Container>;

template<class T,
         class Container = Vector<T>,
         class Compare = std::less<typename Container::value_type>>
using PriorityQueue = std::priority_queue<T, Container, Compare>;

template<typename Key,
         typename Hash = std::hash<Key>,
         typename KeyEqual = std::equal_to<Key>>
using UnorderedSet = std::unordered_set<Key, Hash, KeyEqual, DefaultSTLAllocator<Key>>;

template<typename Key,
         typename T,
         typename Hash = std::hash<Key>,
         typename KeyEqual = std::equal_to<Key>>
using UnorderedMap = std::unordered_map<Key, T, Hash, KeyEqual, DefaultSTLAllocator<std::pair<const Key, T>>>;

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

template <class T>
inline T Min(T a, T b)
{
    return (a < b) ? (a) : (b);
}

template <class T>
inline T Max(T a, T b)
{
    return (a > b) ? (a) : (b);
}

template <class T>
inline T Abs(T a)
{
    return (a >= 0) ? (a) : (-a);
}

template <class T>
inline T Clamp(T val, T a, T b)
{
    return Min(b, Max(val, a));
}

#if defined(__DAVAENGINE_WIN32__)
#define Snprintf    _snprintf
#else //#if defined(__DAVAENGINE_WIN32__)
#define Snprintf    snprintf
#endif //#if defined(__DAVAENGINE_WIN32__)

#define Memcmp memcmp
#define Memcpy memcpy
#define Memset memset
#define Memmove memmove

template <class TYPE>
void SafeDelete(TYPE * &d)
{
    if (d != nullptr)
    {
        delete d;
        d = nullptr;
    }
}

template <class TYPE>
void SafeDeleteArray(TYPE * & d)
{
    if (d != nullptr)
    {
        delete [] d;
        d = nullptr;
    }
}

#ifndef SAFE_DELETE // for compatibility with FCollada
#define SAFE_DELETE(x) if (x) { delete x; x = nullptr; };
#endif 

#ifndef SAFE_DELETE_ARRAY // for compatibility with FCollada
#define SAFE_DELETE_ARRAY(x) if (x) { delete [] x; x = nullptr; };
#endif

#ifndef OBJC_SAFE_RELEASE
#define OBJC_SAFE_RELEASE(x) [x release];x = nil;
#endif 

/**
 \enum Graphical object aligment.
*/
enum eAlign 
{
    ALIGN_LEFT      = 0x01, //!<Align graphical object by the left side.
    ALIGN_HCENTER   = 0x02, //!<Align graphical object by the horizontal center.
    ALIGN_RIGHT     = 0x04, //!<Align graphical object by the right side.
    ALIGN_TOP       = 0x08, //!<Align graphical object by the top side.
    ALIGN_VCENTER   = 0x10, //!<Align graphical object by the vertical center.
    ALIGN_BOTTOM    = 0x20, //!<Align graphical object by the bottom side.
    ALIGN_HJUSTIFY  = 0x40  //!<Used only for the fonts. Stretch font string over all horizontal size of the area.
};

#ifndef COUNT_OF
#define COUNT_OF(x) (sizeof(x)/sizeof(*x))
#endif
    
#ifndef REMOVE_IN_RELEASE
    #if defined(__DAVAENGINE_DEBUG__)
        #define REMOVE_IN_RELEASE (x) x
    #else
        #define REMOVE_IN_RELEASE (x) 
    #endif
#endif

    
//#if defined(__DAVAENGINE_IPHONE__)
#ifdef __thumb__
//#error "This file should be compiled in ARM mode only."
    // Note in Xcode, right click file, Get Info->Build, Other compiler flags = "-marm"
#endif
//#endif//#if !defined(__DAVAENGINE_ANDROID__)


#ifndef DAVAENGINE_HIDE_DEPRECATED
#ifdef __GNUC__
#define DAVA_DEPRECATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define DAVA_DEPRECATED(func) __declspec(deprecated) func
#else
#pragma message("WARNING: You need to implement DAVA_DEPRECATED for this compiler")
#define DAVA_DEPRECATED(func) func
#endif
#else
#define DAVA_DEPRECATED(func) func
#endif //DAVAENGINE_HIDE_DEPRECATED
    
enum eErrorCode
{
    SUCCESS,
    ERROR_FILE_FORMAT_INCORRECT,
    ERROR_FILE_NOTFOUND,
    ERROR_READ_FAIL,
    ERROR_WRITE_FAIL
};

};

#endif


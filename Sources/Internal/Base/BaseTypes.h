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


#ifndef __DAVAENGINE_BASETYPES_H__
#define __DAVAENGINE_BASETYPES_H__

#include "Platform/PlatformDetection.h"
#include "Base/TemplateHelpers.h"

#include <memory>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <set>
#include <unordered_set>
#include <stack>
#include <queue>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <cerrno>

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
#include "MemoryManager/AllocPools.h"
#include "MemoryManager/MemoryManagerAllocator.h"
#endif

namespace DAVA
{

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

//Platform-independent signed and unsigned integer type
//Always has a size equal to pointer size (4 bytes in x86, 8 in x64)
typedef ptrdiff_t int_t;
typedef size_t    uint_t;

typedef uint_t pointer_size;

typedef char        char8;
typedef wchar_t     char16;

typedef float       float32;
typedef double      float64;

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

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

template< class T, 
          std::size_t N > 
using Array = std::array<T, N>;

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

template <typename T1, typename T2>
DAVA_CONSTEXPR auto Min(const T1 &a, const T2 &b) -> decltype(a < b ? a : b)
{
    return a < b ? a : b;
}

template <typename T1, typename T2>
DAVA_CONSTEXPR auto Max(const T1 &a, const T2 &b) -> decltype(a > b ? a : b)
{
    return a > b ? a : b;
}

template <typename T>
DAVA_CONSTEXPR auto Abs(const T &a) -> decltype(a >= 0 ? a : -a)
{
    return a >= 0 ? a : -a;
}

template <typename T1, typename T2, typename T3>
DAVA_CONSTEXPR auto Clamp(const T1 &val, const T2 &a, const T3 &b) -> decltype(Min(b, Max(val, a)))
{
    return Min(b, Max(val, a));
}

#if defined(__DAVAENGINE_WINDOWS__)
#define Snprintf    _snprintf
#else
#define Snprintf    snprintf
#endif

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

template <typename T, uint_t N>
DAVA_CONSTEXPR uint_t COUNT_OF(T(&)[N]) { return N; }
    
#ifndef REMOVE_IN_RELEASE
    #if defined(__DAVAENGINE_DEBUG__)
        #define REMOVE_IN_RELEASE (x) x
    #else
        #define REMOVE_IN_RELEASE (x) 
    #endif
#endif

enum eErrorCode
{
    SUCCESS,
    ERROR_FILE_FORMAT_INCORRECT,
    ERROR_FILE_NOTFOUND,
    ERROR_READ_FAIL,
    ERROR_WRITE_FAIL
};

};

#endif  // __DAVAENGINE_BASETYPES_H__
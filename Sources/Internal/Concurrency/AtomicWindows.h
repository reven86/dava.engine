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


#ifndef __DAVAENGINE_ATOMIC_WINDOWS_H__
#define __DAVAENGINE_ATOMIC_WINDOWS_H__

#include "Base/Platform.h"
#ifndef USE_CPP11_CONCURRENCY

#include <intrin.h>
#include "Concurrency/Atomic.h"

namespace DAVA
{

#if defined(__DAVAENGINE_WINDOWS__)

//-----------------------------------------------------------------------------
//Atomic template class realization using native Windows functions
//-----------------------------------------------------------------------------
namespace Detail
{

template <size_t N>
struct TypeSelector;

template <>
struct TypeSelector<sizeof(CHAR)> { using Type = CHAR; };
template <>
struct TypeSelector<sizeof(SHORT)> { using Type = SHORT; };
template <>
struct TypeSelector<sizeof(LONG)> { using Type = LONG; };
template <>
struct TypeSelector<sizeof(LONGLONG)> { using Type = LONGLONG; };

//atomic increment overloads
inline CHAR AtomicIncrement(volatile CHAR* value)
{
    CHAR initial = ::_InterlockedExchangeAdd8(value, 1);
    return initial + 1;

}
inline SHORT AtomicIncrement(volatile SHORT* value)
{
    return ::_InterlockedIncrement16(value);
}
inline LONG AtomicIncrement(volatile LONG* value)
{
    return ::_InterlockedIncrement(value);
}
inline LONGLONG AtomicIncrement(volatile LONGLONG* value)
{
    return ::_InterlockedIncrement64(value);
}

//atomic decrement overloads 
inline CHAR AtomicDecrement(volatile CHAR* value)
{
    CHAR initial = ::_InterlockedExchangeAdd8(value, -1);
    return initial - 1;
}
inline SHORT AtomicDecrement(volatile SHORT* value)
{
    return ::_InterlockedDecrement16(value);
}
inline LONG AtomicDecrement(volatile LONG* value)
{
    return ::_InterlockedDecrement(value);
}
inline LONGLONG AtomicDecrement(volatile LONGLONG* value)
{
    return ::_InterlockedDecrement64(value);
}

//atomic swap overloads 
inline CHAR AtomicSwap(volatile CHAR* target, CHAR desired)
{
    return ::_InterlockedExchange8(target, desired);
}
inline SHORT AtomicSwap(volatile SHORT* target, SHORT desired)
{
    return ::_InterlockedExchange16(target, desired);
}
inline LONG AtomicSwap(volatile LONG* target, LONG desired)
{
    return ::_InterlockedExchange(target, desired);
}
inline LONGLONG AtomicSwap(volatile LONGLONG* target, LONGLONG desired)
{
    return ::_InterlockedExchange64(target, desired);
}

//atomic cas overloads
inline CHAR AtomicCAS(volatile CHAR* target, CHAR expected, CHAR desired)
{
    return ::_InterlockedCompareExchange8(target, desired, expected);
}
inline SHORT AtomicCAS(volatile SHORT* target, SHORT expected, SHORT desired)
{
    return ::_InterlockedCompareExchange16(target, desired, expected);
}
inline LONG AtomicCAS(volatile LONG* target, LONG expected, LONG desired)
{
    return ::_InterlockedCompareExchange(target, desired, expected);
}
inline LONGLONG AtomicCAS(volatile LONGLONG* target, LONGLONG expected, LONGLONG desired)
{
    return ::_InterlockedCompareExchange64(target, desired, expected);
}

} //  namespace Detail

template <typename T>
void Atomic<T>::Set(T val) DAVA_NOEXCEPT
{
    Swap(val);
}

template <typename T>
T Atomic<T>::Get() const DAVA_NOEXCEPT
{
    T val = value;
    MemoryBarrier();
    return val;
}

template <typename T>
T Atomic<T>::GetRelaxed() const DAVA_NOEXCEPT
{
    volatile T val = value;
    return val;
}

template <typename T>
T Atomic<T>::Increment() DAVA_NOEXCEPT
{
    using Type = typename Detail::TypeSelector<sizeof(T)>::Type;
    Type* val = reinterpret_cast<Type*>(&value);

    T result = Cast(Detail::AtomicIncrement(val));
    return result;
}

template <typename T>
T Atomic<T>::Decrement() DAVA_NOEXCEPT
{
    using Type = typename Detail::TypeSelector<sizeof(T)>::Type;
    Type* val = reinterpret_cast<Type*>(&value);

    T result = Cast(Detail::AtomicDecrement(val));
    return result;
}

template <typename T>
T Atomic<T>::Swap(T desired) DAVA_NOEXCEPT
{
    using Type = typename Detail::TypeSelector<sizeof(T)>::Type;
    Type* val = reinterpret_cast<Type*>(&value);
    Type des = static_cast<Type>(desired);

    T result = Cast(Detail::AtomicSwap(val, des));
    return result;
}

template <typename T>
bool Atomic<T>::CompareAndSwap(T expected, T desired) DAVA_NOEXCEPT
{
    using Type = typename Detail::TypeSelector<sizeof(T)>::Type;
    Type* val = reinterpret_cast<Type*>(&value);
    Type exp = static_cast<Type>(expected);
    Type des = static_cast<Type>(desired);

    T result = Cast(Detail::AtomicCAS(val, exp, des));
    return result == expected;
}

template <typename T>
template <typename Y> 
T Atomic<T>::Cast(Y val)
{
    return static_cast<T>(val);
}

template <>
template <typename Y>
bool Atomic<bool>::Cast(Y val)
{
    return val != 0;
}


#endif //  __DAVAENGINE_WINDOWS__

} //  namespace DAVA

#endif //  !USE_CPP11_CONCURRENCY
#endif //  __DAVAENGINE_ATOMIC_WINDOWS_H__
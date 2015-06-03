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
struct TypeSelector<sizeof(SHORT)> { using Type = SHORT; };
template <>
struct TypeSelector<sizeof(LONG)> { using Type = LONG; };
template <>
struct TypeSelector<sizeof(LONGLONG)> { using Type = LONGLONG; };

//atomic increment overloads 
inline SHORT AtomicIncrement(SHORT* value)
{
    return ::InterlockedIncrement16(value);
}
inline LONG AtomicIncrement(LONG* value)
{
    return ::InterlockedIncrement(value);
}
inline LONGLONG AtomicIncrement(LONGLONG* value)
{
    return ::InterlockedIncrement64(value);
}

//atomic decrement overloads 
inline short AtomicDecrement(short* value)
{
    return ::InterlockedDecrement16(value);
}
inline LONG AtomicDecrement(LONG* value)
{
    return ::InterlockedDecrement(value);
}
inline LONGLONG AtomicDecrement(LONGLONG* value)
{
    return ::InterlockedDecrement64(value);
}

//atomic swap overloads 
inline SHORT AtomicSwap(SHORT* target, SHORT desired)
{
    return ::InterlockedExchange16(target, desired);
}
inline LONG AtomicSwap(LONG* target, LONG desired)
{
    return ::InterlockedExchange(target, desired);
}
inline LONGLONG AtomicSwap(LONGLONG* target, LONGLONG desired)
{
    return ::InterlockedExchange64(target, desired);
}

//atomic cas overloads
inline SHORT AtomicCAS(SHORT* target, SHORT expected, SHORT desired)
{
    return ::InterlockedCompareExchange16(target, desired, expected);
}
inline LONG AtomicCAS(LONG* target, LONG expected, LONG desired)
{
    return ::InterlockedCompareExchange(target, desired, expected);
}
inline LONGLONG AtomicCAS(LONGLONG* target, LONGLONG expected, LONGLONG desired)
{
    return ::InterlockedCompareExchange64(target, desired, expected);
}

} //  namespace Detail

template <typename T>
Atomic<T>::Atomic(T val) DAVA_NOEXCEPT : value(val) {}

template <typename T>
void Atomic<T>::Set(T val) DAVA_NOEXCEPT
{
    Swap(val);
}

template <typename T>
T Atomic<T>::Get() const DAVA_NOEXCEPT
{
    static_assert(sizeof(T) <= sizeof(void*), "Unsuppored type");
    T val = value;
    MemoryBarrier();
    return val;
}

template <typename T>
T Atomic<T>::Increment() DAVA_NOEXCEPT
{
    using Type = Detail::TypeSelector<sizeof(T)>::Type;
    Type* val = reinterpret_cast<Type*>(&value);

    T result = static_cast<T>(Detail::AtomicIncrement(val));
    return result;
}

template <typename T>
T Atomic<T>::Decrement() DAVA_NOEXCEPT
{
    using Type = Detail::TypeSelector<sizeof(T)>::Type;
    Type* val = reinterpret_cast<Type*>(&value);

    T result = static_cast<T>(Detail::AtomicDecrement(val));
    return result;
}

template <typename T>
T Atomic<T>::Swap(T desired) DAVA_NOEXCEPT
{
    using Type = Detail::TypeSelector<sizeof(T)>::Type;
    Type* val = reinterpret_cast<Type*>(&value);
    Type des = static_cast<Type>(desired);

    T result = static_cast<T>(Detail::AtomicSwap(val, des));
    return result;
}

template <typename T>
bool Atomic<T>::CompareAndSwap(T expected, T desired) DAVA_NOEXCEPT
{
    using Type = Detail::TypeSelector<sizeof(T)>::Type;
    Type* val = reinterpret_cast<Type*>(&value);
    Type exp = static_cast<Type>(expected);
    Type des = static_cast<Type>(desired);

    T result = static_cast<T>(Detail::AtomicCAS(val, exp, des));
    return result == expected;
}

#endif //  __DAVAENGINE_WINDOWS__

} //  namespace DAVA

#endif //  !USE_CPP11_CONCURRENCY
#endif //  __DAVAENGINE_ATOMIC_WINDOWS_H__
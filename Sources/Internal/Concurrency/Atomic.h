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


#ifndef __DAVAENGINE_ATOMIC_H__
#define __DAVAENGINE_ATOMIC_H__

#include <type_traits>
#include "Base/Platform.h"

#ifdef USE_CPP11_CONCURRENCY
#   include <atomic> //for std::atomic
#endif

namespace DAVA
{

//-----------------------------------------------------------------------------
//Atomic template class. 
//-----------------------------------------------------------------------------
template <typename T>
class Atomic
{
    static_assert(std::is_integral<T>::value || 
                  std::is_pointer<T>::value  ||
                  std::is_enum<T>::value, 
                  "Not valid type for atomic operations");
public:
    Atomic(T val = T()) DAVA_NOEXCEPT : value(val) {}

    Atomic(const Atomic& other) = delete;
    Atomic& operator=(const Atomic& other) = delete;

    Atomic& operator=(T val) DAVA_NOEXCEPT;
    operator T() DAVA_NOEXCEPT { return Get(); }

    void Set(T val) DAVA_NOEXCEPT;
    T Get() const DAVA_NOEXCEPT;
    T GetRelaxed() const DAVA_NOEXCEPT;

    T Increment() DAVA_NOEXCEPT;
    T Decrement() DAVA_NOEXCEPT;

    T operator++() DAVA_NOEXCEPT { return Increment(); }
    T operator++(int) DAVA_NOEXCEPT { return Increment() - 1; }
    T operator--() DAVA_NOEXCEPT { return Decrement(); }
    T operator--(int) DAVA_NOEXCEPT { return Decrement() + 1; }

    T Swap(T desired) DAVA_NOEXCEPT;
    bool CompareAndSwap(T expected, T desired) DAVA_NOEXCEPT;

private:

#ifdef USE_CPP11_CONCURRENCY
    std::atomic<T> value;
#else

#   ifdef __DAVAENGINE_WINDOWS__
    template <typename Y> T Cast(Y val);
    T value;
#   else
    DAVA_ALIGNED(T value, sizeof(T));
#   endif

#endif
};

//-----------------------------------------------------------------------------
//Common realization
//-----------------------------------------------------------------------------
template <typename T>
Atomic<T>& Atomic<T>::operator=(T val) DAVA_NOEXCEPT
{
    Set(val);
    return *this;
}

} //  namespace DAVA

//-----------------------------------------------------------------------------
//Specific platform realization
//-----------------------------------------------------------------------------
#if defined (__DAVAENGINE_WINDOWS__) && !defined(USE_CPP11_CONCURRENCY)
#   include "Concurrency/AtomicWindows.h"
#elif defined (__GNUC__) && !defined(USE_CPP11_CONCURRENCY)
#   include "Concurrency/AtomicGNU.h"
#elif defined(USE_CPP11_CONCURRENCY)

namespace DAVA
{

//-----------------------------------------------------------------------------
//Atomic template class realization using std::atomic
//-----------------------------------------------------------------------------
template <typename T>
void Atomic<T>::Set(T val) DAVA_NOEXCEPT
{
    value = val; 
}

template <typename T>
T Atomic<T>::Get() const DAVA_NOEXCEPT
{
    return value;
}

template <typename T>
T Atomic<T>::GetRelaxed() const DAVA_NOEXCEPT
{
    return value.load(std::memory_order_relaxed);
}

template <typename T>
T Atomic<T>::Increment() DAVA_NOEXCEPT
{
    return ++value;
}

template <typename T>
T Atomic<T>::Decrement() DAVA_NOEXCEPT
{
    return --value;
}

template <typename T>
T Atomic<T>::Swap(T desired) DAVA_NOEXCEPT
{
    return value.exchange(desired);
}

template <typename T>
bool Atomic<T>::CompareAndSwap(T expected, T desired) DAVA_NOEXCEPT
{
    return value.compare_exchange_strong(expected, desired);
}

} //  namespace DAVA

#endif

#endif //  __DAVAENGINE_ATOMIC_H__
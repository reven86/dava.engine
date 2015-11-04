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


#ifndef __DAVAENGINE_ATOMIC_GNU_H__
#define __DAVAENGINE_ATOMIC_GNU_H__

#include "Base/Platform.h"
#ifndef USE_CPP11_CONCURRENCY

#include "Concurrency/Atomic.h"

namespace DAVA
{

#if defined(__GNUC__)

//-----------------------------------------------------------------------------
//Atomic template class realization using built-in intrisics
//-----------------------------------------------------------------------------
template <typename T>
void Atomic<T>::Set(T val) DAVA_NOEXCEPT
{
    __atomic_store(&value, &val, __ATOMIC_SEQ_CST);
}

template <typename T>
T Atomic<T>::Get() const DAVA_NOEXCEPT
{
    return __atomic_load_n(&value, __ATOMIC_SEQ_CST);
}

template <typename T>
T Atomic<T>::GetRelaxed() const DAVA_NOEXCEPT
{
    return __atomic_load_n(&value, __ATOMIC_RELAXED);
}

template <typename T>
T Atomic<T>::Increment() DAVA_NOEXCEPT
{
    return __sync_add_and_fetch(&value, 1);
}

template <typename T>
T Atomic<T>::Decrement() DAVA_NOEXCEPT
{
    return __sync_sub_and_fetch(&value, 1);
}

template <typename T>
T Atomic<T>::Swap(T desired) DAVA_NOEXCEPT
{
    return __atomic_exchange_n(&value, desired, __ATOMIC_SEQ_CST);
}

template <typename T>
bool Atomic<T>::CompareAndSwap(T expected, T desired) DAVA_NOEXCEPT
{
    return __atomic_compare_exchange(&value, &expected, &desired,
                                     false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

#endif //  __GNUC__

} //  namespace DAVA

#endif //  !USE_CPP11_CONCURRENCY
#endif //  __DAVAENGINE_ATOMIC_GNU_H__

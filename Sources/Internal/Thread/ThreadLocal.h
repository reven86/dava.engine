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

#ifndef __DAVAENGINE_THREADLOCAL_H__
#define __DAVAENGINE_THREADLOCAL_H__

#include <type_traits>

#include "Base/BaseTypes.h"

namespace DAVA
{

/*
    template class ThreadLocal - implementation of cross-platform thread local storage (TLS) variables of specified type.
    Specified type can be pointer or fundamental type (int, char, etc). Size of type cannot exceed pointer size.

    C++ 11 supports thread_local keyword which does the same thing but not all compilers support it.

    Restrictions:
        variables of type ThreadLocal can have only static storage duration (global or local static, and static data member)
        if you declare ThreadLocal as automatic object it's your own problems, so don't cry: Houston, we've got a problem
*/
template<typename T>
class ThreadLocal final
{
    static_assert(sizeof(T) <= sizeof(void*) && (std::is_fundamental<T>::value || std::is_pointer<T>::value), "ThreadLocal supports only fundamental and pointer types of no more than pointer size");

#if defined(__DAVAENGINE_WIN32__)
    using KeyType = DWORD;
#else
    using KeyType = pthread_key_t;
#endif

public:
    ThreadLocal() DAVA_NOEXCEPT;
    ~ThreadLocal() DAVA_NOEXCEPT;

    ThreadLocal(const ThreadLocal&) = delete;
    ThreadLocal& operator = (const ThreadLocal&) = delete;
    ThreadLocal(ThreadLocal&&) = delete;
    ThreadLocal& operator = (ThreadLocal&&) = delete;

    ThreadLocal& operator = (const T value) DAVA_NOEXCEPT;
    operator T () const DAVA_NOEXCEPT;

private:
    void CreateTlsKey() DAVA_NOEXCEPT;
    void DeleteTlsKey() const DAVA_NOEXCEPT;
    void SetTlsValue(T value) const DAVA_NOEXCEPT;
    T GetTlsValue() const DAVA_NOEXCEPT;

private:
    KeyType key;
};

//////////////////////////////////////////////////////////////////////////
template<typename T>
inline ThreadLocal<T>::ThreadLocal() DAVA_NOEXCEPT
{
    CreateTlsKey();
}

template<typename T>
inline ThreadLocal<T>::~ThreadLocal() DAVA_NOEXCEPT
{
    DeleteTlsKey();
}

template<typename T>
inline ThreadLocal<T>& ThreadLocal<T>::operator = (const T value) DAVA_NOEXCEPT
{
    SetTlsValue(value);
    return *this;
}

template<typename T>
inline ThreadLocal<T>::operator T () const DAVA_NOEXCEPT
{
    return GetTlsValue();
}

// Win32 specific implementation
#if defined(__DAVAENGINE_WIN32__)

template<typename T>
inline void ThreadLocal<T>::CreateTlsKey() DAVA_NOEXCEPT
{
    key = TlsAlloc();
}

template<typename T>
inline void ThreadLocal<T>::DeleteTlsKey() const DAVA_NOEXCEPT
{
    TlsFree(key);
}

template<typename T>
inline void ThreadLocal<T>::SetTlsValue(T value) const DAVA_NOEXCEPT
{
    TlsSetValue(key, reinterpret_cast<LPVOID>(value));
}

template<typename T>
inline T ThreadLocal<T>::GetTlsValue() const DAVA_NOEXCEPT
{
    return reinterpret_cast<T>(TlsGetValue(key));
}

#else   // POSIX specific implementation

template<typename T>
inline void ThreadLocal<T>::CreateTlsKey() DAVA_NOEXCEPT
{
    key = ...;
}

template<typename T>
inline void ThreadLocal<T>::DeleteTlsKey() const DAVA_NOEXCEPT
{
    
}

template<typename T>
inline void ThreadLocal<T>::SetTlsValue(T value) const DAVA_NOEXCEPT
{
    
}

template<typename T>
inline T ThreadLocal<T>::GetTlsValue() const DAVA_NOEXCEPT
{
    return ...;
}

#endif
}

#endif  // __DAVAENGINE_THREADLOCAL_H__

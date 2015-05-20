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

#ifndef __DAVAENGINE_INTERNALALLOCATOR_H__
#define __DAVAENGINE_INTERNALALLOCATOR_H__

#include "Base/BaseTypes.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include <cstddef>
#include <limits>

#include "MemoryManager/AllocatorBridge.h"

namespace DAVA
{

template<typename T>
class InternalAllocator final : public std::allocator<T>
{
public:
    typedef T value_type;

    template<typename U> struct rebind { typedef InternalAllocator<U> other; };

    InternalAllocator() {}
    InternalAllocator(const InternalAllocator<T>&) = default;
    template <typename U>
    InternalAllocator(const InternalAllocator<U>&) {}

    InternalAllocator<T>& operator = (const InternalAllocator<T>&) = default;
    template <typename Other>
    InternalAllocator<T>& operator = (const InternalAllocator<Other>&) DAVA_NOEXCEPT {return *this;};
    
    ~InternalAllocator() = default;

    T* allocate(size_t n)
    {
        void* ptr = Memory::InternalAlloc(n * sizeof(T));
        if (ptr != nullptr)
        {
            return static_cast<T*>(ptr);
        }
        throw std::bad_alloc();
    }
    void deallocate(void* ptr, size_t)
    {
        Memory::InternalDealloc(ptr);
    }
};

#if 0
template<typename T>
class InternalAllocator final
{
public:
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    template <typename U>
    struct rebind
    {
        typedef InternalAllocator<U> other;
    };

    InternalAllocator() = default;
    InternalAllocator(const InternalAllocator&) = default;
    InternalAllocator& operator = (const InternalAllocator&) = delete;
    template <typename U>
    InternalAllocator(const InternalAllocator<U>&) {}
    ~InternalAllocator() = default;

    T* address(T& ref) const;
    const T* address(const T& cref) const;
    size_t max_size() const;

    bool operator == (const InternalAllocator& other) const;
    bool operator != (const InternalAllocator& other) const;

    void construct(T* const ptr, const T& obj) const;
    template<typename U, typename... Args>
    void construct(U* ptr, Args&&... args);
    void destroy(T* const ptr) const;

    T* allocate(const size_t n) const;
    void deallocate(T* const ptr, const size_t n) const;

    template <typename U>
    T* allocate(const size_t n, const U* /* const hint */) const;
};

//////////////////////////////////////////////////////////////////////////

template<typename T>
inline T* InternalAllocator<T>::address(T& ref) const
{
    return &ref;
}

template<typename T>
inline const T* InternalAllocator<T>::address(const T& cref) const
{
    return &cref;
}

template<typename T>
inline size_t InternalAllocator<T>::max_size() const
{
    return std::numeric_limits<size_t>::max() / sizeof(T);
}

template<typename T>
inline bool InternalAllocator<T>::operator == (const InternalAllocator& other) const
{
    return true;
}

template<typename T>
inline bool InternalAllocator<T>::operator != (const InternalAllocator& other) const
{
    return !(*this == other);
}

template<typename T>
inline void InternalAllocator<T>::construct(T* const ptr, const T& obj) const
{
    void* const buf = static_cast<void*>(ptr);
    new (buf) T(obj);
}

template<typename T>
template<typename U, typename... Args>
inline void InternalAllocator<T>::construct(U* ptr, Args&&... args)
{
    void* buf = static_cast<void*>(ptr);
    new (buf) U(std::forward<Args>(args)...);
}

template<typename T>
inline void InternalAllocator<T>::destroy(T* const ptr) const
{
    ptr->~T();
}

template<typename T>
inline T* InternalAllocator<T>::allocate(const size_t n) const
{
    void* ptr = InternalAlloc(n * sizeof(T));
    if (nullptr == ptr)
    {
        throw std::bad_alloc();
    }
    return static_cast<T*>(ptr);
}

template<typename T>
inline void InternalAllocator<T>::deallocate(T* const ptr, const size_t n) const
{
    InternalDealloc(ptr);
}

template<typename T>
template <typename U>
inline T* InternalAllocator<T>::allocate(const size_t n, const U*) const
{
    return allocate(n);
}
#endif

}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)

#endif  // __DAVAENGINE_INTERNALALLOCATOR_H__

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


#ifndef __DAVAENGINE_POOL_ALLOCATOR_H__
#define __DAVAENGINE_POOL_ALLOCATOR_H__

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"

// The following headers are required for all allocators.
#include <stddef.h>  // Required for size_t and ptrdiff_t and NULL
#include <new>       // Required for placement new and std::bad_alloc
#include <stdexcept> // Required for std::length_error

// The following headers contain stuff that Mallocator uses.
#include <stdlib.h>  // For malloc() and free()
#include <iostream>  // For std::cout
#include <ostream>   // For std::endl

namespace DAVA
{
    
#ifdef new
#undef new
#endif
    
template <typename T> 
class Mallocator 
{
public:
    
    // The following will be the same for virtually all allocators.
    typedef T * pointer;
    typedef const T * const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    
    T * address(T& r) const {
        return &r;
    }
    
    const T * address(const T& s) const {
        return &s;
    }
    
    size_t max_size() const {
        // The following has been carefully written to be independent of
        // the definition of size_t and to avoid signed/unsigned warnings.
        return (static_cast<size_t>(0) - static_cast<size_t>(1)) / sizeof(T);
    }
    
    
    // The following must be the same for all allocators.
    template <typename U> struct rebind {
        typedef Mallocator<U> other;
    };
    
    bool operator!=(const Mallocator& other) const {
        return !(*this == other);
    }
    
    void construct(T * const p, const T& t) const {
        void * const pv = static_cast<void *>(p);
        
        new (pv) T(t);
    }
    
    void destroy(T * const p) const; // Defined below.
    
    
    // Returns true if and only if storage allocated from *this
    // can be deallocated from other, and vice versa.
    // Always returns true for stateless allocators.
    bool operator==(const Mallocator& other) const {
        return true;
    }
    
    
    // Default constructor, copy constructor, rebinding constructor, and destructor.
    // Empty for stateless allocators.
    Mallocator() { }
    
    Mallocator(const Mallocator&) { }
    
    template <typename U> Mallocator(const Mallocator<U>&) { }
    
    ~Mallocator() { }
    
    
    // The following will be different for each allocator.
    T * allocate(const size_t n) const {
        // Mallocator prints a diagnostic message to demonstrate
        // what it's doing. Real allocators won't do this.
//        std::cout << "Allocating " << n << (n == 1 ? " object" : "objects")
//        << " of size " << sizeof(T) << "." << std::endl;
        
        // The return value of allocate(0) is unspecified.
        // Mallocator returns NULL in order to avoid depending
        // on malloc(0)'s implementation-defined behavior
        // (the implementation can define malloc(0) to return NULL,
        // in which case the bad_alloc check below would fire).
        // All allocators can return NULL in this case.
        if (n == 0) {
            return NULL;
        }
        
        // All allocators should contain an integer overflow check.
        // The Standardization Committee recommends that std::length_error
        // be thrown in the case of integer overflow.
        if (n > max_size()) {
            throw std::length_error("Mallocator<T>::allocate() - Integer overflow.");
        }
        
        // Mallocator wraps malloc().
        void * const pv = malloc(n * sizeof(T));
        
        // Allocators should throw std::bad_alloc in the case of memory allocation failure.
        if (pv == NULL) {
            throw std::bad_alloc();
        }
        
        return static_cast<T *>(pv);
    }
    
    void deallocate(T * const p, const size_t n) const {
//        // Mallocator prints a diagnostic message to demonstrate
//        // what it's doing. Real allocators won't do this.
//        std::cout << "Deallocating " << n << (n == 1 ? " object" : "objects")
//        << " of size " << sizeof(T) << "." << std::endl;
        
        // Mallocator wraps free().
        free(p);
    }
    
    
    // The following will be the same for all allocators that ignore hints.
    template <typename U> T * allocate(const size_t n, const U * /* const hint */) const {
        return allocate(n);
    }
    
    
    // Allocators are not required to be assignable, so
    // all allocators should have a private unimplemented
    // assignment operator. Note that this will trigger the
    // off-by-default (enabled under /Wall) warning C4626
    // "assignment operator could not be generated because a
    // base class assignment operator is inaccessible" within
    // the STL headers, but that warning is useless.
private:
    Mallocator& operator=(const Mallocator&);
};
    
// A compiler bug causes it to believe that p->~T() doesn't reference p.
    
//#ifdef _MSC_VER
//#pragma warning(push)
//#pragma warning(disable: 4100) // unreferenced formal parameter
//#endif
    
// The definition of destroy() must be the same for all allocators.
template <typename T> void Mallocator<T>::destroy(T * const p) const 
{
    p->~T();
}
    
//#ifdef _MSC_VER
//#pragma warning(pop)
//#endif
    
};

#endif // __DAVAENGINE_FIXED_SIZE_POOL_ALLOCATOR_H__
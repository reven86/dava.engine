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

#ifndef __DAVAENGINE_MEMORYPROFILER_H__
#define __DAVAENGINE_MEMORYPROFILER_H__

#include "Base/BaseTypes.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include "MemoryManager.h"

#define MEMORY_PROFILER_REGISTER_TAG(index, name)           DAVA::MemoryManager::RegisterTagName(index, name)
#define MEMORY_PROFILER_REGISTER_ALLOC_POOL(index, name)    DAVA::MemoryManager::RegisterAllocPoolName(index, name)

#define MEMORY_PROFILER_ENTER_TAG(tag)                      DAVA::MemoryManager::Instance()->EnterTagScope(tag)
#define MEMORY_PROFILER_LEAVE_TAG(tag)                      DAVA::MemoryManager::Instance()->LeaveTagScope(tag)

#define ENABLE_CLASS_ALLOCATION_TRACKING(allocPool)                                                             \
private:                                                                                                        \
    template<typename T> static T* TrackedNew() {                                                               \
        void* buf = DAVA::MemoryManager::Instance()->Allocate(sizeof(T), allocPool);                            \
        return new (buf) T;                                                                                     \
    }                                                                                                           \
    template<typename T, typename... Args> static T* TrackedNew(Args&&... args) {                               \
        void* buf = DAVA::MemoryManager::Instance()->Allocate(sizeof(T), allocPool);                            \
        return new (buf) T(std::forward<Args...>(args...));                                                     \
    }                                                                                                           \
    template<typename T> static T* TrackedNewArray(size_t n) {                                                  \
        void* buf = DAVA::MemoryManager::Instance()->Allocate(sizeof(T) * n, allocPool);                        \
        return new (buf) T[n];                                                                                  \
    }                                                                                                           \
public:                                                                                                         \
    static void* operator new (size_t size) { return MemoryManager::Instance()->Allocate(size, allocPool); }    \
    static void* operator new [](size_t size) { return MemoryManager::Instance()->Allocate(size, allocPool); }  \
    static void operator delete (void* ptr) DAVA_NOEXCEPT { MemoryManager::Instance()->Deallocate(ptr); }       \
    static void operator delete [](void* ptr) DAVA_NOEXCEPT { MemoryManager::Instance()->Deallocate(ptr); }     \
    static void* operator new (size_t size, void* place) DAVA_NOEXCEPT{ return place; }                         \
    static void* operator new [](size_t size, void* place) DAVA_NOEXCEPT{ return place; }                       \
    static void operator delete (void* ptr, void* place) DAVA_NOEXCEPT {}                                       \
    static void operator delete [] (void* ptr, void* place) DAVA_NOEXCEPT {}                                    \

#define TRACKED_NEW(type, ...)      TrackedNew<type>(##__VA_ARGS__)
#define TRACKED_NEW_ARRAY(type, n)  TrackedNewArray<type>(n)

#else   // defined(DAVA_MEMORY_PROFILING_ENABLE)

#define MEMORY_PROFILER_REGISTER_TAG(index, name)
#define MEMORY_PROFILER_REGISTER_ALLOC_POOL(index, name)

#define MEMORY_PROFILER_ENTER_TAG(tag)
#define MEMORY_PROFILER_LEAVE_TAG(tag)
#define MEMORY_PROFILER_CHECKPOINT(checkpoint)

#define ENABLE_CLASS_ALLOCATION_TRACKING(allocPool)

#define TRACKED_NEW(type, ...)      new type(##__VA_ARGS__)
#define TRACKED_NEW_ARRAY(type, n)  new type[n]

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)

#endif  // __DAVAENGINE_MEMORYPROFILER_H__

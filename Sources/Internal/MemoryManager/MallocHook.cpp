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


#include "Base/BaseTypes.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include <cstdlib>
#include <cassert>

#if defined(__DAVAENGINE_WIN32__)
#include <detours/detours.h>
#elif defined(__DAVAENGINE_ANDROID__)
#include <malloc.h>
#include <dlfcn.h>
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#include <dlfcn.h>
#include <malloc/malloc.h>
#else
#error "Unknown platform"
#endif

#include "MallocHook.h"
#include "AllocPools.h"
#include "MemoryManager.h"

namespace
{

void* HookedMalloc(size_t size)
{
    return DAVA::MemoryManager::Instance()->Allocate(size, DAVA::ALLOC_POOL_DEFAULT);
}

void* HookedRealloc(void* ptr, size_t newSize)
{
    return DAVA::MemoryManager::Instance()->Reallocate(ptr, newSize);
}

void* HookedCalloc(size_t count, size_t elemSize)
{
    void* ptr = nullptr;
    if (count > 0 && elemSize > 0)
    {
        ptr = malloc(count * elemSize);
        if (ptr != nullptr)
        {
            memset(ptr, 0, count * elemSize);
        }
    }
    return ptr;
}

char* HookedStrdup(const char* src)
{
    char* dst = nullptr;
    if (src != nullptr)
    {
        dst = static_cast<char*>(malloc(strlen(src)+1));
        if (dst != nullptr)
            strcpy(dst, src);
    }
    return dst;
}

void HookedFree(void* ptr)
{
    DAVA::MemoryManager::Instance()->Deallocate(ptr);
}
} // unnamed namespace

namespace DAVA
{

void* (*MallocHook::RealMalloc)(size_t) = &malloc;
void* (*MallocHook::RealRealloc)(void*, size_t) = &realloc;
void (*MallocHook::RealFree)(void*) = &free;
#if defined(__DAVAENGINE_ANDROID__)
size_t (*MallocHook::RealMallocSize)(void*) = nullptr;
#endif

MallocHook::MallocHook()
{
    Install();
}

void* MallocHook::Malloc(size_t size)
{
    return RealMalloc(size);
}

void* MallocHook::Realloc(void* ptr, size_t newSize)
{
    return RealRealloc(ptr, newSize);
}

void MallocHook::Free(void* ptr)
{
    RealFree(ptr);
}

size_t MallocHook::MallocSize(void* ptr)
{
#if defined(__DAVAENGINE_WIN32__)
    return _msize(ptr);
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
    return malloc_size(ptr);
#elif defined(__DAVAENGINE_ANDROID__)
    return RealMallocSize(ptr);
#else
#error "Unknown platform"
#endif
}

void MallocHook::Install()
{
    /*
     Explanation of allocation flow:
        app calls malloc --> HookedMalloc --> MemoryManager::Allocate --> MallocHook::Malloc --> original malloc
     Same flow with little differences is applied to other functions
     Such a long chain is neccessary for keeping as mush common code among different platforms as possible.
     To be able to call HookedMalloc instead of malloc I use some technique to intercept/replace functions.

     On Win32 I use Microsoft Detours library which modifies function prologue code with call to so called
     trampoline function. This trampoline function makes call to address which was specified by me (HookedMalloc).

     On *nix platforms (Android, iOS, Mac OS X, etc) I simply define my own implementation of malloc. In glibc
     malloc is a weak symbol which means that it can be overriden by an application. Additionally I get original
     address of malloc using dlsym function.
    */
#if defined(__DAVAENGINE_WIN32__)
    void* (*realCalloc)(size_t, size_t) = &calloc;
    char* (*realStrdup)(const char*) = &_strdup;

    auto detours = [](PVOID* what, PVOID hook) -> void {
        LONG result = 0;
        result = DetourTransactionBegin();
        assert(0 == result);
        result = DetourUpdateThread(GetCurrentThread());
        assert(0 == result);
        result = DetourAttach(what, hook);
        assert(0 == result);
        result = DetourTransactionCommit();
        assert(0 == result);
    };

    // On detours error you will see assert message
    detours(reinterpret_cast<PVOID*>(&RealMalloc), reinterpret_cast<PVOID>(&HookedMalloc));
    detours(reinterpret_cast<PVOID*>(&RealRealloc), reinterpret_cast<PVOID>(&HookedRealloc));
    detours(reinterpret_cast<PVOID*>(&realCalloc), reinterpret_cast<PVOID>(&HookedCalloc));
    detours(reinterpret_cast<PVOID*>(&realStrdup), reinterpret_cast<PVOID>(&HookedStrdup));
    detours(reinterpret_cast<PVOID*>(&RealFree), reinterpret_cast<PVOID>(&HookedFree));

#elif defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
    void* fptr = nullptr;

    // RTLD_DEFAULT tells to find the next occurrence of the desired symbol
    // in the search order after the current library
    // RTLD_NEXT tells to find the next occurrence of the desired symbol
    // in the search order after the current library

    // On Android use RTLD_DEFAULT as on RTLD_NEXT dlsym returns null
    // On Mac OS and iOS use RTLD_NEXT to not call malloc recursively
#if defined(__DAVAENGINE_ANDROID__)
    void* handle = RTLD_DEFAULT;
#else
    void* handle = RTLD_NEXT;
#endif
    fptr = dlsym(handle, "malloc");
    RealMalloc = reinterpret_cast<void* (*)(size_t)>(fptr);
    assert(fptr != nullptr && "Failed to get 'malloc'");

    fptr = dlsym(handle, "realloc");
    RealRealloc = reinterpret_cast<void* (*)(void*, size_t)>(fptr);
    assert(fptr != nullptr && "Failed to get 'realloc'");
    
    fptr = dlsym(handle, "free");
    RealFree = reinterpret_cast<void (*)(void*)>(fptr);
    assert(fptr != nullptr && "Failed to get 'free'");

#if defined(__DAVAENGINE_ANDROID__)
    // Get address of malloc_usable_size as it isn't exported on android
    void* libc = dlopen("libc.so", 0);
    assert(libc != nullptr && "Failed to load libc.so");
    fptr = dlsym(libc, "malloc_usable_size");
    RealMallocSize = reinterpret_cast<size_t (*)(void*)>(fptr);
    assert(fptr != nullptr && "Failed to get 'malloc_usable_size'");
#endif

#else
#error "Unknown platform"
#endif
}

}   // namespace DAVA

#if defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)

void* malloc(size_t size)
{
    return HookedMalloc(size);
}

void free(void* ptr)
{
    HookedFree(ptr);
}

void* realloc(void* ptr, size_t newSize)
{
    return HookedRealloc(ptr, newSize);
}

void* calloc(size_t count, size_t elemSize)
{
    return HookedCalloc(count, elemSize);
}

char* strdup(const char *src)
{
    return HookedStrdup(src);
}

#endif  // defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)

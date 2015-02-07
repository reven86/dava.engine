#include "memprof_config.h"

#if defined(MEMPROF_ENABLE)

#if defined(MEMPROF_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <detours/detours.h>
//#include <malloc.h>
#include <cstdlib>
#elif defined(MEMPROF_MACOS) || defined(MEMPROF_IOS) || defined(MEMPROF_ANDROID)
#include <cstdlib>
#include <dlfcn.h>
#endif

#include "malloc_hook.h"
#include "mem_profiler.h"

void* (__cdecl *malloc_hook::real_malloc)(size_t) = nullptr;
void* (__cdecl *malloc_hook::real_calloc)(size_t, size_t) = nullptr;
void* (__cdecl *malloc_hook::real_realloc)(void*, size_t) = nullptr;
void(__cdecl *malloc_hook::real_free)(void*) = nullptr;

malloc_hook::malloc_hook()
{
    install();
}

malloc_hook::~malloc_hook()
{
    uninstall();
}

void* malloc_hook::do_malloc(size_t size)
{
    return real_malloc(size);
}

void* malloc_hook::do_calloc(size_t count, size_t elem_size)
{
    return real_calloc(count, elem_size);
}

void* malloc_hook::do_realloc(void* ptr, size_t new_size)
{
    return real_realloc(ptr, new_size);
}

void malloc_hook::do_free(void* ptr)
{
    real_free(ptr);
}

void* __cdecl malloc_hook::hooked_malloc(size_t size)
{
    return mem_profiler::allocate(size, mem_type_e::MEM_TYPE_OTHER);
}

void* __cdecl malloc_hook::hooked_calloc(size_t count, size_t elem_size)
{
    return nullptr;
}

void* __cdecl malloc_hook::hooked_realloc(void* ptr, size_t new_size)
{
    return nullptr;
}

void __cdecl malloc_hook::hooked_free(void* ptr)
{
    mem_profiler::deallocate(ptr);
}

void malloc_hook::install()
{
#if defined(MEMPROF_WIN32)
    real_malloc  = &malloc;
    real_calloc  = &calloc;
    real_realloc = &realloc;
    real_free    = &free;

    //DetourTransactionBegin();
    //DetourUpdateThread(GetCurrentThread());
    //DetourAttach(reinterpret_cast<PVOID*>(&real_calloc), reinterpret_cast<PVOID>(&hooked_calloc));
    //DetourTransactionCommit();

    //DetourTransactionBegin();
    //DetourUpdateThread(GetCurrentThread());
    //DetourAttach(reinterpret_cast<PVOID*>(&real_realloc), reinterpret_cast<PVOID>(&hooked_realloc));
    //DetourTransactionCommit();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(reinterpret_cast<PVOID*>(&real_free), reinterpret_cast<PVOID>(&hooked_free));
    DetourTransactionCommit();

    // malloc should be last detoured function as detours lib uses malloc internally :)
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(reinterpret_cast<PVOID*>(&real_malloc), reinterpret_cast<PVOID>(&hooked_malloc));
    DetourTransactionCommit();
    
#elif defined(MEMPROF_MACOS) || defined(MEMPROF_IOS) || defined(MEMPROF_ANDROID)

    void* handle = reinterpret_cast<void*>(-1);
    void* fptr   = nullptr;
    
    fptr = dlsym(handle, "malloc");
    real_malloc = reinterpret_cast<void* (*)(size_t)>(fptr);
    
    fptr = dlsym(handle, "free");
    real_free = reinterpret_cast<void (*)(void*)>(fptr);
    
#endif
}

void malloc_hook::uninstall()
{

}

#if defined(MEMPROF_MACOS) || defined(MEMPROF_IOS) || defined(MEMPROF_ANDROID)

void* malloc(size_t size)
{
    return mem_profiler::allocate(size, mem_type_e::MEM_TYPE_OTHER);
}

void free(void* ptr)
{
    mem_profiler::deallocate(ptr);
}

#endif  // defined(MEMPROF_MACOS) || defined(MEMPROF_IOS) || defined(MEMPROF_ANDROID)

#endif

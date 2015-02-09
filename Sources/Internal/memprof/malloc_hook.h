#pragma once

#include "memprof_config.h"

#if defined(MEMPROF_ENABLE)

#include <cstddef>

class malloc_hook final
{
    friend class mem_profiler;

private:
    malloc_hook();
    ~malloc_hook();

public:
    static void* do_malloc(size_t size);
    static void* do_calloc(size_t count, size_t elem_size);
    static void* do_realloc(void* ptr, size_t new_size);
    static void do_free(void* ptr);

private:
    static void install();
    static void uninstall();
    
public: // make public to call from realloc, calloc on android, macos
    static void* __cdecl hooked_malloc(size_t size);
    static void* __cdecl hooked_calloc(size_t count, size_t elem_size);
    static void* __cdecl hooked_realloc(void* ptr, size_t new_size);
    static void __cdecl hooked_free(void* ptr);

private:
    static void* (__cdecl *real_malloc)(size_t);
    static void* (__cdecl *real_calloc)(size_t, size_t);
    static void* (__cdecl *real_realloc)(void*, size_t);
    static void (__cdecl *real_free)(void*);
};

#endif

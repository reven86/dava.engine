#ifndef __DAVAENGINE_MALLOCHOOK_H__
#define __DAVAENGINE_MALLOCHOOK_H__

#include "Base/BaseTypes.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

namespace DAVA
{
class MallocHook final
{
public:
    MallocHook();
    ~MallocHook() = default;

    static void* Malloc(size_t size);
    static void* Realloc(void* ptr, size_t newSize);
    static void Free(void* ptr);
    static size_t MallocSize(void* ptr);

private:
    static void Install();

private:
    static void* (*RealMalloc)(size_t);
    static void* (*RealRealloc)(void*, size_t);
    static void (*RealFree)(void*);
#if defined(__DAVAENGINE_ANDROID__)
    static size_t (*RealMallocSize)(void*);
#endif
};

} // namespace DAVA

#endif // defined(DAVA_MEMORY_PROFILING_ENABLE)

#endif // __DAVAENGINE_MALLOCHOOK_H__

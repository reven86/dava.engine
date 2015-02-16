#include "Base/BaseTypes.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include "MemoryManager.h"

/*
* http://en.cppreference.com/w/cpp/memory/new/operator_new
* The single-object version is called by the standard library implementations of all other versions,
* so replacing that one function is sufficient to handle all deallocations.	(since C++11)
*/

#if defined(__DAVAENGINE_WIN32__)
#define NOEXCEPT    throw()
#else
#define NOEXCEPT    noexcept
#endif

void* operator new(size_t size)
{
    return DAVA::MemoryManager::Allocate(size, DAVA::ALLOC_POOL_APP);
}

void operator delete(void* ptr) NOEXCEPT
{
    DAVA::MemoryManager::Deallocate(ptr);
}

void* operator new [](size_t size)
{
    return DAVA::MemoryManager::Allocate(size, DAVA::ALLOC_POOL_APP);
}

void operator delete[](void* ptr) NOEXCEPT
{
    DAVA::MemoryManager::Deallocate(ptr);
}

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)

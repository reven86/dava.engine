#include "Concurrency/Semaphore.h"

#include "Base/Platform.h"
#include "Debug/DVAssert.h"

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
#include <dispatch/dispatch.h>
#elif defined(__DAVAENGINE_ANDROID__)
#include <semaphore.h>
#endif //PLATFORMS

namespace DAVA
{

#if defined(__DAVAENGINE_WINDOWS__)
// HANDLE semaphore;
static_assert(sizeof(HANDLE) == sizeof(void*), "fix native semaphore type");
#elif defined(__DAVAENGINE_APPLE__)
// dispatch_semaphore_t semaphore;
static_assert(sizeof(dispatch_semaphore_t) == sizeof(void*), "fix native semaphore type");
#elif defined(__DAVAENGINE_ANDROID__)
// sem_t semaphore;
static_assert(sizeof(sem_t) == sizeof(void*), "fix native semaphore type");
#endif //PLATFORMS



#if defined(__DAVAENGINE_WINDOWS__)

// ##########################################################################################################
// Windows implementation
// ##########################################################################################################

Semaphore::Semaphore(uint32 count)
{
#ifdef __DAVAENGINE_WIN32__
    semaphore = CreateSemaphore(nullptr, count, 0x0FFFFFFF, nullptr);
#else
    semaphore = CreateSemaphoreEx(nullptr, count, 0x0FFFFFFF, nullptr, 0, SEMAPHORE_ALL_ACCESS);
#endif
    DVASSERT(nullptr != semaphore);
}

Semaphore::~Semaphore()
{
    CloseHandle(semaphore);
}

void Semaphore::Post(uint32 count)
{
    DVASSERT(count > 0);
    ReleaseSemaphore(semaphore, count, nullptr);
}

void Semaphore::Wait()
{
    WaitForSingleObjectEx(semaphore, INFINITE, FALSE);
}

#elif defined(__DAVAENGINE_APPLE__)

// ##########################################################################################################
// MacOS/IOS implementation
// ##########################################################################################################

Semaphore::Semaphore(uint32 count)
{
    semaphore = dispatch_semaphore_create(count);
}

Semaphore::~Semaphore()
{
    dispatch_release(reinterpret_cast<dispatch_semaphore_t>(semaphore));
}

void Semaphore::Post(uint32 count)
{
    while (count-- > 0)
    {
        dispatch_semaphore_signal(reinterpret_cast<dispatch_semaphore_t>(semaphore));
    }
}

void Semaphore::Wait()
{
    dispatch_semaphore_wait(reinterpret_cast<dispatch_semaphore_t>(semaphore), DISPATCH_TIME_FOREVER);
}

#elif defined(__DAVAENGINE_ANDROID__)

// ##########################################################################################################
// Android implementation
// ##########################################################################################################
Semaphore::Semaphore(uint32 count)
{
    sem_init(&semaphore, 0, count);
}

Semaphore::~Semaphore()
{
    sem_destroy(&semaphore);
}

void Semaphore::Post(uint32 count)
{
    while (count-- > 0)
    {
        sem_post(&semaphore);
    }
}

void Semaphore::Wait()
{
    sem_wait(&semaphore);
}
#endif
} // end namespace DAVA

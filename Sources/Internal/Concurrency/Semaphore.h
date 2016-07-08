#ifndef __DAVAENGINE_SEMAPHORE_H__
#define __DAVAENGINE_SEMAPHORE_H__

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
#include <dispatch/dispatch.h>
#elif defined(__DAVAENGINE_ANDROID__)
#include <semaphore.h>
#endif //PLATFORMS

namespace DAVA
{
/*! brief Semaphore wrapper class compatible with Thread class. Supports Win32, MacOS, iPhone, Android platforms. */
class Semaphore
{
public:
    Semaphore(uint32 count = 0);
    ~Semaphore();

    void Post(uint32 count = 1);
    void Wait();

protected:

#if defined(__DAVAENGINE_WINDOWS__)
    HANDLE semaphore;
#elif defined(__DAVAENGINE_APPLE__)
    dispatch_semaphore_t semaphore;
#elif defined(__DAVAENGINE_ANDROID__)
    sem_t semaphore;
#endif //PLATFORMS
};

#if defined(__DAVAENGINE_WINDOWS__)

// ##########################################################################################################
// Windows implementation
// ##########################################################################################################

inline Semaphore::Semaphore(uint32 count)
{
#ifdef __DAVAENGINE_WIN32__
    semaphore = CreateSemaphore(nullptr, count, 0x0FFFFFFF, nullptr);
#else
    semaphore = CreateSemaphoreEx(nullptr, count, 0x0FFFFFFF, nullptr, 0, SEMAPHORE_ALL_ACCESS);
#endif
    DVASSERT(nullptr != semaphore);
}

inline Semaphore::~Semaphore()
{
    CloseHandle(semaphore);
}

inline void Semaphore::Post(uint32 count)
{
    DVASSERT(count > 0);
    ReleaseSemaphore(semaphore, count, nullptr);
}

inline void Semaphore::Wait()
{
    WaitForSingleObjectEx(semaphore, INFINITE, FALSE);
}

#elif defined(__DAVAENGINE_APPLE__)

// ##########################################################################################################
// MacOS/IOS implementation
// ##########################################################################################################

inline Semaphore::Semaphore(uint32 count)
{
    semaphore = dispatch_semaphore_create(count);
}

inline Semaphore::~Semaphore()
{
    dispatch_release(semaphore);
}

inline void Semaphore::Post(uint32 count)
{
    while (count-- > 0)
    {
        dispatch_semaphore_signal(semaphore);
    }
}

inline void Semaphore::Wait()
{
    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
}

#elif defined(__DAVAENGINE_ANDROID__)

// ##########################################################################################################
// Android implementation
// ##########################################################################################################
inline Semaphore::Semaphore(uint32 count)
{
    sem_init(&semaphore, 0, count);
}

inline Semaphore::~Semaphore()
{
    sem_destroy(&semaphore);
}

inline void Semaphore::Post(uint32 count)
{
    while (count-- > 0)
    {
        sem_post(&semaphore);
    }
}

inline void Semaphore::Wait()
{
    sem_wait(&semaphore);
}
#endif
};

#endif // __DAVAENGINE_SEMAPHORE_H__
#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
/*! brief Semaphore wrapper class compatible with Thread class. Supports Win32, MacOS, iPhone, Android platforms. */
class Semaphore
{
public:
    Semaphore(uint32 count = 0);
    Semaphore(const Semaphore&) = delete;
    Semaphore(Semaphore&&) = delete;
    ~Semaphore();

    void Post(uint32 count = 1);
    void Wait();

protected:
    void* semaphore = nullptr;
};

} // end namespace DAVA

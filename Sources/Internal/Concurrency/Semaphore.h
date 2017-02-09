#pragma once

#include <cstdint>

namespace DAVA
{
/*! brief Semaphore wrapper class compatible with Thread class. Supports Win32, MacOS, iPhone, Android platforms. */
class Semaphore
{
public:
    /**
		default semaphore state is zero 0
	*/
    Semaphore(uint32_t count = 0U);
    Semaphore(const Semaphore&) = delete;
    Semaphore(Semaphore&&) = delete;
    ~Semaphore();

    /**
		increment semaphore with count (default 1)
	*/
    void Post(uint32_t count = 1);
    /**
		decrement semaphore with 1 and wait till semaphore value become >= 0,
		if semaphore already zero or more return immediately and continue
	*/
    void Wait();

protected:
    //!< platform dependent semaphore handle
    uintptr_t semaphore = 0;
};

} // end namespace DAVA

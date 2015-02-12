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


#ifndef __DAVAENGINE_SPINLOCK_H__
#define __DAVAENGINE_SPINLOCK_H__

#include "Base/BaseTypes.h"
#include "Base/Atomic.h"

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
#include <libkern/OSAtomic.h>
#endif //PLATFORMS

namespace DAVA
{

/*! brief Spinlock wrapper class compatible with Thread class. Supports Win32, MacOS, iPhone, Android platforms. */
class Spinlock
{
public:
    Spinlock() : spin(0) {};

    void Lock();
    void Unlock();
    bool TryLock();

protected:

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_ANDROID__)
    volatile int32 spin;
#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
    OSSpinLock spin;
#endif //PLATFORMS

};

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_ANDROID__)

// ##########################################################################################################
// Windows/Android implementation
// ##########################################################################################################

inline void Spinlock::Lock()
{
    // try to set spin = 1
    while(!AtomicCompareAndSwap(0, 1, (int&) spin))
    {
        // wait till spin become equal to 0
        while(0 != spin) 
        { }
    }
}

inline void Spinlock::Unlock()
{
    // set spin = 0
    AtomicCompareAndSwap(1, 0, (int&) spin);
}

inline bool Spinlock::TryLock()
{
    // try to set spin = 1
    return AtomicCompareAndSwap(0, 1, (int&) spin);
}

#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)

// ##########################################################################################################
// MacOS/IOS implementation
// ##########################################################################################################

inline void Spinlock::Lock()
{
    OSSpinLockLock(&spin);
}

inline void Spinlock::Unlock()
{
    OSSpinLockUnlock(&spin);
}

inline bool Spinlock::TryLock()
{
    return OSSpinLockTry(&spin);
}

#endif //PLATFORMS

};

#endif // __DAVAENGINE_SPINLOCK_H__

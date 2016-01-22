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

#ifndef __DAVAENGINE_MANUAL_RESET_EVENT_H__
#define __DAVAENGINE_MANUAL_RESET_EVENT_H__

#include "SemaphoreLite.h"
#include <atomic>

namespace DAVA
{
/*!
    \brief ManualResetEvent
    Notifies one or more waiting threads that an event has occurred.

    When a thread begins an activity that must complete before other threads proceed, it calls Reset
    to put ManualResetEvent in the non-signaled state. This thread can be thought of as controlling 
    the ManualResetEvent. Threads that call Wait() on the ManualResetEvent will block, awaiting the 
    signal. When the controlling thread completes the activity, it calls Signal() to signal that the
    waiting threads can proceed. All waiting threads are released.

    Once it has been signaled, ManualResetEvent remains signaled until it is manually reset. That is,
    calls to Wait() return immediately.
*/
class ManualResetEvent final
{
public:
    ManualResetEvent(bool isSignaled = true)
    {
        status.store(isSignaled ? 1 : 0, std::memory_order_release);
    }

    void Signal()
    {
        int oldStatus = status.exchange(1, std::memory_order_release);
        if (oldStatus < 0)
        {
            sem.Post(-oldStatus);
        }
    }

    void Reset()
    {
        int curValue = 1;
        status.compare_exchange_weak(curValue, 0, std::memory_order_release, std::memory_order_acquire);
    }

    void Wait()
    {
        int curStatus = status.load(std::memory_order_relaxed);
        while (curStatus < 1)
        {
            // status is in reset state, so we should
            // subtract 1 from it and sleep on semaphore
            int newStatus = curStatus - 1;

            // try to compare and swap old value (curStatus) with new one (newStatus)
            // if value in status changed so that it isn't equal to the curStatus
            // CAS operation will fail and curStatus variable will be renewed
            if (status.compare_exchange_weak(curStatus, newStatus, std::memory_order_release, std::memory_order_relaxed))
            {
                sem.Wait();
                break;
            }
        }
    }

private:
    // status == 1: is signaled
    // status == 0: is reset and no threads are waiting
    // status == -N: is reset and N threads are waiting
    std::atomic<int> status;
    DAVA::SemaphoreLite sem;
};

} // namespace DAVA

#endif // __DAVAENGINE_MANUAL_RESET_EVENT_H__

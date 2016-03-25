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

#ifndef __DAVAENGINE_AUTO_RESET_EVENT_H__
#define __DAVAENGINE_AUTO_RESET_EVENT_H__

#include "SemaphoreLite.h"
#include <atomic>

namespace DAVA
{
/*! 
    \brief AutoResetEvent
    Notifies a waiting thread that an event has occurred.

    A thread waits for a signal by calling Wait() on the AutoResetEvent. If the AutoResetEvent is 
    in the non-signaled state, the thread blocks, waiting for the thread that currently controls
    the resource to signal that the resource is available by calling Signal().

    Calling Signal() signals AutoResetEvent to release a waiting thread. AutoResetEvent remains signaled
    until a single waiting thread is released, and then automatically returns to the non-signaled state.
    If no threads are waiting, the state remains signaled indefinitely.
*/
class AutoResetEvent final
{
public:
    AutoResetEvent(bool isSignaled = false, uint32 spinCount_ = SemaphoreLite::defaultSpinCount)
        : sem(0, spinCount_)
    {
        status.store((isSignaled) ? 1 : 0, std::memory_order_release);
    }

    inline void Signal()
    {
        int curStatus = status.load(std::memory_order_relaxed);

        while (true)
        {
            int newStatus = curStatus < 1 ? curStatus + 1 : 1;

            // try to compare and swap old value (curStatus) with new one (newStatus)
            // if value in status changed so that it isn't equal to the curStatus
            // CAS operation will fail and curStatus variable will be renewed
            if (status.compare_exchange_weak(curStatus, newStatus, std::memory_order_release, std::memory_order_relaxed))
            {
                break;
            }
        }

        if (curStatus < 0)
        {
            // release one waiting thread
            sem.Post();
        }
    }

    inline void Wait()
    {
        int oldStatus = status.fetch_sub(1, std::memory_order_acquire);
        if (oldStatus < 1)
        {
            // wasn't signaled, so wait
            sem.Wait();
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

#endif // __DAVAENGINE_AUTO_RESET_EVENT_H__

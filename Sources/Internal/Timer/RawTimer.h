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


#ifndef __DAVAENGINE_RAW_TIMER_H__
#define __DAVAENGINE_RAW_TIMER_H__

#include "Base/BaseTypes.h"

namespace  DAVA
{
    
/*
 Raw Timer should be used when you need to get elapsed time in miliseconds from calling Start() to calling GetElapsed().
 It is not thread safe class.
*/

class RawTimer
{
public:
    /* 
       \brief Starts time calculation. Now GetElapsed() should return nut 0
     */
    void Start();
    /*
     \brief Stops time calculation. GetElapsed() whould return 0.
     */
    void Stop();
    /*
     \brief Resumes stopped time calculation. It means that GetElapsed() will return time delta from calling Start().
     */
    void Resume();
    
    /*
     \brief Indicates if time calculation is started
     */
    bool IsStarted();
    /*
     \brief Returns time in ms elapsed from calling Start(). Returns 0 if timer is stopped.
     */
    uint64 GetElapsed();
    
private:
    uint64 timerStartTime;
    bool isStarted = false;
};
    
}
#endif //__DAVAENGINE_RAW_TIMER_H__


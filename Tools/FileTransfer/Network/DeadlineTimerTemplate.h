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


#ifndef __DAVAENGINE_DEADLINETIMERTEMPLATE_H__
#define __DAVAENGINE_DEADLINETIMERTEMPLATE_H__

#include <libuv/uv.h>

#include <Base/BaseTypes.h>

#include "DeadlineTimerBase.h"

namespace DAVA
{

class IOLoop;

/*
 Template class DeadlineTimerTemplate provides basic waitable timer functionality
 Template parameter T specifies type that inherits DeadlineTimerTemplate(CRTP idiom)
 Bool template parameter autoRepeat specifies timer behaviour:
    when autoRepeat is true libuv automatically issues next wait operations until AsyncStop is called
    when autoRepeat is false user should explicitly issue next wait operation

 Type specified by T should implement methods:
    void HandleTimer()
        This method is called when timeout has expired
    void HandleClose()
        This method is called after underlying timer has been closed by libuv

 Summary of methods that should be implemented by T:
    void HandleTimer();
    void HandleClose();
*/
template<typename T, bool autoRepeat = false>
class DeadlineTimerTemplate : public DeadlineTimerBase
{
private:
    typedef DeadlineTimerBase BaseClassType;
    typedef T                 DerivedClassType;

    static const bool autoRepeatFlag = autoRepeat;

public:
    explicit DeadlineTimerTemplate(IOLoop* loop);
    ~DeadlineTimerTemplate() {}

    void Close();
    void StopAsyncWait();

protected:
    int32 InternalAsyncStartWait(uint32 timeout, uint32 repeat);

private:
    void HandleClose() {}
    void HandleTimer() {}

    static void HandleCloseThunk(uv_handle_t* handle);
    static void HandleTimerThunk(uv_timer_t* handle);
};

//////////////////////////////////////////////////////////////////////////
template<typename T, bool autoRepeat>
DeadlineTimerTemplate<T, autoRepeat>::DeadlineTimerTemplate(IOLoop* loop) : BaseClassType(loop)
{
    handle.data = static_cast<DerivedClassType*>(this);
}

template<typename T, bool autoRepeat>
void DeadlineTimerTemplate<T, autoRepeat>::Close()
{
    BaseClassType::InternalClose(&HandleCloseThunk);
}

template<typename T, bool autoRepeat>
void DeadlineTimerTemplate<T, autoRepeat>::StopAsyncWait()
{
    uv_timer_stop(Handle());
}

template<typename T, bool autoRepeat>
int32 DeadlineTimerTemplate<T, autoRepeat>::InternalAsyncStartWait(uint32 timeout, uint32 repeat)
{
    return uv_timer_start(Handle(), &HandleTimerThunk, timeout, repeat);
}

template<typename T, bool autoRepeat>
void DeadlineTimerTemplate<T, autoRepeat>::HandleCloseThunk(uv_handle_t* handle)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->CleanUpBeforeNextUse();
    pthis->HandleClose();
}

template<typename T, bool autoRepeat>
void DeadlineTimerTemplate<T, autoRepeat>::HandleTimerThunk(uv_timer_t* handle)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    if(!autoRepeatFlag)
    {
        pthis->StopAsyncWait();
    }
    pthis->HandleTimer();
}

}   // namespace DAVA

#endif  // __DAVAENGINE_DEADLINETIMERTEMPLATE_H__

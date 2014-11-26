/*==================================================================================
    Copyright(c) 2008, binaryzebra
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

#include "HandleBase.h"

namespace DAVA
{

class IOLoop;

/*
 Template class DeadlineTimerTemplate provides basic waitable timer functionality
 Template parameter T specifies type that inherits DeadlineTimerTemplate and implements necessary methods (CRTP idiom).

 Type specified by T should implement methods:
    1. void HandleClose(), called after timer handle has been closed by libuv
    2. void HandleTimer(), called when timeout has expired

 Summary of methods that should be implemented by T:
    void HandleTimer();
    void HandleClose();
*/
template<typename T>
class DeadlineTimerTemplate : public HandleBase<uv_timer_t>
{
private:
    typedef HandleBase<uv_timer_t> BaseClassType;
    typedef T                      DerivedClassType;

public:
    DeadlineTimerTemplate(IOLoop* loop);
    ~DeadlineTimerTemplate() {}

    void Close();
    void StopAsyncWait();

    // Methods should be implemented in derived class
    void HandleClose();
    void HandleTimer();

protected:
    int32 InternalAsyncWait(uint32 timeout, uint32 repeat);

    // Thunks between C callbacks and C++ class methods
    static void HandleCloseThunk(uv_handle_t* handle);
    static void HandleTimerThunk(uv_timer_t* handle);
};

//////////////////////////////////////////////////////////////////////////
template<typename T>
DeadlineTimerTemplate<T>::DeadlineTimerTemplate(IOLoop* loop) : BaseClassType(loop)
{
    SetHandleData(static_cast<DerivedClassType*>(this));
}

template<typename T>
void DeadlineTimerTemplate<T>::Close()
{
    BaseClassType::InternalClose(&HandleCloseThunk);
}

template<typename T>
void DeadlineTimerTemplate<T>::StopAsyncWait()
{
    uv_timer_stop(Handle<uv_timer_t>());
}

template<typename T>
int32 DeadlineTimerTemplate<T>::InternalAsyncWait(uint32 timeout, uint32 repeat)
{
    return uv_timer_start(Handle<uv_timer_t>(), &HandleTimerThunk, timeout, repeat);
}

///   Thunks   ///////////////////////////////////////////////////////////
template<typename T>
void DeadlineTimerTemplate<T>::HandleCloseThunk(uv_handle_t* handle)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->InternalInit();
    pthis->HandleClose();
}

template<typename T>
void DeadlineTimerTemplate<T>::HandleTimerThunk(uv_timer_t* handle)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->HandleTimer();
}

}   // namespace DAVA

#endif  // __DAVAENGINE_DEADLINETIMERTEMPLATE_H__

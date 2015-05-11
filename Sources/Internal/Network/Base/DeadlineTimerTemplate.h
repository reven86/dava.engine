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

#include <Base/BaseTypes.h>
#include <Base/Noncopyable.h>

#include <Network/Base/IOLoop.h>

namespace DAVA
{
namespace Net
{

/*
 Template class DeadlineTimerTemplate wraps timer from underlying network library and provides interface to user
 through CRTP idiom. Class specified by template parameter T should inherit DeadlineTimerTemplate and provide some
 members that will be called by base class (DeadlineTimerTemplate) using compile-time polymorphism.
*/
template<typename T>
class DeadlineTimerTemplate : private Noncopyable
{
public:
    DeadlineTimerTemplate(IOLoop* loop);
    ~DeadlineTimerTemplate();

    void CancelWait();

    bool IsOpen() const;
    bool IsClosing() const;

protected:
    int32 DoOpen();
    int32 DoWait(uint32 timeout);
    void DoClose();

    // Thunks between C callbacks and C++ class methods
    static void HandleCloseThunk(uv_handle_t* handle);
    static void HandleTimerThunk(uv_timer_t* handle);

private:
    uv_timer_t uvhandle;        // libuv handle itself
    IOLoop* loop;               // IOLoop object handle is attached to
    bool isOpen;                // Handle has been initialized and can be used in operations
    bool isClosing;             // Close has been issued and waiting for close operation complete, used mainly for asserts
};

//////////////////////////////////////////////////////////////////////////
template<typename T>
DeadlineTimerTemplate<T>::DeadlineTimerTemplate(IOLoop* ioLoop) : uvhandle()
                                                                , loop(ioLoop)
                                                                , isOpen(false)
                                                                , isClosing(false)
{
    DVASSERT(ioLoop != NULL);
}

template<typename T>
DeadlineTimerTemplate<T>::~DeadlineTimerTemplate()
{
    // libuv handle should be closed before destroying object
    DVASSERT(false == isOpen && false == isClosing);
}

template<typename T>
void DeadlineTimerTemplate<T>::CancelWait()
{
    DVASSERT(true == isOpen && false == isClosing);
    //UNCOMMENT
    //uv_timer_stop(&uvhandle);
}

template<typename T>
bool DeadlineTimerTemplate<T>::IsOpen() const
{
    return isOpen;
}

template<typename T>
bool DeadlineTimerTemplate<T>::IsClosing() const
{
    return isClosing;
}

template<typename T>
int32 DeadlineTimerTemplate<T>::DoOpen()
{
    DVASSERT(false == isOpen && false == isClosing);
    //UNCOMMENT
    int32 error = 0;//uv_timer_init(loop->Handle(), &uvhandle);
    if (0 == error)
    {
        isOpen = true;
        uvhandle.data = this;
    }
    return error;
}

template<typename T>
int32 DeadlineTimerTemplate<T>::DoWait(uint32 timeout)
{
    DVASSERT(false == isClosing);
    int32 error = 0;
    if (false == isOpen)
        error = DoOpen();   // Automatically open on first call
    //UNCOMMENT
    //if (0 == error)
      //  error = uv_timer_start(&uvhandle, &HandleTimerThunk, timeout, 0);
    return error;
}

template<typename T>
void DeadlineTimerTemplate<T>::DoClose()
{
    DVASSERT(true == isOpen && false == isClosing);
    isOpen = false;
    isClosing = true;
    //uv_close(reinterpret_cast<uv_handle_t*>(&uvhandle), &HandleCloseThunk);
}

///   Thunks   ///////////////////////////////////////////////////////////
template<typename T>
void DeadlineTimerTemplate<T>::HandleCloseThunk(uv_handle_t* handle)
{
    DeadlineTimerTemplate* self = static_cast<DeadlineTimerTemplate*>(handle->data);
    self->isClosing = false;    // Mark timer has been closed
    // And clear handle
    Memset(&self->uvhandle, 0, sizeof(self->uvhandle));

    static_cast<T*>(self)->HandleClose();
}

template<typename T>
void DeadlineTimerTemplate<T>::HandleTimerThunk(uv_timer_t* handle)
{
    DeadlineTimerTemplate* self = static_cast<DeadlineTimerTemplate*>(handle->data);
    static_cast<T*>(self)->HandleTimer();
}

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_DEADLINETIMERTEMPLATE_H__

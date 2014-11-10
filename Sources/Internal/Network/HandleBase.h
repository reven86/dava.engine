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

#ifndef __DAVAENGINE_HANDLEBASE_H__
#define __DAVAENGINE_HANDLEBASE_H__

#include <type_traits>

#include <libuv/uv.h>

#include <Base/Noncopyable.h>
#include <Base/BaseTypes.h>
#include <Debug/DVAssert.h>

#include "HandleTraits.h"

namespace DAVA
{

class IOLoop;

template<typename T>
class HandleBase : private Noncopyable
{
    static_assert(IsHandleType<T>::value, "Not handle type");

public:
    IOLoop* Loop() { return loop; }

    bool IsClosing() const;
    bool IsActive() const;

protected:
    // Protected constructor and destructor to prevent creation and deletion through this type
    HandleBase(IOLoop* ioLoop);
    HandleBase(IOLoop* ioLoop, uv_async_cb asyncCallback);
    ~HandleBase() {}

    void SetHandleData(void* handleData);

    template<typename U>
    U* Handle() { return CastHandleTo<U>(&handle); }
    template<typename U>
    const U* Handle() const { return CastHandleTo<const U>(&handle); }

    void InternalInit();
    void InternalInit(uv_async_cb asyncCallback);
    void InternalClose(uv_close_cb closeCallback);

protected:
    IOLoop* loop;
    T       handle;
};

//////////////////////////////////////////////////////////////////////////
template<typename T>
HandleBase<T>::HandleBase(IOLoop* ioLoop) : loop(ioLoop)
                                          , handle()
{
    DVASSERT(ioLoop != NULL);
    InternalInit();
}

template<typename T>
HandleBase<T>::HandleBase(IOLoop* ioLoop, uv_async_cb asyncCallback) : loop(ioLoop)
                                                                     , handle()
{
    DVASSERT(ioLoop != NULL);
    InternalInit(asyncCallback);
}

template<typename T>
bool HandleBase<T>::IsClosing() const
{
    return uv_is_closing(Handle<uv_handle_t>()) != 0;
}

template<typename T>
bool HandleBase<T>::IsActive() const
{
    return uv_is_active(Handle<uv_handle_t>()) != 0;
}

template<typename T>
void HandleBase<T>::SetHandleData(void* handleData)
{
    handle.data = handleData;
}

template<typename T>
void HandleBase<T>::InternalClose(uv_close_cb closeCallback)
{
    if (!IsClosing())
    {
        uv_close(Handle<uv_handle_t>(), closeCallback);
    }
}

}   // namespace DAVA

#endif  // __DAVAENGINE_HANDLEBASE_H__

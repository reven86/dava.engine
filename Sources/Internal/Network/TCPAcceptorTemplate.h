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

#ifndef __DAVAENGINE_TCPACCEPTORTEMPLATE_H__
#define __DAVAENGINE_TCPACCEPTORTEMPLATE_H__

#include <Debug/DVAssert.h>

#include "TCPSocketBase.h"

namespace DAVA
{

class IOLoop;

/*
 Template class TCPAcceptorTemplate provides basic capabilities: accepting incoming TCP connections.
 Template parameter T specifies type that inherits TCPAcceptorTemplate(CRTP idiom)

 Type specified by T should implement methods:
    void HandleConnect(int32 error)
        This method is called on recieving new incoming TCP connection.
        Parameter error is non zero on error
    void HandleClose() - optional
        This method is called after underlying socket has been closed by libuv

 Summary of methods that should be implemented by T:
    void HandleConnect(int32 error);
    void HandleClose();
*/
template <typename T>
class TCPAcceptorTemplate : public TCPSocketBase
{
private:
    typedef TCPSocketBase BaseClassType;
    typedef T             DerivedClassType;

public:
    explicit TCPAcceptorTemplate(IOLoop* ioLoop);
    ~TCPAcceptorTemplate() {}

    void Close();
    int32 Accept(TCPSocketBase* socket);

protected:
    int32 InternalAsyncListen(int32 backlog);

private:
    void HandleClose() {}
    void HandleConnect(int32 /*error*/) {}

    static void HandleCloseThunk(uv_handle_t* handle);
    static void HandleConnectThunk(uv_stream_t* handle, int error);
};

//////////////////////////////////////////////////////////////////////////
template <typename T>
TCPAcceptorTemplate<T>::TCPAcceptorTemplate(IOLoop* ioLoop) : TCPSocketBase(ioLoop)
{
    handle.data = static_cast<DerivedClassType*>(this);
}

template <typename T>
void TCPAcceptorTemplate<T>::Close()
{
    BaseClassType::InternalClose(&HandleCloseThunk);
}

template <typename T>
int32 TCPAcceptorTemplate<T>::Accept(TCPSocketBase* socket)
{
    DVASSERT(socket);
    return uv_accept(HandleAsStream(), socket->HandleAsStream());
}

template <typename T>
int32 TCPAcceptorTemplate<T>::InternalAsyncListen(int32 backlog)
{
    DVASSERT(backlog > 0);
    return uv_listen(HandleAsStream(), backlog, &HandleConnectThunk);
}

template <typename T>
void TCPAcceptorTemplate<T>::HandleCloseThunk(uv_handle_t* handle)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->CleanUpBeforeNextUse();
    pthis->HandleClose();
}

template <typename T>
void TCPAcceptorTemplate<T>::HandleConnectThunk(uv_stream_t* handle, int error)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->HandleConnect(error);
}

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPACCEPTORTEMPLATE_H__

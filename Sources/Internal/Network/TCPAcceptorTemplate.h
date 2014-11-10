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

#include "Endpoint.h"
#include "HandleBase.h"

namespace DAVA
{

class IOLoop;

/*
 Template class TCPAcceptorTemplate provides basic capabilities for accepting incoming TCP connections.
 Template parameter T specifies type that inherits TCPAcceptorTemplate and implements necessary methods (CRTP idiom).

 Type specified by T should implement methods:
    1. void HandleClose(), called after acceptor handle has been closed by libuv
    2. void HandleConnect(int32 error), called after incoming connection has arrived
        Parameters:
            error - nonzero if error has occured
*/
template <typename T>
class TCPAcceptorTemplate : public HandleBase<uv_tcp_t>
{
private:
    typedef HandleBase<uv_tcp_t> BaseClassType;
    typedef T                    DerivedClassType;

public:
    TCPAcceptorTemplate(IOLoop* ioLoop);
    ~TCPAcceptorTemplate() {}

    void Close();

    int32 Bind(const Endpoint& endpoint);
    int32 Bind(const char8* ipaddr, uint16 port);
    int32 Bind(uint16 port);

    int32 Accept(HandleBase<uv_tcp_t>* socket);

protected:
    int32 InternalAsyncListen(int32 backlog);

private:
    // Methods should be implemented in derived class
    void HandleClose();
    void HandleConnect(int32 error);

    // Thunks between C callbacks and C++ class methods
    static void HandleCloseThunk(uv_handle_t* handle);
    static void HandleConnectThunk(uv_stream_t* handle, int error);
};

//////////////////////////////////////////////////////////////////////////
template <typename T>
TCPAcceptorTemplate<T>::TCPAcceptorTemplate(IOLoop* ioLoop) : BaseClassType(ioLoop)
{
    SetHandleData(static_cast<DerivedClassType*>(this));
}

template <typename T>
void TCPAcceptorTemplate<T>::Close()
{
    BaseClassType::InternalClose(&HandleCloseThunk);
}

template <typename T>
int32 TCPAcceptorTemplate<T>::Bind(const Endpoint& endpoint)
{
    return uv_tcp_bind(Handle<uv_tcp_t>(), endpoint.CastToSockaddr(), 0);
}

template <typename T>
int32 TCPAcceptorTemplate<T>::Bind(const char8* ipaddr, uint16 port)
{
    DVASSERT(ipaddr != NULL);

    Endpoint endpoint;
    int32 result = uv_ip4_addr(ipaddr, port, endpoint.CastToSockaddrIn());
    if(0 == result)
    {
        result = Bind(endpoint);
    }
    return result;
}

template <typename T>
int32 TCPAcceptorTemplate<T>::Bind(uint16 port)
{
    return Bind(Endpoint(port));
}

template <typename T>
int32 TCPAcceptorTemplate<T>::Accept(HandleBase<uv_tcp_t>* socket)
{
    DVASSERT(socket);
    return uv_accept(Handle<uv_stream_t>(), socket->Handle<>());
}

template <typename T>
int32 TCPAcceptorTemplate<T>::InternalAsyncListen(int32 backlog)
{
    DVASSERT(backlog > 0);
    return uv_listen(Handle<uv_stream_t>(), backlog, &HandleConnectThunk);
}

///   Thunks   ///////////////////////////////////////////////////////////
template <typename T>
void TCPAcceptorTemplate<T>::HandleCloseThunk(uv_handle_t* handle)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->InternalInit();
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

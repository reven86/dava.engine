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

#ifndef __DAVAENGINE_TCPACCEPTOR_H__
#define __DAVAENGINE_TCPACCEPTOR_H__

#include <Debug/DVAssert.h>
#include <Base/Function.h>

#include "TCPAcceptorTemplate.h"

namespace DAVA
{

/*
 Class TCPAcceptor - fully functional implementation which can be used in most cases.
 This class provides ability to call user-specified functional object on incoming TCP connection event
 Functional objects prototypes:
    void ConnectHandlerType(TCPAcceptor* acceptor, int32 error)
*/
class TCPAcceptor : public TCPAcceptorTemplate<TCPAcceptor>
{
private:
    typedef TCPAcceptorTemplate<TCPAcceptor> BaseClassType;

public:
    typedef Function<void(TCPAcceptor* acceptor)>              CloseHandlerType;
	typedef Function<void(TCPAcceptor* acceptor, int32 error)> ConnectHandlerType;

public:
    explicit TCPAcceptor(IOLoop* ioLoop);
    ~TCPAcceptor() {}

    void SetCloseHandler(CloseHandlerType handler);

    int32 AsyncListen(ConnectHandlerType handler, int32 backlog = SOMAXCONN);

    void HandleClose();
    void HandleConnect(int32 error);

private:
    CloseHandlerType   closeHandler;
    ConnectHandlerType connectHandler;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPACCEPTOR_H__

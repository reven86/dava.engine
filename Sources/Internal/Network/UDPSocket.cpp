#include "UDPSocket.h"

namespace DAVA {

UDPSocket::UDPSocket (IOLoop* ioLoop, bool autoDeleteOnCloseFlag) : BaseClassType (ioLoop)
                                                                  , autoDeleteOnClose (autoDeleteOnCloseFlag)
                                                                  , receiveHandler ()
{

}

void UDPSocket::HandleClose ()
{
    if (!(closeHandler == 0))
        closeHandler (this);

    if (autoDeleteOnClose)
        delete this;
}

void UDPSocket::HandleReceive (int error, std::size_t nread, const uv_buf_t* buffer, const Endpoint& endpoint, bool partial)
{
    receiveHandler (this, error, nread, buffer->base, endpoint, partial);
}

void UDPSocket::HandleSend (SendRequest* request, int error)
{
    request->sendHandler (this, error, request->buffer.base);
}

}   // namespace DAVA

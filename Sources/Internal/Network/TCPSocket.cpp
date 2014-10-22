#include "TCPSocket.h"

namespace DAVA {

TCPSocket::TCPSocket (IOLoop* ioLoop, bool autoDeleteOnCloseFlag) : BaseClassType (ioLoop)
                                                                  , autoDeleteOnClose (autoDeleteOnClose)
                                                                  , connectHandler ()
                                                                  , readHandler ()
{

}

void TCPSocket::HandleClose ()
{
    if (autoDeleteOnClose)
        delete this;
}

void TCPSocket::HandleConnect (int error)
{
    connectHandler (this, error);
}

void TCPSocket::HandleRead (int error, size_t nread, const uv_buf_t* buffer)
{
    readHandler (this, error, nread, buffer->base);
}

void TCPSocket::HandleWrite (WriteRequest* request, int error)
{
    request->writeHandler (this, error, request->buffer.base);
}

}   // namespace DAVA

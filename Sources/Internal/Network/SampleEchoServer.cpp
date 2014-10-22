#include <Base/Bind.h>
#include <Base/Function.h>

#include "IOLoop.h"
#include "TCPSocket.h"
#include "SampleEchoServer.h"

namespace DAVA {

SampleEchoServer::SampleEchoServer (IOLoop* ioLoop) : acceptor (ioLoop), udpSocket (ioLoop, false)
{

}

SampleEchoServer::~SampleEchoServer ()
{

}

void SampleEchoServer::Start (unsigned short port)
{
    acceptor.Bind (9999);
    acceptor.AsyncListen (Bind (MakeFunction (this, &SampleEchoServer::HandleConnect),
                                              _1,
                                              _2));

    udpSocket.Bind (9999);
    udpSocket.AsyncReceive (tcpInbuf, sizeof (tcpInbuf) - 1, Bind (MakeFunction (this, &SampleEchoServer::HandleReceive),
                                                                                 DAVA::_1,
                                                                                 DAVA::_2,
                                                                                 DAVA::_3,
                                                                                 DAVA::_4,
                                                                                 DAVA::_5,
                                                                                 DAVA::_6));
}

void SampleEchoServer::Stop ()
{
    acceptor.Close ();
}

char* SampleEchoServer::DuplicateInput (char* input, std::size_t nread)
{
    char* output = new char[nread + 1];
    input[nread] = '\0';
    
    strcpy (output, input);
    return output;
}

void SampleEchoServer::HandleConnect (TCPAcceptor* acceptor, int error)
{
    TCPSocket* socket = new TCPSocket (acceptor->Loop (), true);
    acceptor->Accept (socket);

    socket->AsyncRead (tcpInbuf, sizeof (tcpInbuf) - 1, Bind (MakeFunction (this, &SampleEchoServer::HandleRead),
                                                                            _1,
                                                                            _2,
                                                                            _3,
                                                                            _4));
}

void SampleEchoServer::HandleRead (TCPSocket* socket, int error, std::size_t nread, void* buffer)
{
    if (error == 0)
    {
        socket->AsyncWrite (DuplicateInput (static_cast<char*> (buffer), nread), nread, Bind (MakeFunction (this, &SampleEchoServer::HandleWrite),
                                                                                                            _1,
                                                                                                            _2,
                                                                                                            _3));
    }
    else
        socket->Close ();
}

void SampleEchoServer::HandleWrite (TCPSocket* socket, int error, const void* buffer)
{
    if (error == 0)
        socket->AsyncRead (new char[100], 99, Bind (MakeFunction (this, &SampleEchoServer::HandleRead), _1, _2, _3, _4));
    else
        socket->Close ();
    delete [] static_cast<const char*> (buffer);
}

void SampleEchoServer::HandleReceive (UDPSocket* socket, int error, std::size_t nread, void* buffer, const Endpoint& endpoint, bool partial)
{
    if (error == 0)
    {
        socket->AsyncSend (endpoint, DuplicateInput (static_cast<char*> (buffer), nread), nread, Bind (MakeFunction (this, &SampleEchoServer::HandleSend),
                                                                                                                     _1,
                                                                                                                     _2,
                                                                                                                     _3));
    }
}

void SampleEchoServer::HandleSend (UDPSocket* socket, int error, const void* buffer)
{
    if (error == 0)
    {
        udpSocket.AsyncReceive (tcpInbuf, sizeof (tcpInbuf) - 1, Bind (MakeFunction (this, &SampleEchoServer::HandleReceive),
                                                                                     DAVA::_1,
                                                                                     DAVA::_2,
                                                                                     DAVA::_3,
                                                                                     DAVA::_4,
                                                                                     DAVA::_5,
                                                                                     DAVA::_6));
    }
    delete [] static_cast<const char*> (buffer);
}

}   // namespace DAVA

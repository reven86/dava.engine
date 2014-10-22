#include "TCPAcceptor.h"

namespace DAVA {

TCPAcceptor::TCPAcceptor (IOLoop* ioLoop) : BaseClassType (ioLoop) {}

void TCPAcceptor::HandleConnect (int error)
{
    connectHandler (this, error);
}

}   // namespace DAVA

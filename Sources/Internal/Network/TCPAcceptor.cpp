#include "TCPAcceptor.h"

namespace DAVA {

TCPAcceptor::TCPAcceptor (IOLoop* ioLoop, bool autoDeleteOnCloseFlag) : BaseClassType (ioLoop)
                                                                      , autoDeleteOnClose (autoDeleteOnCloseFlag)
                                                                      , closeHandler ()
                                                                      , connectHandler ()
{

}

void TCPAcceptor::HandleClose ()
{
    if (!(closeHandler == 0))
        closeHandler (this);

    if (autoDeleteOnClose)
        delete this;
}

void TCPAcceptor::HandleConnect (int error)
{
    connectHandler (this, error);
}

}   // namespace DAVA

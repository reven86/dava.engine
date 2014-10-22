#include <cstring>

#include <libuv/uv.h>

#include <Debug/DVAssert.h>

#include "Endpoint.h"

namespace DAVA {

Endpoint::Endpoint (const char* address, unsigned short port) : data ()
{
    InitSockaddrIn (IPAddress::FromString (address).ToULong (), port);
}

Endpoint::Endpoint (const sockaddr* sa)
{
    DVASSERT (sa);
    memcpy (&data, sa, sizeof (data));
}

Endpoint::Endpoint (const sockaddr_in* sin)
{
    DVASSERT (sin);
    memcpy (&data, sin, sizeof (data));
}

bool Endpoint::ToString (char* buffer, std::size_t size) const
{
    DVASSERT (buffer && size > 0);
    if (Address ().ToString (buffer, size))
    {
        char port[20];
        _snprintf (port, COUNT_OF (port), ":%hu", Port ());

        std::size_t addrLen = strlen (buffer);
        std::size_t portLen = strlen (port);
        if (addrLen + portLen < size)
        {
            strcat (buffer + addrLen, port);
            return true;
        }
    }
    return false;
}

String Endpoint::ToString () const
{
    char buf[50];
    if (ToString (buf, COUNT_OF (buf)))
        return String (buf);
    return String ();
}

void Endpoint::InitSockaddrIn (unsigned long addr, unsigned short port)
{
    data.sin_family = AF_INET;
    data.sin_port   = htons (port);
#ifdef WIN32
    data.sin_addr.S_un.S_addr = htonl (addr);
#else
    data.sin_addr.s_addr      = htonl (addr);
#endif
}

unsigned long Endpoint::GetSockaddrAddr () const
{
#ifdef WIN32
    return ntohl (data.sin_addr.S_un.S_addr);
#else
    return ntohl (data.sin_addr.s_addr);
#endif
}

}   // namespace DAVA

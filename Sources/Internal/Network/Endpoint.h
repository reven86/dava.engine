#ifndef __DAVAENGINE_ENDPOINT_H__
#define __DAVAENGINE_ENDPOINT_H__

#include "IPAddress.h"

namespace DAVA {

class Endpoint
{
public:
    explicit Endpoint (unsigned short port = 0) : data ()
    {
        InitSockaddrIn (INADDR_ANY, port);
    }

    Endpoint (const IPAddress& address, unsigned short port) : data ()
    {
        InitSockaddrIn (address.ToULong (), port);
    }

    Endpoint (const char* address, unsigned short port);

    Endpoint (const sockaddr* sa);

    Endpoint (const sockaddr_in* sin);

    IPAddress Address () const
    {
        return IPAddress (GetSockaddrAddr ());
    }

    unsigned short Port () const
    {
        return ntohs (data.sin_port);
    }

    std::size_t Size () const
    {
        return sizeof (data);
    }

    bool ToString (char* buffer, std::size_t size) const;

    String ToString () const;

          sockaddr* CastToSockaddr ()       { return reinterpret_cast<sockaddr*> (&data); }
    const sockaddr* CastToSockaddr () const { return reinterpret_cast<const sockaddr*> (&data); }
    
          sockaddr_in* CastToSockaddrIn ()       { return reinterpret_cast<sockaddr_in*> (&data); }
    const sockaddr_in* CastToSockaddrIn () const { return reinterpret_cast<const sockaddr_in*> (&data); }

private:
    void InitSockaddrIn (unsigned long addr, unsigned short port);

	unsigned long GetSockaddrAddr () const;
	
private:
    sockaddr_in data;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_ENDPOINT_H__

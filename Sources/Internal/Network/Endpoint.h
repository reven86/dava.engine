#ifndef __DAVAENGINE_ENDPOINT_H__
#define __DAVAENGINE_ENDPOINT_H__

#include <cstring>

#include <Debug/DVAssert.h>

#include <libuv/uv.h>

#include "IPAddress.h"

namespace DAVA {

class Endpoint
{
public:
    Endpoint () : data ()
    {
        data.sin_family = AF_INET;
        data.sin_port   = 0;
		SetSockaddrAddr (INADDR_ANY);
    }

    Endpoint (unsigned short port) : data ()
    {
        data.sin_family = AF_INET;
        data.sin_port   = htons (port);
        SetSockaddrAddr (INADDR_ANY);
    }

    Endpoint (const IPAddress& address, unsigned short port)
    {
        data.sin_family = AF_INET;
        data.sin_port   = htons (port);
		SetSockaddrAddr (address.ToULong ());
    }

    Endpoint (const sockaddr* sa)
    {
        DVASSERT (sa);
        memcpy (&data, sa, sizeof (data));
    }

    Endpoint (const sockaddr_in* sin)
    {
        DVASSERT (sin);
        memcpy (&data, sin, sizeof (data));
    }

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

    sockaddr* CastToSockaddr ()
    {
        return reinterpret_cast<sockaddr*> (&data);
    }
    
    const sockaddr* CastToSockaddr () const
    {
        return reinterpret_cast<const sockaddr*> (&data);
    }
    
    sockaddr_in* CastToSockaddrIn ()
    {
        return reinterpret_cast<sockaddr_in*> (&data);
    }
    
    const sockaddr_in* CastToSockaddrIn () const
    {
        return reinterpret_cast<const sockaddr_in*> (&data);
    }

private:
	void SetSockaddrAddr (unsigned long addr)
	{
#ifdef WIN32
		data.sin_addr.S_un.S_addr = htonl (addr);
#else
        data.sin_addr.s_addr      = htonl (addr);
#endif
	}
	
	unsigned long GetSockaddrAddr () const
	{
#ifdef WIN32
		return ntohl (data.sin_addr.S_un.S_addr);
#else
        return ntohl (data.sin_addr.s_addr);
#endif
	}
	
private:
    sockaddr_in data;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_ENDPOINT_H__

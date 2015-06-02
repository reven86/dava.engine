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


#ifndef __DAVAENGINE_ENDPOINT_H__
#define __DAVAENGINE_ENDPOINT_H__

#include <Base/BaseTypes.h>

#include <Network/Base/IPAddress.h>

namespace DAVA
{
namespace Net
{

/*
 Class Endpoint represents a endpoint - simple and clear description :)
 Endpoint is a pair of IP address and port number.
*/
class Endpoint
{
public:
    // Construct using port number, address part wil be any address (INADDR_ANY)
    explicit Endpoint(uint16 port = 0);
    // Construct using IPAddress object and port number
    Endpoint(const IPAddress& address, uint16 port);
    // Construct using IP address represented as string (see IPAddress::FromString) and port number
    Endpoint(const char8* address, uint16 port);
    // Construct using low level structures, purpose is to closely interact with libuv
    Endpoint(const sockaddr* sa);
    Endpoint(const sockaddr_in* sin);

    IPAddress Address() const;
    uint16 Port() const;
    size_t Size() const;

    bool ToString(char8* buffer, size_t size) const;
    String ToString() const;

    // These methods are used for close interaction with libuv
    sockaddr* CastToSockaddr();
    const sockaddr* CastToSockaddr() const;
    
    sockaddr_in* CastToSockaddrIn();
    const sockaddr_in* CastToSockaddrIn() const;

    friend bool operator == (const Endpoint& left, const Endpoint& right);
    friend bool operator < (const Endpoint& left, const Endpoint& right);

private:
    void InitSockaddrIn(uint32 addr, uint16 port);
	uint32 GetSockaddrAddr() const;
	
private:
    sockaddr_in data;
};

//////////////////////////////////////////////////////////////////////////
inline Endpoint::Endpoint(uint16 port) : data()
{
    InitSockaddrIn(INADDR_ANY, port);
}

inline Endpoint::Endpoint(const IPAddress& address, uint16 port) : data()
{
    InitSockaddrIn(address.ToUInt(), port);
}

inline IPAddress Endpoint::Address() const
{
    return IPAddress(GetSockaddrAddr());
}

inline uint16 Endpoint::Port() const
{
    return ntohs(data.sin_port);
}

inline size_t Endpoint::Size() const
{
    return sizeof(data);
}

inline sockaddr* Endpoint::CastToSockaddr() { return reinterpret_cast<sockaddr*>(&data); }
inline const sockaddr* Endpoint::CastToSockaddr() const { return reinterpret_cast<const sockaddr*>(&data); }

inline sockaddr_in* Endpoint::CastToSockaddrIn() { return reinterpret_cast<sockaddr_in*>(&data); }
inline const sockaddr_in* Endpoint::CastToSockaddrIn() const { return reinterpret_cast<const sockaddr_in*>(&data); }

inline bool operator == (const Endpoint& left, const Endpoint& right)
{
    return left.Address() == right.Address() && left.Port() == right.Port();
}

inline bool operator < (const Endpoint& left, const Endpoint& right)
{
    return left.Address() == right.Address() ? left.Port() < right.Port()
                                             : left.Address() < right.Address();
}

}   // namespace Net
}	// namespace DAVA

#endif  // __DAVAENGINE_ENDPOINT_H__

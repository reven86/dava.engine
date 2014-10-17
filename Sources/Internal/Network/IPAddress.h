#ifndef __DAVAENGINE_IPADDRESS_H__
#define __DAVAENGINE_IPADDRESS_H__

#include <cstring>

#include <libuv/uv.h>

namespace DAVA {

class IPAddress
{
public:
    IPAddress () : addr (0) {}
    IPAddress (unsigned long address) : addr (htonl (address)) {}

    unsigned long ToULong () const
    {
        return ntohl (addr);
    }

    bool ToString (char* buffer, std::size_t size) const;

    String ToString () const;

    static IPAddress FromString (const char* addr);

private:
    unsigned long addr;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_IPADDRESS_H__

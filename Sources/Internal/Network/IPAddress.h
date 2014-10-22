#ifndef __DAVAENGINE_IPADDRESS_H__
#define __DAVAENGINE_IPADDRESS_H__

#include <libuv/uv.h>

namespace DAVA {

class IPAddress
{
public:
    IPAddress (unsigned long address = 0) : addr (htonl (address)) {}

    unsigned long ToULong () const
    {
        return ntohl (addr);
    }

    bool IsUnspecified () const
    {
        return 0 == addr;
    }

    bool ToString (char* buffer, std::size_t size) const;

    String ToString () const;

    static IPAddress FromString (const char* addr);

private:
    unsigned long addr;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_IPADDRESS_H__

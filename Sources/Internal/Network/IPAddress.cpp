#include <Base/BaseTypes.h>
#include <Debug/DVAssert.h>

#include "Endpoint.h"
#include "IPAddress.h"

namespace DAVA {

bool IPAddress::ToString (char* buffer, std::size_t size) const
{
    DVASSERT (buffer && size > 0);
    return 0 == uv_ip4_name (Endpoint (*this, 0).CastToSockaddrIn (), buffer, size);
}

String IPAddress::ToString () const
{
    char buf[50];
    if (ToString (buf, COUNT_OF (buf)))
        return String (buf);
    return String ();
}

IPAddress IPAddress::FromString (const char* addr)
{
    DVASSERT (addr);

    Endpoint endp;
    if (0 == uv_ip4_addr (addr, 0, endp.CastToSockaddrIn ()))
        return endp.Address ();
    return IPAddress ();
}

}   // namespace DAVA

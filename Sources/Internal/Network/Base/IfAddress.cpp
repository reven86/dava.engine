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

#include "Base/BaseTypes.h"
#include <libuv/uv.h>

#include "Network/Base/Endpoint.h"
#include "Network/Base/IfAddress.h"

namespace DAVA
{
namespace Net
{

Vector<IfAddress> IfAddress::GetInstalledInterfaces(bool withInternal)
{
    Vector<IfAddress> result;
    int n = 0;
    uv_interface_address_t* ifaddr = NULL;
    int error = uv_interface_addresses(&ifaddr, &n);
    if (0 == error)
    {
        result.reserve(n);
        for (int i = 0;i < n;++i)
        {
            if (ifaddr[i].address.address4.sin_family != AF_INET) continue; // For now list only IPv4 addresses

            if (true == Endpoint(&ifaddr[i].address.address4).Address().IsUnspecified()) continue;   // List only interfaces with specified IP-address

            if (false == withInternal && ifaddr[i].is_internal != 0) continue;  // Do not list internal interfaces

            PhysAddress physAddr;
            Endpoint addr(&ifaddr[i].address.address4);
            Endpoint mask(&ifaddr[i].netmask.netmask4);
            bool isInternal = ifaddr[i].is_internal != 0;
            Memcpy(physAddr.data, ifaddr[i].phys_addr, 6);

            result.push_back(IfAddress(isInternal, physAddr, addr.Address(), mask.Address()));
        }
        uv_free_interface_addresses(ifaddr, n);
    }
    return result;
}

}   // namespace Net
}   // namespace DAVA

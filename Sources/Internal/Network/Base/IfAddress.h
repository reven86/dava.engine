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

#ifndef __DAVAENGINE_IFADDRESS_H__
#define __DAVAENGINE_IFADDRESS_H__

#include <Base/BaseTypes.h>

#include <Network/Base/IPAddress.h>

namespace DAVA
{
namespace Net
{

/*
 Class IfAddress represents network interface address.
 Also it provides static member for retrieving addresses of all installed interfaces
*/
class IfAddress
{
public:
    struct PhysAddress
    {
        uint8 data[6];
    };

public:
    IfAddress();
    IfAddress(bool isInternal_, const PhysAddress& physAddr_, const IPAddress& addr_, const IPAddress& mask_);

    bool IsInternal() const;
    const PhysAddress& PhysicalAddress() const;
    const IPAddress& Address() const;
    const IPAddress& Mask() const;

    static Vector<IfAddress> GetInstalledInterfaces(bool withInternal = false);

    friend bool operator < (const IfAddress& left, const IfAddress& right);

private:
    bool isInternal;
    PhysAddress physAddr;
    IPAddress addr;
    IPAddress mask;
};

//////////////////////////////////////////////////////////////////////////
inline IfAddress::IfAddress()
    : isInternal(true)
{
    Memset(&physAddr, 0, sizeof(PhysAddress));
}

inline IfAddress::IfAddress(bool isInternal_, const PhysAddress& physAddr_, const IPAddress& addr_, const IPAddress& mask_)
    : isInternal(isInternal_)
    , physAddr(physAddr_)
    , addr(addr_)
    , mask(mask_)
{

}

inline bool IfAddress::IsInternal() const
{
    return isInternal;
}

inline const IfAddress::PhysAddress& IfAddress::PhysicalAddress() const
{
    return physAddr;
}

inline const IPAddress& IfAddress::Address() const
{
    return addr;
}

inline const IPAddress& IfAddress::Mask() const
{
    return mask;
}

inline bool operator < (const IfAddress& left, const IfAddress& right)
{
    // Order by address, internal interfaces moves to back
    return left.isInternal == right.isInternal ? left.addr < right.addr
                                               : left.isInternal == right.isInternal;
}

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_IFADDRESS_H__

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


#ifndef __DAVAENGINE_UAP_NETWORK_HELPER_H__
#define __DAVAENGINE_UAP_NETWORK_HELPER_H__

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN_UAP__)
{

#include "Network/Base/Endpoint.h"
#include "Network/NetworkCommon.h"

    namespace DAVA
    {
    class UAPNetworkHelper
    {
    public:
        static const uint16 UAP_DESKTOP_TCP_PORT = 7777;
        static const uint16 UAP_MOBILE_TCP_PORT = 1911;
        static const char* UAP_IP_ADDRESS;

        static Net::eNetworkRole GetCurrentNetworkRole();
        static Net::Endpoint GetCurrentEndPoint();
        static Net::Endpoint GetEndPoint(Net::eNetworkRole role);
    };

    } // namespace DAVA
}

#endif
#endif // __DAVAENGINE_UAP_NETWORK_HELPER_H__

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


#ifndef __DAVAENGINE_SIMPLE_NETWORKING_COMMON_H__
#define __DAVAENGINE_SIMPLE_NETWORKING_COMMON_H__

#include "Base/Platform.h"

namespace DAVA
{

#ifdef __DAVAENGINE_WINDOWS__

using socket_t = SOCKET;
#define DV_SD_BOTH SD_BOTH
#define DV_INVALID_SOCKET INVALID_SOCKET

inline bool CheckSocketResult(int result)
{
    return result != SOCKET_ERROR;
}

inline void CloseSocket(socket_t sock)
{
    ::closesocket(sock);
}

#else

using socket_t = int;
#define DV_SD_BOTH SHUT_RDWR
#define DV_INVALID_SOCKET -1

inline bool CheckSocketResult(int result)
{
    return result == 0;
}

inline void CloseSocket(socket_t sock)
{
    ::close(sock);
}

#endif
    
}  // namespace DAVA

#endif  // __DAVAENGINE_SIMPLE_NETWORKING_COMMON_H__
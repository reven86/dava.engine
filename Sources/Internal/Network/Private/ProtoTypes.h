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


#ifndef __DAVAENGINE_PROTOTYPES_H__
#define __DAVAENGINE_PROTOTYPES_H__

#include <Base/BaseTypes.h>

namespace DAVA
{
namespace Net
{

struct ProtoHeader
{
    uint16 frameSize;           // Frame length: header + data
    uint16 frameType;           // Frame type
    uint32 channelId;           // Channel identifier
    uint32 packetId;            // Packet Id for acknoledgements
    uint32 totalSize;           // Total size of user data
};

const size_t PROTO_MAX_FRAME_SIZE = 1024 * 64 - 1;
const size_t PROTO_MAX_FRAME_DATA_SIZE = PROTO_MAX_FRAME_SIZE - sizeof(ProtoHeader);

enum eProtoFrameType
{
    TYPE_DATA,              // Frame carries user data
    TYPE_CHANNEL_QUERY,     // Control frame: check whether channel is available
    TYPE_CHANNEL_ALLOW,     // Control frame: answer to CHANNEL_QUERY frame: channel is available
    TYPE_CHANNEL_DENY,      // Control frame: answer to CHANNEL_QUERY frame: channel is not available
    TYPE_PING,              // Control frame: keep-alive request
    TYPE_PONG,              // Control frame: answer to PING frame
    TYPE_DELIVERY_ACK,      // Control frame: user data packet delivered

    TYPE_FIRST = TYPE_DATA,
    TYPE_CONTROL_FIRST = TYPE_CHANNEL_QUERY,
    TYPE_LAST  = TYPE_DELIVERY_ACK
};

enum eProtoFrameFlags
{
    FRAME_NO_DELIVERY_ACK = 0x01
};

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_PROTOTYPES_H__

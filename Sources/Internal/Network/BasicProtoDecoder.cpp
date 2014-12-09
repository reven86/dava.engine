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

#include <Debug/DVAssert.h>

#include "BasicProtoDecoder.h"

namespace DAVA
{
namespace Net
{

BasicProtoDecoder::eStatus BasicProtoDecoder::Decode(uint8* buffer, std::size_t bufferSize, DecodeResult* result)
{
    DVASSERT(buffer != NULL && result != NULL);

    eStatus status = PACKET_INCOMPLETE;
    if (bufferSize >= sizeof(BasicProtoHeader))
    {
        BasicProtoHeader* header = reinterpret_cast<BasicProtoHeader*>(buffer);
        DVASSERT(header->packetSize >= sizeof(BasicProtoHeader) && TYPE_FIRST <= header->packetType && header->packetType <= TYPE_LAST);
        if (header->packetSize >= sizeof(BasicProtoHeader))
        {
            switch(header->packetType)
            {
            case TYPE_DATA:
                if (bufferSize >= header->packetSize)
                {
                    result->decodedSize    = header->packetSize;
                    result->packetDataSize = result->decodedSize - sizeof(BasicProtoHeader);
                    result->packetData     = buffer + sizeof(BasicProtoHeader);
                    result->header         = header;
                    status = PACKET_OK;
                }
                break;
            case TYPE_PING:
            case TYPE_PONG:
            case TYPE_ACK:
                if (sizeof(BasicProtoHeader) == header->packetSize)
                {
                    result->decodedSize    = header->packetSize;
                    result->packetDataSize = 0;
                    result->packetData     = NULL;
                    result->header         = header;
                    status = PACKET_OK;
                }
                else
                    status = PACKET_INVALID;
                break;
            default:
                DVASSERT(0);
                status = PACKET_INVALID;
                break;
            }
        }
        else
        {
            status = PACKET_INVALID;
        }
    }
    return status;
}

size_t BasicProtoDecoder::Encode(BasicProtoHeader* header, uint32 channelId, uint32 packetId, size_t totalSize, size_t encodedSize)
{
    DVASSERT(header != NULL && totalSize > 0 && encodedSize < totalSize);

    // Compute size of user data that can fit in packet
    size_t sizeToEncode = Min(totalSize - encodedSize, MAX_DATA_SIZE);

    header->packetSize  = static_cast<uint16>(sizeof(BasicProtoHeader) + sizeToEncode);
    header->packetType  = TYPE_DATA;
    header->totalSize   = totalSize;
    header->channelId   = channelId;
    header->packetId    = packetId;
    return sizeToEncode;
}

size_t BasicProtoDecoder::EncodePing(BasicProtoHeader* header)
{
    DVASSERT(header != NULL);
    header->packetSize = sizeof(BasicProtoHeader);
    header->packetType = TYPE_PING;
    header->totalSize  = 0;
    header->channelId  = 0;
    header->packetId   = 0;
    return sizeof(BasicProtoHeader);
}

size_t BasicProtoDecoder::EncodePong(BasicProtoHeader* header)
{
    DVASSERT(header != NULL);
    header->packetSize = sizeof(BasicProtoHeader);
    header->packetType = TYPE_PONG;
    header->totalSize  = 0;
    header->channelId  = 0;
    header->packetId   = 0;
    return sizeof(BasicProtoHeader);
}

size_t BasicProtoDecoder::EncodeAck(BasicProtoHeader* header, uint32 channelId, uint32 packetId)
{
    DVASSERT(header != NULL);
    header->packetSize = sizeof(BasicProtoHeader);
    header->packetType = TYPE_ACK;
    header->totalSize  = 0;
    header->channelId  = channelId;
    header->packetId   = packetId;
    return sizeof(BasicProtoHeader);
}

}   // namespace Net
}   // namespace DAVA

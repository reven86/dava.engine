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

BasicProtoDecoder::eStatus BasicProtoDecoder::Decode(uint8* buffer, std::size_t bufferSize, DecodeResult* result)
{
    DVASSERT(buffer != NULL && result != NULL);

    eStatus status = PACKET_INCOMPLETE;
    if (bufferSize >= sizeof(BasicProtoHeader))
    {
        BasicProtoHeader* header = reinterpret_cast<BasicProtoHeader*>(buffer);
        DVASSERT(header->packetSize >= sizeof(BasicProtoHeader) && PROTO_MAGIC == header->packetMagic);
        if (header->packetSize >= sizeof(BasicProtoHeader) && PROTO_MAGIC == header->packetMagic)
        {
            if (bufferSize >= header->packetSize)
            {
                result->decodedSize    = header->packetSize;
                result->packetDataSize = result->decodedSize - sizeof(BasicProtoHeader);
                result->packetData     = buffer + sizeof(BasicProtoHeader);
                result->header         = header;
                status = PACKET_OK;
            }
        }
        else
        {
            status = PACKET_INVALID;
        }
    }
    return status;
}

size_t BasicProtoDecoder::Encode(BasicProtoHeader* header, uint32 channelId, size_t totalSize, size_t encodedSize)
{
    DVASSERT(header != NULL && totalSize > 0 && encodedSize < totalSize);

    // Compute size of user data that can fit in packet
    size_t sizeToEncode = Min(totalSize - encodedSize, MAX_DATA_SIZE);

    header->packetSize  = static_cast<uint16>(sizeof(BasicProtoHeader) + sizeToEncode);
    header->packetMagic = PROTO_MAGIC;
    header->totalSize   = totalSize;
    header->channelId   = channelId;
    return sizeToEncode;
}

}   // namespace DAVA

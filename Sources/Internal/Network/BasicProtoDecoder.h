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

#ifndef __DAVAENGINE_BASICPROTODECODER_H__
#define __DAVAENGINE_BASICPROTODECODER_H__

#include <Base/BaseTypes.h>

#include <Network/Base/Buffer.h>

namespace DAVA
{

/*
 Basic transport protocol is intended to carry data over network
 On sender side data is broken into logical packets which are transfered over network.
 On receiver side incoming data chunks are restored into logical packets and data is delivered to clients
*/

/*
 Each logical packet consists of two part: header and client data
*/
struct BasicProtoHeader
{
    uint16 packetSize;          // Packet length: header + data
    uint16 packetMagic;         // Magic number to distinguish packets
    uint32 totalSize;           // Total size of user data
    uint32 channelId;           // Channel identifier
};

class BasicProtoDecoder
{
public:
    static const size_t MAX_PACKET_SIZE = 1024 * 64 - 1;
    static const size_t MAX_DATA_SIZE   = MAX_PACKET_SIZE - sizeof(BasicProtoHeader);
    static const uint16 PROTO_MAGIC     = 0xBAAB;

    // Possible status of packet decoding
    enum eStatus
    {
        STATUS_OK,          // packet decoded
        STATUS_INCOMPLETE,  // packet incomplete, need more data
        STATUS_INVALID      // packet invalid, e.g. magic unrecognizable number
    };

    // Result of buffer decoding
    struct DecodeResult
    {
        size_t decodedSize;
        size_t packetDataSize;
        uint8* packetData;
        BasicProtoHeader* header;
    };

public:
    static eStatus Decode(uint8* buffer, size_t bufferSize, DecodeResult* result);
    static size_t Encode(BasicProtoHeader* header, uint32 channelId, size_t totalSize, size_t encodedSize);
};

}   // namespace DAVA

#endif  // __DAVAENGINE_BASICPROTODECODER_H__

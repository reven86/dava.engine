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


#ifndef __DAVAENGINE_GHOSTPROTODECODER_H__
#define __DAVAENGINE_GHOSTPROTODECODER_H__

#include <Network/Private/ProtoTypes.h>

namespace DAVA
{
namespace Net
{

class ProtoDecoder
{
public:
    enum eDecodeStatus
    {
        DECODE_OK,          // Frame or data packet decoded
        DECODE_INCOMPLETE,  // Frame or data packet is incomplete, need more data
        DECODE_INVALID      // Frame is invalid
    };

    struct DecodeResult
    {
        size_t decodedSize;     // Number of bytes consumed from input buffer
        uint32 type;
        uint32 channelId;
        uint32 packetId;
        size_t dataSize;
        uint8* data;            // Pointer to user data of data packet
    };

public:
    ProtoDecoder();

    eDecodeStatus Decode(const void* buffer, size_t length, DecodeResult* result);
    size_t EncodeDataFrame(ProtoHeader* header, uint32 channelId, uint32 packetId, size_t packetSize, size_t encodedSize) const;
    size_t EncodeControlFrame(ProtoHeader* header, uint32 type, uint32 channelId, uint32 packetId) const;

private:
    eDecodeStatus ProcessDataFrame(ProtoHeader* header, DecodeResult* result);
    eDecodeStatus ProcessControlFrame(ProtoHeader* header, DecodeResult* result);

    eDecodeStatus GatherHeader(const void* buffer, size_t length, DecodeResult* result);
    eDecodeStatus GatherFrame(const void* buffer, size_t length, DecodeResult* result);
    eDecodeStatus CheckHeader(const ProtoHeader* header) const;

private:
    size_t totalDataSize;
    size_t accumulatedSize;
    Vector<uint8> accum;

    uint8 curFrame[PROTO_MAX_FRAME_SIZE];
    size_t curFrameSize;
};

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_ _H__

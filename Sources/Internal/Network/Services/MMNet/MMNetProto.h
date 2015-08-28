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

#ifndef __DAVAENGINE_MMNETPROTO_H__
#define __DAVAENGINE_MMNETPROTO_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
namespace Net
{

namespace MMNetProto
{

enum ePacketType
{
    TYPE_REQUEST_TOKEN = 0,
    TYPE_REQUEST_SNAPSHOT,
    
    TYPE_REPLY_TOKEN = 100,
    TYPE_REPLY_SNAPSHOT,
    
    TYPE_AUTO_STAT = 200,
    TYPE_AUTO_SNAPSHOT
};

enum eStatus
{
    STATUS_SUCCESS = 0,     // Everything is ok
    STATUS_ERROR,           // Some kind of error has occured
    STATUS_TOKEN,           // Request TYPE_REQUEST_TOKEN has not been performed
    STATUS_BUSY             // Object is busy and cannot fulfil the request
};

struct PacketHeader
{
    uint32 length;          // Total length of packet including header
    uint16 type;            // Type of command/reply encoded in packet
    uint16 status;          // Result of executing command
    uint16 itemCount;       // Number of data items in packet if applied
    uint16 flags;
    uint32 token;           // Connection token
};
static_assert(sizeof(PacketHeader) == 16, "sizeof(MMNetProto::PacketHeader) != 16");

struct PacketParamSnapshot
{
    uint32 flags;           // Flags: 0 - snapshot unpacked, 1 - snapshot packed
    uint32 snapshotSize;    // Total size of unpacked memory snapshot
    uint32 chunkOffset;     // Chunk byte offset in unpacked snapshot
    uint32 chunkSize;       // Chunk size in unpacked snapshot
};
static_assert(sizeof(PacketParamSnapshot) == 16, "sizeof(MMNetProto::PacketParamSnapshot) != 16");

//////////////////////////////////////////////////////////////////////////
class Packet
{
public:
    Packet() = default;
    Packet(size_t dataSize);
    Packet(Packet&& other);
    Packet& operator = (Packet&& other);

    const uint8* PlainBytes() const;
    size_t PlainSize() const;

    PacketHeader* Header();
    template<typename T>
    T* Data(const size_t offset = 0);

private:
    Vector<uint8> plainBytes;
};

//////////////////////////////////////////////////////////////////////////
inline Packet::Packet(size_t dataSize)
    : plainBytes(sizeof(PacketHeader) + dataSize)
{}

inline Packet::Packet(Packet&& other)
    : plainBytes(std::move(other.plainBytes))
{}

inline Packet& Packet::operator = (Packet&& other)
{
    if (this != &other)
    {
        plainBytes = std::move(other.plainBytes);
    }
    return *this;
}

inline const uint8* Packet::PlainBytes() const
{
    return &plainBytes[0];
}

inline size_t Packet::PlainSize() const
{
    return plainBytes.size();
}

inline PacketHeader* Packet::Header()
{
    return reinterpret_cast<PacketHeader*>(&*plainBytes.begin());
}

template<typename T>
inline T* Packet::Data(const size_t offset)
{
    return OffsetPointer<T>(&*plainBytes.begin(), sizeof(PacketHeader) + offset);
}

}   // namespace MMNetProto
}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_MMNETPROTO_H__

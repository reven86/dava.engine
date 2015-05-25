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
    TYPE_REQUEST_DUMP,
    TYPE_REQUEST_CANCEL,
    
    TYPE_REPLY_TOKEN = 100,
    TYPE_REPLY_DUMP,
    TYPE_REPLY_CANCEL,
    
    TYPE_AUTO_STAT = 200,
    TYPE_AUTO_DUMP,
    
    TYPE_TOKEN = 0,         // Request connection token
    TYPE_DUMP,              // Request to make memory dump or memory dump auto send
    TYPE_STAT               // Memory usage auto send
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

struct PacketParamDump
{
    uint32 flags;           // Flags: 0 - dump unpacked, 1 - dump packed
    uint32 dumpSize;        // Total size of unpacked dump
    uint32 chunkOffset;     // Chunk byte offset in unpacked dump
    uint32 chunkSize;       // Chunk size in unpacked dump
};
static_assert(sizeof(PacketParamDump) == 16, "sizeof(MMNetProto::PacketParamDump) != 16");

////////////////////////////////////////////////
struct Header
{
    uint32 type;
    uint32 status;
    uint32 length;          // Size of data sending with header
    uint32 totalLength;     // Total size of data
    uint32 itemCount;       // Number of items
    uint32 specific[3];
};
static_assert(sizeof(Header) == 32, "sizeof(Header) != 32");

struct HeaderInit
{
    uint32 type;
    uint32 status;
    uint32 length;
    uint32 totalLength;
    uint32 itemCount;
    uint32 sessionId;
    uint32 unused[2];
};
static_assert(sizeof(HeaderInit) == sizeof(Header), "sizeof(HeaderInit) != sizeof(Header)");

struct HeaderStat
{
    uint32 type;
    uint32 status;
    uint32 length;
    uint32 totalLength;
    uint32 itemCount;
    uint32 unused[3];
};
static_assert(sizeof(HeaderStat) == sizeof(Header), "sizeof(HeaderStat) != sizeof(Header)");

struct HeaderDump
{
    uint32 type;
    uint32 status;
    uint32 length;
    uint32 totalLength;
    uint32 itemCount;
    uint32 isPacked;            // 0 - dump not packed, 1 - dump - packed
    uint32 unused[2];
};
static_assert(sizeof(HeaderDump) == sizeof(Header), "sizeof(HeaderDump) != sizeof(Header)");

}   // namespace MMNetProto

enum class eMMProtoCmd
{
    INIT_COMM,
    CUR_STAT,
    DUMP,
    DUMP_BEGIN,
    DUMP_CHUNK,
    DUMP_END
};

enum class eMMProtoStatus
{
    ACK,
    DENY,
};

struct MMProtoHeader
{
    uint32 sessionId;
    uint32 cmd;             // Command
    uint32 status;          // 
    uint32 length;          // Length of data attached to command, or zero if no data
};

static_assert(sizeof(MMProtoHeader) == 16, "sizeof(MMProtoHeader) == 16");

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_MMNETPROTO_H__

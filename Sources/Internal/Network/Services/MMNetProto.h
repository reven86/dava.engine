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
    TYPE_INIT = 0,
    TYPE_STAT,
    TYPE_DUMP
};

enum eStatus
{
    STATUS_OK = 0,
    STATUS_DENY,
    STATUS_ERROR,
    //STATUS_DISABLED
};

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

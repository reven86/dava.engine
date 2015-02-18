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

#ifndef __DAVAENGINE_MEMPROFILERTYPES_H__
#define __DAVAENGINE_MEMPROFILERTYPES_H__

#include "Base/BaseTypes.h"

namespace DAVA
{

/*
 GeneralAllocStat - general memory statistics
*/
struct GeneralAllocStat
{
    uint32 ghostBlockCount;     // Number of blocks allocated bypassing memory manager
    uint32 ghostSize;           // Size of bypassed blocks
    uint32 padding[2];
};

static_assert(sizeof(GeneralAllocStat) % 16 == 0, "sizeof(GeneralAllocStat) % 16 == 0");

/*
 AllocPoolStat - memory statistics calculated ever for every allocation pool
*/
struct AllocPoolStat
{
    uint32 allocByApp;      // Number of bytes allocated by application
    uint32 allocTotal;      // Total allocated bytes: allocByApp + overhead imposed by memory manager and aligning
    uint32 blockCount;      // Number of allocated blocks
    uint32 maxBlockSize;    // Max allocated block size
};

static_assert(sizeof(AllocPoolStat) % 16 == 0, "sizeof(AllocPoolStat) % 16 == 0");

/*
 MMTagStack holds stack of tags
*/
struct MMTagStack
{
    static const size_t MAX_DEPTH = 8;
    
    uint32 depth;
    uint32 padding[3];
    uint32 stack[MAX_DEPTH];
    uint32 begin[MAX_DEPTH];
};

static_assert(sizeof(MMTagStack) % 16 == 0, "sizeof(MMTagStack) % 16 == 0");

/*
 MMItemName is used to store name of tag, allocation pool, etc
 It's desirable that name is not too long and size of struct is multiple of 16
*/
struct MMItemName
{
    static const size_t NAME_LENGTH = 16;
    
    char8 name[NAME_LENGTH];
};

static_assert(sizeof(MMItemName) % 16 == 0, "sizeof(MMItemName) % 16 == 0");

/*
 MMStatConfig represents memory manager configuration: number and names of registered tags and allocation pools
*/
struct MMStatConfig
{
    uint32 maxTagCount;         // Max possible tag count
    uint32 maxAllocPoolCount;   // Max possible count of allocation pools
    uint32 tagCount;            // Number of registered tags
    uint32 allocPoolCount;      // Number of registered allocation pools
    MMItemName names[1];        // Item names: elements in range [0, tagCount) - tag names
                                //             elements in range [tagCount, tagCount + allocPoolCount) - allocation pool names
};

static_assert(sizeof(MMStatConfig) % 16 == 0, "sizeof(MMStatConfig) % 16 == 0");

/*
 MMStat represents current memory allocation statistics
*/
struct MMStat
{
    uint64 timestamp;       // Some kind of timestamp (not necessary real time), filled by statistic transmitter
    uint32 allocCount;      // Total number of allocation occured
    uint32 allocPoolCount;  // Duplicate from MMStatConfig::allocPoolCount
    MMTagStack tags;
    GeneralAllocStat generalStat;
    AllocPoolStat poolStat[1];  // Array size should be calculated as: tags.depth * allocPoolCount
};

static_assert(sizeof(MMStat) % 16 == 0, "sizeof(MMStat) % 16 == 0");

struct MMBacktrace
{
    uint64 frames[16];
};

struct MMSymbol
{
    static const size_t NAME_LENGTH = 136;
    uint64 addr;
    char8 name[NAME_LENGTH];
};

static_assert(sizeof(MMSymbol) % 16 == 0, "sizeof(MMSymbol) % 16 == 0");

struct MMBlock
{
    uint64 addr;
    uint32 allocByApp;
    uint32 allocTotal;
    uint32 pool;
    uint32 orderNo;
    uint32 padding[2];
    MMBacktrace backtrace;
};

static_assert(sizeof(MMBlock) % 16 == 0, "sizeof(MMBlock) % 16 == 0");

struct MMDump
{
    uint64 timestampBegin;
    uint64 timestampEnd;
    uint32 blockCount;
    uint32 nameCount;
    uint32 blockBegin;
    uint32 blockEnd;
    //uint32 tag;
    //uint32 x;
    MMBlock blocks[1];
};

static_assert(sizeof(MMDump) % 16 == 0, "sizeof(MMDump) % 16 == 0");

}   // namespace DAVA

#endif  // __DAVAENGINE_MEMPROFILERTYPES_H__

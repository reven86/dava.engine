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

struct MMConst
{
    static const size_t BACKTRACE_DEPTH = 16;
    static const size_t MAX_TAG_DEPTH = 8;              // Maximum depth of tag stack
    static const size_t DEFAULT_TAG = 0;                // Default tag which corresponds to whole application time line
    
    static const size_t MAX_ALLOC_POOL_COUNT = 8;       // Max supported count of allocation pools
    static const size_t MAX_TAG_COUNT = 8;              // Max supported count of tags
    static const size_t MAX_NAME_LENGTH = 16;           // Max length of name: tag, allocation type, counter
    
    enum {
        DUMP_REQUEST_USER,
        DUMP_REQUEST_TAG,
        DUMP_REQUEST_CHECKPOINT
    };
};

/*
 GeneralAllocStat - general memory statistics
*/
struct GeneralAllocStat
{
    uint32 allocInternal;       // Size of memory allocated for memory manager internal use: symbol table, etc
    uint32 internalBlockCount;  // Number of internal memory blocks
    uint32 ghostBlockCount;     // Number of blocks allocated bypassing memory manager
    uint32 ghostSize;           // Size of bypassed memory
    uint32 realSize;
    //padding
    uint32 padding[3];
};

static_assert(sizeof(GeneralAllocStat) % 16 == 0, "sizeof(GeneralAllocStat) % 16 == 0");

/*
 AllocPoolStat - memory statistics calculated for every allocation pool
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
    uint32 depth;
    uint32 padding[3];
    uint32 stack[MMConst::MAX_TAG_DEPTH];
    uint32 begin[MMConst::MAX_TAG_DEPTH];
};

static_assert(sizeof(MMTagStack) % 16 == 0, "sizeof(MMTagStack) % 16 == 0");

/*
 MMItemName is used to store name of tag, allocation pool, etc
 It's desirable that name is not too long and size of struct is multiple of 16
*/
struct MMItemName
{
    char8 name[MMConst::MAX_NAME_LENGTH];
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
    uint32 markCount;
    uint32 allocPoolCount;      // Number of registered allocation pools
    uint32 padding[3];
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
    uint32 hash;
    uint32 padding[3];
    uint64 frames[MMConst::BACKTRACE_DEPTH];
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
    uint32 backtraceHash;
    uint32 padding;
};

static_assert(sizeof(MMBlock) % 16 == 0, "sizeof(MMBlock) % 16 == 0");

struct MMDump
{
    uint64 timestampBegin;      //
    uint64 timestampEnd;        //
    uint32 blockCount;          // Number of blocks in dump
    uint32 backtraceCount;      // Number of backtraces in dump
    uint32 symbolCount;         // Number of symbols in dump
    uint32 blockBegin;          // Order number of first block in dump
    uint32 blockEnd;            // Order number of last block in dump
    uint32 type;                // Dump type: user request, tag ended, checkpoint
    uint32 tag;                 // What tag has ended
    uint32 padding;
    // Memory layout after this struct
    //MMBlock blocks[];
    //MMBacktrace backtraces[];
    //MMSymbol symbols[];
};

static_assert(sizeof(MMDump) % 16 == 0, "sizeof(MMDump) % 16 == 0");

}   // namespace DAVA

#endif  // __DAVAENGINE_MEMPROFILERTYPES_H__

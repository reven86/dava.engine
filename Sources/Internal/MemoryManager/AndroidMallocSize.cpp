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


#include "Base/BaseTypes.h"     // For platfrom recognition

#if defined(__DAVAENGINE_ANDROID__)

#include <cstddef>

// Internal android memory structures taken from Android Open Source Project (glibc/malloc/malloc.c)
struct malloc_chunk
{
    size_t        prev_size;    // Size of previous chunk (if free)
    size_t        size;         // Size in bytes, also low order 3 bits contain some flags

    malloc_chunk* fd;           // Double links (forward and backward) -- used only if free
    malloc_chunk* bk;
};

size_t AndroidMallocSize(void* ptr)
{
    /*
    This is how memory chunks organized in memory, we are only care of allocated chunks
    chunk->     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                |             Size of previous chunk, if allocated            | |
                +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                |             Size of chunk, in bytes                     |A|M|P|
          mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                |             User data starts here...                          .
                .                                                               .
                .                                                               .
    So to get malloc_chunk beginning we should move pointer backward by two size_t
    To get chunk size we should clear flag bits
    */

    // Mask to extract chunk size
    const size_t EXTRACT_SIZE_MASK = ~0x07;

    malloc_chunk* chunk = reinterpret_cast<malloc_chunk*>(static_cast<char*>(ptr) - sizeof(size_t) * 2);
    return chunk->size & EXTRACT_SIZE_MASK;
}

#endif  // __DAVAENGINE_ANDROID__

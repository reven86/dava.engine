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

#include "Base/BlockAllocator.h"

using namespace DAVA;

static std::unordered_map<char*, size_t> customAllocations;

char* BlockAllocatorBase::AllocateTracked(uint32 size)
{
    char* ptr = reinterpret_cast<char*>(malloc(size));
    customAllocations[ptr] = size;
    return ptr;
}

void BlockAllocatorBase::FreeTracked(void* ptr)
{
    char* cptr = reinterpret_cast<char*>(ptr);
    DVASSERT(customAllocations.count(cptr) > 0);
    customAllocations.erase(cptr);
    free(ptr);
}

bool BlockAllocatorBase::ContainsPointer(void* ptr)
{
    return customAllocations.count(reinterpret_cast<char*>(ptr)) > 0;
}

uint32 BlockAllocatorBase::AllocationSizeForPointer(void* ptr)
{
    DVASSERT(ContainsPointer(ptr));
    return customAllocations[reinterpret_cast<char*>(ptr)];
}

void BlockAllocatorBase::ReleaseTrackedMemory()
{
    for (auto& data : customAllocations)
    {
        free(data.first);
    }
    customAllocations.clear();
}

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

#ifndef __DAVAENGINE_MEMORYPROFILER_H__
#define __DAVAENGINE_MEMORYPROFILER_H__

#include "Base/BaseTypes.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include "MemoryManager.h"

void * operator new (size_t size, DAVA::ePredefAllocPools pool);
void * operator new[](size_t size, DAVA::ePredefAllocPools pool);
void operator delete(void * ptr, DAVA::ePredefAllocPools pool);
void operator delete[](void * ptr, DAVA::ePredefAllocPools pool);
#define MEMORY_PROFILER_REGISTER_TAG(index, name)           DAVA::MemoryManager::RegisterTagName(static_cast<int>(index), name)
#define MEMORY_PROFILER_REGISTER_ALLOC_POOL(index, name)    DAVA::MemoryManager::RegisterAllocPoolName(index, name)
#define MEMORY_PROFILER_REGISTER_MARKER(index,name)         DAVA::MemoryManager::RegisterMarkerName(index,name)



#define MEMORY_PROFILER_ENTER_TAG(tag)                      DAVA::MemoryManager::Instance()->EnterTagScope(static_cast<int>(tag))
#define MEMORY_PROFILER_LEAVE_TAG(tag)                      DAVA::MemoryManager::Instance()->LeaveTagScope(static_cast<int>(tag))

#define MEMORY_PROFILER_CHECKPOINT(checkpoint)              DAVA::MemoryManager::Instance()->Checkpoint(checkpoint)

#define MEMORY_PROFILER_NEW(pool,construct)                 new (pool) construct

#define MEMORY_PROFILER_ALLOCATE(size,pool)                 DAVA::MemoryManager::Instance()->Allocate(size,pool)

#define MEMORY_PROFILER_SETMARKER(marker)                   DAVA::MemoryManager::Instance()->SetCurrentActiveMarker(marker)

#else   // defined(DAVA_MEMORY_PROFILING_ENABLE)

#define MEMORY_PROFILER_REGISTER_TAG(index, name)
#define MEMORY_PROFILER_REGISTER_ALLOC_POOL(index, name)

#define MEMORY_PROFILER_ENTER_TAG(tag)
#define MEMORY_PROFILER_LEAVE_TAG()
#define MEMORY_PROFILER_CHECKPOINT(checkpoint)

#define MEMORY_PROFILER_NEW(pool,construct)                new construct

#define MEMORY_PROFILER_ALLOCATE(size,pool)                malloc(size);

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)

#endif  // __DAVAENGINE_MEMORYPROFILER_H__

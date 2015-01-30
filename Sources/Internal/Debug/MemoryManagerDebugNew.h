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
#ifndef __DAVAENGINE_MEMORYMANAGER_DEBUG_NEW_H__
#define __DAVAENGINE_MEMORYMANAGER_DEBUG_NEW_H__

#include <cstdlib>

enum eMemoryPool
{
    MEMORY_POOL_DEFAULT = 0,
    MEMORY_POOL_BASE_OBJECTS,
    MEMORY_POOL_STL,
    MEMORY_POOL_TEXTURES_IMAGES,
    MEMORY_POOL_POLYGON_GROUPS,
    MEMORY_POOL_COUNT,
};

void * New(size_t size, eMemoryPool pool, const char * name);

#if defined(ENABLE_MEMORY_MANAGER)

#define ENABLE_MEM_MANAGER_TRACK(TYPE) \
void * operator new(std::size_t size) \
{ \
    return New(size, TYPE, 0); \
} \
\
void operator delete(void * ptr) \
{ \
    ::operator delete(ptr);\
}
#else 
#define ENABLE_MEM_MANAGER_TRACK(TYPE)
#endif

#endif // __DAVAENGINE_MEMORYMANAGER_DEBUG_NEW_H__


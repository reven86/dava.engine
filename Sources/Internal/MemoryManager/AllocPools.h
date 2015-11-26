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


#ifndef __DAVAENGINE_ALLOCPOOLS_H__
#define __DAVAENGINE_ALLOCPOOLS_H__

namespace DAVA
{

// Predefined allocation pools
enum ePredefAllocPools
{
    ALLOC_POOL_TOTAL = 0, // Virtual allocation pool for total allocations - sum of all other allocation pools
    ALLOC_POOL_DEFAULT, // Allocation pool used for all other application memory allocations, except custom if any
    ALLOC_GPU_TEXTURE, // Virtual allocation pool for GPU textures
    ALLOC_GPU_RDO_VERTEX, // Virtual allocation pool for GPU vertices from RenderDataObject
    ALLOC_GPU_RDO_INDEX, // Virtual allocation pool for GPU indices from RenderDataObject
    ALLOC_POOL_SYSTEM, // Virtual allocation pool for memory usage reported by system
    ALLOC_POOL_FMOD,
    ALLOC_POOL_BULLET,
    ALLOC_POOL_BASEOBJECT,
    ALLOC_POOL_POLYGONGROUP,
    ALLOC_POOL_COMPONENT,
    ALLOC_POOL_ENTITY,
    ALLOC_POOL_LANDSCAPE,
    ALLOC_POOL_IMAGE,
    ALLOC_POOL_TEXTURE,
    ALLOC_POOL_NMATERIAL,

    ALLOC_POOL_RHI_BUFFER,
    ALLOC_POOL_RHI_VERTEX_MAP,
    ALLOC_POOL_RHI_INDEX_MAP,
    ALLOC_POOL_RHI_TEXTURE_MAP,
    ALLOC_POOL_RHI_RESOURCE_POOL,

    PREDEF_POOL_COUNT,
    FIRST_CUSTOM_ALLOC_POOL = PREDEF_POOL_COUNT // First custom allocation pool must be FIRST_CUSTOM_ALLOC_POOL
};

}   // namespace DAVA

#endif  // __DAVAENGINE_ALLOCPOOLS_H__

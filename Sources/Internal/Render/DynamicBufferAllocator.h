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

#ifndef __DAVAENGINE_DYNAMIC_BUFFER_ALLOCATOR_H_
#define __DAVAENGINE_DYNAMIC_BUFFER_ALLOCATOR_H_

#include "Render/RHI/rhi_Public.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
namespace DynamicBufferAllocator
{
static const uint32 DEFAULT_PAGE_SIZE = 131072;

struct AllocResultVB
{
    rhi::HVertexBuffer buffer;
    uint8* data;
    uint32 baseVertex;
    uint32 allocatedVertices;
};

struct AllocResultIB
{
    rhi::HIndexBuffer buffer;
    uint16* data;
    uint32 baseIndex;
    uint32 allocatedindices;
};

AllocResultVB AllocateVertexBuffer(uint32 vertexSize, uint32 vertexCount);
AllocResultIB AllocateIndexBuffer(uint32 indexCount);

//it has a bit different life cycle - it is put to eviction queue only once greater size buffer is requested (so client code should still request it every frame), still trying to share existing one
rhi::HIndexBuffer AllocateQuadListIndexBuffer(uint32 quadCount);

void BeginFrame();
void EndFrame();
void Clear();

void SetPageSize(uint32 size);
}
}

#endif // !__DAVAENGINE_PARTICLE_RENDER_OBJECT_H_

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

#include "DynamicBufferAllocator.h"

namespace DAVA
{
namespace DynamicBufferAllocator
{
namespace //for private members
{
uint32 defaultPageSize = 0x0000ffff;

struct BufferInfo
{
    rhi::HVertexBuffer buffer;
    uint32 allocatedSize;
    uint32 dirtyTimeout;
    bool shouldBeEvicted;
};

BufferInfo* currentlyMappedBuffer = nullptr;
uint8* currentlyMappedData = nullptr;
uint32 currentlyUsedSize = 0;

List<BufferInfo*> freeBuffers;
List<BufferInfo*> usedBuffers;

}



AllocResultVB AllocateVertexBuffer(uint32 vertexSize, uint32 vertexCount)
{
    DVASSERT(vertexSize);
    DVASSERT((vertexSize * vertexCount) <= defaultPageSize); //assert for now - later allocate as much as possible and return incomplete buffer
    uint32 requiredSize = vertexSize * vertexCount;

    //try to fit in current buffer
    if (currentlyMappedBuffer) //try to fit in existing
    {
        uint32 baseV = ((currentlyUsedSize + vertexSize - 1) / vertexSize);
        uint32 offset = baseV * vertexSize;
        if ((offset + requiredSize) < currentlyMappedBuffer->allocatedSize)
        {
            currentlyUsedSize = offset + requiredSize;
            AllocResultVB res;
            res.buffer = currentlyMappedBuffer->buffer;
            res.data = currentlyMappedData + offset;
            res.baseVertex = baseV;
            res.allocatedVertices = vertexCount;            
            return res;
        }
        else // cant fit - finish it! :)
        {
            rhi::UnmapVertexBuffer(currentlyMappedBuffer->buffer);
            usedBuffers.push_back(currentlyMappedBuffer);
            currentlyMappedBuffer->dirtyTimeout = 3;            
            currentlyMappedData = nullptr;
            currentlyMappedBuffer = nullptr;
            currentlyUsedSize = 0;
        }
    }

    //start new buffer
    if (freeBuffers.size())
    {
        currentlyMappedBuffer = *freeBuffers.begin();
        freeBuffers.pop_front();
    }
    else
    {
        currentlyMappedBuffer = new BufferInfo();
        currentlyMappedBuffer->allocatedSize = defaultPageSize;
        currentlyMappedBuffer->buffer = rhi::CreateVertexBuffer(defaultPageSize);
        currentlyMappedBuffer->shouldBeEvicted = false;        
    }    
    currentlyMappedData = (uint8*) rhi::MapVertexBuffer(currentlyMappedBuffer->buffer, 0, currentlyMappedBuffer->allocatedSize);            
    
    //fit new
    currentlyUsedSize = requiredSize;
    AllocResultVB res;
    res.buffer = currentlyMappedBuffer->buffer;
    res.data = currentlyMappedData;
    res.baseVertex = 0;
    res.allocatedVertices = vertexCount;    
    return res;
    
}


rhi::HIndexBuffer GetDefaultQuadListIndexBuffer(uint32 quadCount)
{
    /*
    uint16 indices[6 * 1000];

    for (int i = 0; i < 1000; ++i)
    {
    indices[i*INDICES_PER_PARTICLE + 0] = i*POINTS_PER_PARTICLE + 0;
    indices[i*INDICES_PER_PARTICLE + 1] = i*POINTS_PER_PARTICLE + 1;
    indices[i*INDICES_PER_PARTICLE + 2] = i*POINTS_PER_PARTICLE + 2;
    indices[i*INDICES_PER_PARTICLE + 3] = i*POINTS_PER_PARTICLE + 2;
    indices[i*INDICES_PER_PARTICLE + 4] = i*POINTS_PER_PARTICLE + 1;
    indices[i*INDICES_PER_PARTICLE + 5] = i*POINTS_PER_PARTICLE + 3; //preserve order
    }
    indexBuffer = rhi::CreateIndexBuffer(1000 * 2);
    rhi::UpdateIndexBuffer(indexBuffer, indices, 0, 1000 * 2);
    */
}

void BeginFrame()
{
    //update used buffers ttl
    auto it = usedBuffers.begin();
    while (it!=usedBuffers.end())
    {
        (*it)->dirtyTimeout--;
        if ((*it)->dirtyTimeout == 0)
        {
            if ((*it)->shouldBeEvicted)
            {
                rhi::DeleteVertexBuffer((*it)->buffer);
                SafeDelete(*it);
            }
            else
            {
                freeBuffers.push_back(*it);
            }
            it = usedBuffers.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void EndFrame()
{    
    if (currentlyMappedBuffer)
    {
        rhi::UnmapVertexBuffer(currentlyMappedBuffer->buffer);
        usedBuffers.push_back(currentlyMappedBuffer);
        currentlyMappedBuffer->dirtyTimeout = 3;
        currentlyMappedData = nullptr;
        currentlyMappedBuffer = nullptr;
        currentlyUsedSize = 0;
    }
}

void Clear()
{
    for (auto b : freeBuffers)
    {
        rhi::DeleteVertexBuffer(b->buffer);
        SafeDelete(b);
    }
    for (auto b : usedBuffers)
        b->shouldBeEvicted = true;
}
void SetDefaultPageSize(uint32 size)
{
    defaultPageSize = size;
    Clear();
}


}
}
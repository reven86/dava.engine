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
uint32 defaultPageSize = 131072; //128kb


template <class HBuffer> class BufferProxy
{
public:
    static HBuffer CreateBuffer(uint32 size);
    static uint8* MapBuffer(HBuffer handle, uint32 offset, uint32 size);
    static void UnmapBuffer(HBuffer handle);
    static void DeleteBuffer(HBuffer handle);
};

template <> class BufferProxy<rhi::HVertexBuffer>
{
public:
    static rhi::HVertexBuffer CreateBuffer(uint32 size)
    { 
        rhi::VertexBuffer::Descriptor descr = rhi::VertexBuffer::Descriptor(size);
        descr.needRestore = false;
        descr.usage = rhi::USAGE_DYNAMICDRAW;
        return rhi::CreateVertexBuffer(descr);
    }
    static uint8* MapBuffer(rhi::HVertexBuffer handle, uint32 offset, uint32 size){ return (uint8*) rhi::MapVertexBuffer(handle, offset, size); }
    static void UnmapBuffer(rhi::HVertexBuffer handle){ rhi::UnmapVertexBuffer(handle); }
    static void DeleteBuffer(rhi::HVertexBuffer handle){ rhi::DeleteVertexBuffer(handle); }
};

template <> class BufferProxy < rhi::HIndexBuffer >
{
public:
    static rhi::HIndexBuffer CreateBuffer(uint32 size)
    { 
        rhi::IndexBuffer::Descriptor descr = rhi::IndexBuffer::Descriptor(size);
        descr.needRestore = false;
        descr.usage = rhi::USAGE_DYNAMICDRAW;
        return rhi::CreateIndexBuffer(descr); 
    }
    static uint8* MapBuffer(rhi::HIndexBuffer handle, uint32 offset, uint32 size){ return (uint8*)rhi::MapIndexBuffer(handle, offset, size); }
    static void UnmapBuffer(rhi::HIndexBuffer handle){ rhi::UnmapIndexBuffer(handle); }
    static void DeleteBuffer(rhi::HIndexBuffer handle){ rhi::DeleteIndexBuffer(handle); }
};




template <class HBuffer> struct BufferAllocator
{    
    struct BufferInfo
    {
        HBuffer buffer;
        uint32 allocatedSize;
        rhi::HSyncObject readySync;
    };

    struct BufferAllocateResult
    {
        HBuffer buffer;
        uint8 *data;
        uint32 base;
        uint32 count;
    };    

    BufferAllocateResult AllocateData(uint32 size, uint32 count)
    {
        DVASSERT(size);
        uint32 requiredSize = (size * count);
        DVASSERT(requiredSize <= defaultPageSize); //assert for now - later allocate as much as possible and return incomplete buffer
        uint32 base = ((currentlyUsedSize + size - 1) / size);
        uint32 offset = base * size;
        //cant fit - start new
        if ((!currentlyMappedBuffer) || ((offset + requiredSize) > currentlyMappedBuffer->allocatedSize))
        {
            if (currentlyMappedBuffer) //unmap it
            {
                BufferProxy<HBuffer>::UnmapBuffer(currentlyMappedBuffer->buffer);                
                usedBuffers.push_back(currentlyMappedBuffer);
            }
            if (freeBuffers.size())
            {
                currentlyMappedBuffer = *freeBuffers.begin();
                freeBuffers.pop_front();
            }
            else
            {
                currentlyMappedBuffer = new BufferInfo();
                currentlyMappedBuffer->allocatedSize = defaultPageSize;
                currentlyMappedBuffer->buffer = BufferProxy<HBuffer>::CreateBuffer(defaultPageSize);                 
            }
            currentlyMappedData = BufferProxy<HBuffer>::MapBuffer(currentlyMappedBuffer->buffer, 0, currentlyMappedBuffer->allocatedSize); 
            currentlyMappedBuffer->readySync = rhi::GetCurrentFrameSyncObject();
            offset = 0;
            base = 0;
        }
        
        BufferAllocateResult res;
        res.buffer = currentlyMappedBuffer->buffer;
        res.data = currentlyMappedData + offset;
        res.base = base;
        res.count = count;

        currentlyUsedSize = offset + requiredSize;

        return res;
    }


    void Clear()
    {
        for (auto b : freeBuffers)
        {
            BufferProxy<HBuffer>::DeleteBuffer(b->buffer);
            SafeDelete(b);
        }
        for (auto b : usedBuffers)
        {
            BufferProxy<HBuffer>::DeleteBuffer(b->buffer);
            SafeDelete(b);
        }            
    }

    void BeginFrame()
    {
        //update used buffers ttl
        auto it = usedBuffers.begin();
        while (it != usedBuffers.end())
        {            
            if (rhi::SyncObjectSignaled((*it)->readySync))
            {                                
                freeBuffers.push_back(*it);                
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
            BufferProxy<HBuffer>::UnmapBuffer(currentlyMappedBuffer->buffer);
            usedBuffers.push_back(currentlyMappedBuffer);            
            currentlyMappedData = nullptr;
            currentlyMappedBuffer = nullptr;
            currentlyUsedSize = 0;
        }
    }    

private:
    BufferInfo* currentlyMappedBuffer = nullptr;
    uint8* currentlyMappedData = nullptr;
    uint32 currentlyUsedSize = 0;
    List<BufferInfo*> freeBuffers;
    List<BufferInfo*> usedBuffers;

};

BufferAllocator<rhi::HVertexBuffer> vertexBufferAllocator;
BufferAllocator<rhi::HIndexBuffer>  indexBufferAllocator;

rhi::HIndexBuffer currQuadList;
uint32 currMaxQuadCount = 0;
}


AllocResultVB AllocateVertexBuffer(uint32 vertexSize, uint32 vertexCount)
{
    BufferAllocator<rhi::HVertexBuffer>::BufferAllocateResult result = vertexBufferAllocator.AllocateData(vertexSize, vertexCount);    
    return AllocResultVB{ result.buffer, result.data, result.base, result.count };        
}

AllocResultIB AllocateIndexBuffer(uint32 indexCount)
{
    BufferAllocator<rhi::HIndexBuffer>::BufferAllocateResult result = indexBufferAllocator.AllocateData(2, indexCount);
    return AllocResultIB{ result.buffer, (uint16 *) result.data, result.base, result.count };
}


rhi::HIndexBuffer AllocateQuadListIndexBuffer(uint32 quadCount)
{

    if (quadCount > currMaxQuadCount)
    {        
        if (currQuadList.IsValid())
            rhi::DeleteIndexBuffer(currQuadList);            

        const uint32 VERTICES_PER_QUAD = 4;
        const uint32 INDICES_PER_QUAD = 6;

        currMaxQuadCount = Max(currMaxQuadCount * 2, quadCount);
        uint32 bufferSize = currMaxQuadCount * INDICES_PER_QUAD * 2; //uint16 = 2 bytes per index
        currQuadList = rhi::CreateIndexBuffer(bufferSize);
        uint16 * indices = (uint16*)rhi::MapIndexBuffer(currQuadList, 0, bufferSize);        
        for (uint32 i = 0; i < currMaxQuadCount; ++i)
        {
            indices[i*INDICES_PER_QUAD + 0] = i*VERTICES_PER_QUAD + 0;
            indices[i*INDICES_PER_QUAD + 1] = i*VERTICES_PER_QUAD + 1;
            indices[i*INDICES_PER_QUAD + 2] = i*VERTICES_PER_QUAD + 2;
            indices[i*INDICES_PER_QUAD + 3] = i*VERTICES_PER_QUAD + 2;
            indices[i*INDICES_PER_QUAD + 4] = i*VERTICES_PER_QUAD + 1;
            indices[i*INDICES_PER_QUAD + 5] = i*VERTICES_PER_QUAD + 3; //preserve order
        }

        rhi::UnmapIndexBuffer(currQuadList);
    }    
    return currQuadList;
}

void BeginFrame()
{
    vertexBufferAllocator.BeginFrame();
    indexBufferAllocator.BeginFrame();
}

void EndFrame()
{    
    vertexBufferAllocator.EndFrame();
    indexBufferAllocator.EndFrame();
}

void Clear()
{
    if (currQuadList.IsValid())
    {
        rhi::DeleteIndexBuffer(currQuadList);
        currQuadList = rhi::HIndexBuffer();
        currMaxQuadCount = 0;
    }        
    vertexBufferAllocator.Clear();
    indexBufferAllocator.Clear();
}
void SetDefaultPageSize(uint32 size)
{
    defaultPageSize = size;
    vertexBufferAllocator.Clear();
    indexBufferAllocator.Clear();
}


}
}
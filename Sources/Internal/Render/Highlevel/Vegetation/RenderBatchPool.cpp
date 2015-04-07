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

#include "Render/Material/NMaterial.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/Vegetation/RenderBatchPool.h"

namespace DAVA
{

RenderBatchPool::RenderBatchPoolEntry::RenderBatchPoolEntry() : poolLine(0)
{
    
}

RenderBatchPool::RenderBatchPoolEntry::~RenderBatchPoolEntry()
{
    size_t batchCount = renderBatches.size();
    for(size_t i = 0; i < batchCount; ++i)
    {
        SafeRelease(renderBatches[i]);
    }
}

/////////////////////////////////////////////////////////////////////////////////

RenderBatchPool::RenderBatchPool()
{
}

RenderBatchPool::~RenderBatchPool()
{
    ReleasePool();
}

void RenderBatchPool::Init(NMaterial* key, uint32 initialCount, VegetationMaterialTransformer* transform)
{
    DVASSERT(key);
    
    RenderBatchPoolEntry* entry = pool.at(key);
    if(NULL == entry)
    {
        entry = new RenderBatchPoolEntry();
        for(uint32 i = 0; i < initialCount; ++i)
        {
            CreateRenderBatch(key, entry, transform);
        }
        
        pool.insert(key, entry);
    }
}

void RenderBatchPool::Clear()
{
    ReleasePool();
}

RenderBatch* RenderBatchPool::Get(NMaterial* key, VegetationMaterialTransformer* transform)
{
    DVASSERT(key);

    RenderBatchPoolEntry* entry = pool.at(key);
    DVASSERT(entry);
    
    RenderBatch* rb = NULL;
    
    size_t currentPoolSize = entry->renderBatches.size();
    if(currentPoolSize <= (size_t)entry->poolLine)
    {
        rb = CreateRenderBatch(key, entry, transform);
    }
    else
    {
        rb = entry->renderBatches[entry->poolLine];
    }
    
    entry->poolLine++;
    
    return rb;
}

void RenderBatchPool::Return(NMaterial* key, uint32 count)
{
    RenderBatchPoolEntry* entry = pool.at(key);
    DVASSERT(entry);
    
    entry->poolLine -= count;
    DVASSERT(entry->poolLine >= 0);
}

void RenderBatchPool::ReturnAll(NMaterial* key)
{
    RenderBatchPoolEntry* entry = pool.at(key);
    DVASSERT(entry);
    
    entry->poolLine = 0;
}
    
void RenderBatchPool::ReturnAll()
{
    HashMap<NMaterial*, RenderBatchPoolEntry*>::iterator end = pool.end();
    for(HashMap<NMaterial*, RenderBatchPoolEntry*>::iterator it = pool.begin();
        it != end;
        ++it)
    {
        it->second->poolLine = 0;
    }
}

void RenderBatchPool::ReleasePool()
{
    HashMap<NMaterial*, RenderBatchPoolEntry*>::iterator end = pool.end();
    for(HashMap<NMaterial*, RenderBatchPoolEntry*>::iterator it = pool.begin();
        it != end;
        ++it)
    {
        SafeDelete(it->second);
    }
    
    pool.clear();
}

RenderBatch* RenderBatchPool::CreateRenderBatch(NMaterial* mat,
                                        RenderBatchPoolEntry* entry,
                                        VegetationMaterialTransformer* transform)
{
    RenderBatch* rb = new RenderBatch();
#if RHI_COMPLETE    
    NMaterial* batchMaterial = NMaterial::CreateMaterialInstance();
    batchMaterial->AddNodeFlags(DataNode::NodeRuntimeFlag);
    batchMaterial->SetParent(mat);
    
    if(transform)
    {
        transform->TransformMaterialOnCreate(batchMaterial);
    }
    
    rb->SetMaterial(batchMaterial);
    
    SafeRelease(batchMaterial);
    
    entry->renderBatches.push_back(rb);
#endif //RHI_COMPLETE
    
    return rb;
}

};

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


#include "CopyLastLODCommand.h"

using namespace DAVA;

CopyLastLODToLod0Command::CopyLastLODToLod0Command(DAVA::LodComponent *component) 
    : Command2(CMDID_LOD_COPY_LAST_LOD, "Copy last LOD to lod0")
    , lodComponent(component)
{
    RenderObject * ro = GetRenderObject(GetEntity());
    DVASSERT(ro);

    uint32 maxLodIndex = ro->GetMaxLodIndex();
    uint32 batchCount = ro->GetRenderBatchCount();
    int32 lodIndex, switchIndex;
    for(uint32 ri = 0; ri < batchCount; ++ri)
    {
        RenderBatch * batch = ro->GetRenderBatch(ri, lodIndex, switchIndex);
        if(lodIndex == maxLodIndex)
        {
            RenderBatch * newBatch = batch->Clone();
            newBatches.push_back(newBatch);
            switchIndices.push_back(switchIndex);
        }
    }
}

CopyLastLODToLod0Command::~CopyLastLODToLod0Command()
{
    uint32 newBatchCount = newBatches.size();
    for(uint32 ri = 0; ri < newBatchCount; ++ri)
        newBatches[ri]->Release();
    
    newBatches.clear();
    switchIndices.clear();
}

void CopyLastLODToLod0Command::Redo()
{
    if(!lodComponent) return;
 
    RenderObject * ro = GetRenderObject(GetEntity());
    DVASSERT(ro);

    for(int32 i = LodComponent::MAX_LOD_LAYERS - 1; i > 0; --i)
    {
        float32 distance = lodComponent->GetLodLayerDistance(i - 1);
        lodComponent->SetLodLayerDistance(i, distance);
    }

    lodComponent->SetLodLayerDistance(0, 0.f);
    lodComponent->SetLodLayerDistance(1, 2.f);

    uint32 batchCount = ro->GetRenderBatchCount();
    int32 lodIndex, switchIndex;
    for(uint32 ri = 0; ri < batchCount; ++ri)
    {
        ro->GetRenderBatch(ri, lodIndex, switchIndex);
        ro->SetRenderBatchLODIndex(ri, lodIndex + 1);
    }

    uint32 newBatchCount = newBatches.size();
    for(uint32 ri = 0; ri < newBatchCount; ++ri)
    {
        ro->AddRenderBatch(newBatches[ri], 0, switchIndices[ri]);
    }
}
 
void CopyLastLODToLod0Command::Undo()
{
    if(!lodComponent) return;

    RenderObject * ro = GetRenderObject(GetEntity());
    DVASSERT(ro);

    for(int32 i = 0; i < LodComponent::MAX_LOD_LAYERS - 1; ++i)
        lodComponent->SetLodLayerDistance(i, lodComponent->GetLodLayerDistance(i + 1));

    lodComponent->SetLodLayerDistance(0, 0.f);

    uint32 newBatchCount = newBatches.size();
    for(uint32 ri = 0; ri < newBatchCount; ++ri)
        ro->RemoveRenderBatch(newBatches[ri]);

    uint32 batchCount = ro->GetRenderBatchCount();
    int32 lodIndex, switchIndex;
    for(uint32 ri = 0; ri < batchCount; ++ri)
    {
        ro->GetRenderBatch(ri, lodIndex, switchIndex);
        ro->SetRenderBatchLODIndex(ri, lodIndex - 1);
    }
}
 
Entity * CopyLastLODToLod0Command::GetEntity() const
{
    if(lodComponent)
        return lodComponent->GetEntity();

    return NULL;
}

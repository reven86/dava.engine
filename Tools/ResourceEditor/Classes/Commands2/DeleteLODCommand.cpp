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



#include "DeleteLODCommand.h"
#include "DeleteRenderBatchCommand.h"


#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/Components/ComponentHelpers.h"

DeleteLODCommand::DeleteLODCommand(DAVA::LodComponent *lod, DAVA::int32 lodIndex, DAVA::int32 switchIndex)
	: Command2(CMDID_LOD_DELETE, "Delete LOD")
	, lodComponent(lod)
    , deletedLodIndex(lodIndex)
    , requestedSwitchIndex(switchIndex)
{
    DVASSERT(lodComponent);
    DAVA::RenderObject *ro = DAVA::GetRenderObject(GetEntity());
    DVASSERT(ro);
    DVASSERT(ro->GetType() != DAVA::RenderObject::TYPE_PARTICLE_EMTITTER);

    savedDistances = lodComponent->lodLayersArray;
}

DeleteLODCommand::~DeleteLODCommand()
{
    DAVA::uint32 count = (DAVA::uint32)deletedBatches.size();
    for(DAVA::uint32 i = 0; i < count; ++i)
    {
        DAVA::SafeDelete(deletedBatches[i]);
    }
    deletedBatches.clear();
}

void DeleteLODCommand::Redo()
{
    DVASSERT(deletedBatches.size() == 0);

    DAVA::Entity *entity = GetEntity();
    DAVA::RenderObject *ro = DAVA::GetRenderObject(entity);

    //save renderbatches
    DAVA::int32 count = (DAVA::int32)ro->GetRenderBatchCount();
    for(DAVA::int32 i = count-1; i >= 0; --i)
    {
        DAVA::int32 lodIndex = 0, switchIndex = 0;
        ro->GetRenderBatch(i, lodIndex, switchIndex);
        if(lodIndex == deletedLodIndex && (requestedSwitchIndex == switchIndex || requestedSwitchIndex == -1))
        {
            DeleteRenderBatchCommand *command = new DeleteRenderBatchCommand(entity, ro, i);
            deletedBatches.push_back(command);

            RedoInternalCommand(command);
        }
    }
    
    //update indexes
    count = ro->GetRenderBatchCount();
    for(DAVA::int32 i = (DAVA::int32)count - 1; i >= 0; --i)
    {
        DAVA::int32 lodIndex = 0, switchIndex = 0;
        DAVA::RenderBatch *batch = ro->GetRenderBatch(i, lodIndex, switchIndex);
        if(lodIndex > deletedLodIndex && (requestedSwitchIndex == switchIndex || requestedSwitchIndex == -1))
        {
            batch->Retain();
            
            ro->RemoveRenderBatch(i);
            ro->AddRenderBatch(batch, lodIndex - 1, switchIndex);
            
            batch->Release();
        }
    }

    //update distances
    for(DAVA::int32 i = deletedLodIndex; i < DAVA::LodComponent::MAX_LOD_LAYERS-1; ++i)
    {
        lodComponent->lodLayersArray[i] = lodComponent->lodLayersArray[i+1];
    }
    
    //last lod
    DAVA::uint32 layersSize = ro->GetMaxLodIndex() + 1;
    if(layersSize)
    {
        lodComponent->lodLayersArray[layersSize-1].SetFarDistance(2 * DAVA::LodComponent::MAX_LOD_DISTANCE);
    }
    
    //first lod
    lodComponent->SetLodLayerDistance(0, 0);
    lodComponent->lodLayersArray[0].SetNearDistance(0.0f);
    
    //visual part
    lodComponent->currentLod = DAVA::LodComponent::INVALID_LOD_LAYER;
    lodComponent->forceLodLayer = DAVA::LodComponent::INVALID_LOD_LAYER;
}

void DeleteLODCommand::Undo()
{
    DAVA::RenderObject *ro = DAVA::GetRenderObject(GetEntity());

    //restore lodindexes
    DAVA::uint32 count = ro->GetRenderBatchCount();
    for(DAVA::int32 i = (DAVA::int32)count - 1; i >= 0; --i)
    {
        DAVA::int32 lodIndex = 0, switchIndex = 0;
        DAVA::RenderBatch *batch = ro->GetRenderBatch(i, lodIndex, switchIndex);
        if(lodIndex >= deletedLodIndex && (requestedSwitchIndex == switchIndex || requestedSwitchIndex == -1))
        {
            batch->Retain();
            
            ro->RemoveRenderBatch(i);
            ro->AddRenderBatch(batch, lodIndex + 1, switchIndex);
            
            batch->Release();
        }
    }

    //restore batches
    count = (DAVA::uint32)deletedBatches.size();
    for(DAVA::uint32 i = 0; i < count; ++i)
    {
        UndoInternalCommand(deletedBatches[i]);
        DAVA::SafeDelete(deletedBatches[i]);
    }
    deletedBatches.clear();

    //restore lodlayers and disatnces
    lodComponent->lodLayersArray = savedDistances;
}


DAVA::Entity * DeleteLODCommand::GetEntity() const
{
    return lodComponent->GetEntity();
}

const DAVA::Vector<DeleteRenderBatchCommand *> & DeleteLODCommand::GetRenderBatchCommands() const
{
    return deletedBatches;
}




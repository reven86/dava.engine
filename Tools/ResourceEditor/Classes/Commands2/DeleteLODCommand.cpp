#include "DeleteLODCommand.h"
#include "DeleteRenderBatchCommand.h"


#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/Components/ComponentHelpers.h"

DeleteLODCommand::DeleteLODCommand(DAVA::LodComponent* lod, DAVA::int32 lodIndex, DAVA::int32 switchIndex)
    : Command2(CMDID_LOD_DELETE, "Delete LOD")
    , lodComponent(lod)
    , deletedLodIndex(lodIndex)
    , requestedSwitchIndex(switchIndex)
{
    DVASSERT(lodComponent);
    DAVA::Entity* entity = GetEntity();
    DAVA::RenderObject* ro = DAVA::GetRenderObject(entity);

    DVASSERT(ro);
    DVASSERT(ro->GetType() != DAVA::RenderObject::TYPE_PARTICLE_EMTITTER);

    savedDistances = lodComponent->lodLayersArray;

    //save renderBatches
    DAVA::int32 count = (DAVA::int32)ro->GetRenderBatchCount();
    for (DAVA::int32 i = count - 1; i >= 0; --i)
    {
        DAVA::int32 batchLodIndex = 0, batchSwitchIndex = 0;
        ro->GetRenderBatch(i, batchLodIndex, batchSwitchIndex);
        if (batchLodIndex == deletedLodIndex && (requestedSwitchIndex == batchSwitchIndex || requestedSwitchIndex == -1))
        {
            DeleteRenderBatchCommand* command = new DeleteRenderBatchCommand(entity, ro, i);
            deletedBatches.push_back(command);
        }
    }
}

DeleteLODCommand::~DeleteLODCommand()
{
    DAVA::uint32 count = (DAVA::uint32)deletedBatches.size();
    for (DAVA::uint32 i = 0; i < count; ++i)
    {
        DAVA::SafeDelete(deletedBatches[i]);
    }
    deletedBatches.clear();
}

void DeleteLODCommand::Redo()
{
    for (DeleteRenderBatchCommand* command : deletedBatches)
    {
        RedoInternalCommand(command);
    }

    //update indexes
    DAVA::RenderObject* ro = DAVA::GetRenderObject(GetEntity());
    DAVA::int32 count = ro->GetRenderBatchCount();
    for (DAVA::int32 i = (DAVA::int32)count - 1; i >= 0; --i)
    {
        DAVA::int32 lodIndex = 0, switchIndex = 0;
        DAVA::RenderBatch* batch = ro->GetRenderBatch(i, lodIndex, switchIndex);
        if (lodIndex > deletedLodIndex && (requestedSwitchIndex == switchIndex || requestedSwitchIndex == -1))
        {
            batch->Retain();

            ro->RemoveRenderBatch(i);
            ro->AddRenderBatch(batch, lodIndex - 1, switchIndex);

            batch->Release();
        }
    }

    //update distances
    for (DAVA::int32 i = deletedLodIndex; i < DAVA::LodComponent::MAX_LOD_LAYERS - 1; ++i)
    {
        lodComponent->lodLayersArray[i] = lodComponent->lodLayersArray[i + 1];
    }

    //last lod
    DAVA::uint32 layersSize = ro->GetMaxLodIndex() + 1;
    if (layersSize)
    {
        lodComponent->lodLayersArray[layersSize - 1].SetFarDistance(2 * DAVA::LodComponent::MAX_LOD_DISTANCE);
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
    DAVA::RenderObject* ro = DAVA::GetRenderObject(GetEntity());

    //restore lodindexes
    DAVA::uint32 count = ro->GetRenderBatchCount();
    for (DAVA::int32 i = (DAVA::int32)count - 1; i >= 0; --i)
    {
        DAVA::int32 lodIndex = 0, switchIndex = 0;
        DAVA::RenderBatch* batch = ro->GetRenderBatch(i, lodIndex, switchIndex);
        if (lodIndex >= deletedLodIndex && (requestedSwitchIndex == switchIndex || requestedSwitchIndex == -1))
        {
            batch->Retain();

            ro->RemoveRenderBatch(i);
            ro->AddRenderBatch(batch, lodIndex + 1, switchIndex);

            batch->Release();
        }
    }

    //restore batches
    count = (DAVA::uint32)deletedBatches.size();
    for (DAVA::uint32 i = 0; i < count; ++i)
    {
        UndoInternalCommand(deletedBatches[i]);
    }

    //restore lodlayers and distances
    lodComponent->lodLayersArray = savedDistances;
}

DAVA::Entity* DeleteLODCommand::GetEntity() const
{
    return lodComponent->GetEntity();
}

const DAVA::Vector<DeleteRenderBatchCommand*>& DeleteLODCommand::GetRenderBatchCommands() const
{
    return deletedBatches;
}

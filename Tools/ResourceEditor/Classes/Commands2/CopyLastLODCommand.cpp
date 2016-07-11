#include "CopyLastLODCommand.h"

using namespace DAVA;

CopyLastLODToLod0Command::CopyLastLODToLod0Command(DAVA::LodComponent* component)
    : RECommand(CMDID_LOD_COPY_LAST_LOD, "Copy last LOD to lod0")
    , lodComponent(component)
{
    RenderObject* ro = GetRenderObject(GetEntity());
    DVASSERT(ro);

    uint32 maxLodIndex = ro->GetMaxLodIndex();
    uint32 batchCount = ro->GetRenderBatchCount();
    int32 lodIndex, switchIndex;
    for (uint32 ri = 0; ri < batchCount; ++ri)
    {
        RenderBatch* batch = ro->GetRenderBatch(ri, lodIndex, switchIndex);
        if (lodIndex == maxLodIndex)
        {
            RenderBatch* newBatch = batch->Clone();
            newBatches.push_back(newBatch);
            switchIndices.push_back(switchIndex);
        }
    }
}

CopyLastLODToLod0Command::~CopyLastLODToLod0Command()
{
    for (DAVA::RenderBatch* batch : newBatches)
    {
        DAVA::SafeRelease(batch);
    }

    newBatches.clear();
    switchIndices.clear();
}

void CopyLastLODToLod0Command::Redo()
{
    if (!lodComponent)
        return;

    RenderObject* ro = GetRenderObject(GetEntity());
    DVASSERT(ro);

    for (int32 i = LodComponent::MAX_LOD_LAYERS - 1; i > 0; --i)
    {
        float32 distance = lodComponent->GetLodLayerDistance(i - 1);
        lodComponent->SetLodLayerDistance(i, distance);
    }

    lodComponent->SetLodLayerDistance(0, 0.f);
    lodComponent->SetLodLayerDistance(1, 2.f);

    uint32 batchCount = ro->GetRenderBatchCount();
    int32 lodIndex, switchIndex;
    for (uint32 ri = 0; ri < batchCount; ++ri)
    {
        ro->GetRenderBatch(ri, lodIndex, switchIndex);
        ro->SetRenderBatchLODIndex(ri, lodIndex + 1);
    }

    uint32 newBatchCount = newBatches.size();
    for (uint32 ri = 0; ri < newBatchCount; ++ri)
    {
        ro->AddRenderBatch(newBatches[ri], 0, switchIndices[ri]);
    }
}

void CopyLastLODToLod0Command::Undo()
{
    if (!lodComponent)
        return;

    RenderObject* ro = GetRenderObject(GetEntity());
    DVASSERT(ro);

    for (int32 i = 0; i < LodComponent::MAX_LOD_LAYERS - 1; ++i)
        lodComponent->SetLodLayerDistance(i, lodComponent->GetLodLayerDistance(i + 1));

    lodComponent->SetLodLayerDistance(0, 0.f);

    uint32 newBatchCount = newBatches.size();
    for (uint32 ri = 0; ri < newBatchCount; ++ri)
        ro->RemoveRenderBatch(newBatches[ri]);

    uint32 batchCount = ro->GetRenderBatchCount();
    int32 lodIndex, switchIndex;
    for (uint32 ri = 0; ri < batchCount; ++ri)
    {
        ro->GetRenderBatch(ri, lodIndex, switchIndex);
        ro->SetRenderBatchLODIndex(ri, lodIndex - 1);
    }
}

Entity* CopyLastLODToLod0Command::GetEntity() const
{
    if (lodComponent)
        return lodComponent->GetEntity();

    return NULL;
}

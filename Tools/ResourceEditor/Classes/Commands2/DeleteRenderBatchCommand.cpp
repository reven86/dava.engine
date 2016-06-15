#include "DeleteRenderBatchCommand.h"

DeleteRenderBatchCommand::DeleteRenderBatchCommand(DAVA::Entity* en, DAVA::RenderObject* ro, DAVA::uint32 batchIndex)
    : Command2(CMDID_DELETE_RENDER_BATCH, "Delete Render Batch")
    , entity(en)
    , renderObject(ro)
{
    DVASSERT(entity);
    DVASSERT(renderObject);
    DVASSERT(batchIndex < renderObject->GetRenderBatchCount());

    renderBatch = renderObject->GetRenderBatch(batchIndex, lodIndex, switchIndex);
    DAVA::SafeRetain(renderBatch);
}

DeleteRenderBatchCommand::~DeleteRenderBatchCommand()
{
    DAVA::SafeRelease(renderBatch);
}

void DeleteRenderBatchCommand::Redo()
{
    renderObject->RemoveRenderBatch(renderBatch);
}

void DeleteRenderBatchCommand::Undo()
{
    renderObject->AddRenderBatch(renderBatch, lodIndex, switchIndex);
}

DAVA::Entity* DeleteRenderBatchCommand::GetEntity() const
{
    return entity;
}

DAVA::RenderBatch* DeleteRenderBatchCommand::GetRenderBatch() const
{
    return renderBatch;
}

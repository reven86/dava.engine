#ifndef __DELETE_LOD_COMMAND_H__
#define __DELETE_LOD_COMMAND_H__

#include "QtTools/Commands/CommandWithoutExecute.h"

#include "Render/Highlevel/RenderBatch.h"
#include "Scene3D/Components/LodComponent.h"

class DeleteRenderBatchCommand;
class DeleteLODCommand : public CommandWithoutExecute
{
public:
    DeleteLODCommand(DAVA::LodComponent* lod, DAVA::int32 lodIndex, DAVA::int32 switchIndex);
    virtual ~DeleteLODCommand();

    void Undo() override;
    void Redo() override;
    DAVA::Entity* GetEntity() const;

    const DAVA::Vector<DeleteRenderBatchCommand*>& GetRenderBatchCommands() const;

protected:
    DAVA::LodComponent* lodComponent;
    DAVA::int32 deletedLodIndex;
    DAVA::int32 requestedSwitchIndex;

    DAVA::Vector<DAVA::LodComponent::LodDistance> savedDistances;
    DAVA::Vector<DeleteRenderBatchCommand*> deletedBatches;
};


#endif // __DELETE_LOD_COMMAND_H__

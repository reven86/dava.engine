#ifndef __DELETE_LOD_COMMAND_H__
#define __DELETE_LOD_COMMAND_H__

#include "Commands2/Base/Command2.h"

#include "Render/Highlevel/RenderBatch.h"
#include "Scene3D/Lod/LodComponent.h"

class DeleteRenderBatchCommand;
class DeleteLODCommand : public Command2
{
public:
    DeleteLODCommand(DAVA::LodComponent* lod, DAVA::int32 lodIndex, DAVA::int32 switchIndex);
    virtual ~DeleteLODCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const;

    const DAVA::Vector<DeleteRenderBatchCommand*>& GetRenderBatchCommands() const;

protected:
    DAVA::LodComponent* lodComponent;
    DAVA::int32 deletedLodIndex;
    DAVA::int32 requestedSwitchIndex;

    DAVA::Vector<DeleteRenderBatchCommand*> deletedBatches;
};


#endif // __DELETE_LOD_COMMAND_H__

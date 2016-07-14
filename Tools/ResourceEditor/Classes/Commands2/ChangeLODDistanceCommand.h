#ifndef __CHANGE_LOD_DISTANCE_COMMAND_H__
#define __CHANGE_LOD_DISTANCE_COMMAND_H__

#include "QtTools/Commands/CommandWithoutExecute.h"
#include "DAVAEngine.h"

class ChangeLODDistanceCommand : public CommandWithoutExecute
{
public:
    ChangeLODDistanceCommand(DAVA::LodComponent* lod, DAVA::int32 lodLayer, DAVA::float32 distance);

    void Undo() override;
    void Redo() override;
    DAVA::Entity* GetEntity() const;

protected:
    DAVA::LodComponent* lodComponent;
    DAVA::int32 layer;
    DAVA::float32 newDistance;
    DAVA::float32 oldDistance;
};

#endif // __CHANGE_LOD_DISTANCE_COMMAND_H__

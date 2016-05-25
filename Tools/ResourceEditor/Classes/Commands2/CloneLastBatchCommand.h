#ifndef __CLONE_LAST_BATCH_COMMAND_H__
#define __CLONE_LAST_BATCH_COMMAND_H__

#include "Commands2/Base/Command2.h"

#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/RenderObject.h"

class CloneLastBatchCommand : public Command2
{
public:
    CloneLastBatchCommand(DAVA::RenderObject* renderObject);
    virtual ~CloneLastBatchCommand();

    virtual void Undo();
    virtual void Redo();

    virtual DAVA::Entity* GetEntity() const;

    inline const DAVA::Vector<DAVA::RenderBatch*>& GetNewBatches() const;

protected:
    DAVA::RenderObject* renderObject;
    DAVA::int32 maxLodIndexes[2];

    DAVA::int32 requestedSwitchIndex;
    DAVA::Vector<DAVA::RenderBatch*> newBatches;
};

inline const DAVA::Vector<DAVA::RenderBatch*>& CloneLastBatchCommand::GetNewBatches() const
{
    return newBatches;
}



#endif // __CLONE_LAST_BATCH_COMMAND_H__

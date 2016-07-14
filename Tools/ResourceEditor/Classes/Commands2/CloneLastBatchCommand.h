#ifndef __CLONE_LAST_BATCH_COMMAND_H__
#define __CLONE_LAST_BATCH_COMMAND_H__

#include "QtTools/Commands/CommandWithoutExecute.h"

#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/RenderObject.h"

class CloneLastBatchCommand : public CommandWithoutExecute
{
public:
    CloneLastBatchCommand(DAVA::RenderObject* renderObject);
    virtual ~CloneLastBatchCommand();

    void Undo() override;
    void Redo() override;

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

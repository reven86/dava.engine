#ifndef __REBUILD_TANGENT_SPACE_COMMAND_H__
#define __REBUILD_TANGENT_SPACE_COMMAND_H__

#include "QtTools/Commands/CommandWithoutExecute.h"

#include "Render/Highlevel/RenderBatch.h"

class RebuildTangentSpaceCommand : public CommandWithoutExecute
{
public:
    RebuildTangentSpaceCommand(DAVA::RenderBatch* renderBatch, bool computeBinormal);
    virtual ~RebuildTangentSpaceCommand();

    void Undo() override;
    void Redo() override;

protected:
    DAVA::RenderBatch* renderBatch;
    bool computeBinormal;
    DAVA::PolygonGroup* originalGroup;
    DAVA::int32 materialBinormalFlagState;
};


#endif
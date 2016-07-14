#ifndef __BAKE_TRANSFORM_COMMAND_H__
#define __BAKE_TRANSFORM_COMMAND_H__

#include "QtTools/Commands/CommandWithoutExecute.h"
#include "Render/Highlevel/RenderObject.h"

class BakeGeometryCommand : public CommandWithoutExecute
{
public:
    BakeGeometryCommand(DAVA::RenderObject* _object, DAVA::Matrix4 _transform);
    ~BakeGeometryCommand();

    void Undo() override;
    void Redo() override;

protected:
    DAVA::RenderObject* object;
    DAVA::Matrix4 transform;
};

#endif // __BAKE_COMMAND_BATCH_H__

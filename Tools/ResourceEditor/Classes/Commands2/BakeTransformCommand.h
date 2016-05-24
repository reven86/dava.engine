#ifndef __BAKE_TRANSFORM_COMMAND_H__
#define __BAKE_TRANSFORM_COMMAND_H__

#include "Commands2/Base/Command2.h"
#include "Render/Highlevel/RenderObject.h"

class BakeGeometryCommand : public Command2
{
public:
    BakeGeometryCommand(DAVA::RenderObject* _object, DAVA::Matrix4 _transform);
    ~BakeGeometryCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const
    {
        return NULL;
    }

protected:
    DAVA::RenderObject* object;
    DAVA::Matrix4 transform;
};

#endif // __BAKE_COMMAND_BATCH_H__

#ifndef __DELETE_RENDER_BATCH_COMMAND_H__
#define __DELETE_RENDER_BATCH_COMMAND_H__

#include "Commands2/Base/Command2.h"

#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/RenderObject.h"

class DeleteRenderBatchCommand : public Command2
{
public:
    DeleteRenderBatchCommand(DAVA::Entity* entity, DAVA::RenderObject* renderObject, DAVA::uint32 renderBatchIndex);
    virtual ~DeleteRenderBatchCommand();

    virtual void Undo();
    virtual void Redo();

    virtual DAVA::Entity* GetEntity() const;

    DAVA::RenderBatch* GetRenderBatch() const;

protected:
    DAVA::Entity* entity;
    DAVA::RenderObject* renderObject;
    DAVA::RenderBatch* renderBatch;

    DAVA::int32 lodIndex;
    DAVA::int32 switchIndex;
};


#endif // __DELETE_RENDER_BATCH_COMMAND_H__

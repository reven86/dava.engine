#ifndef __DELETE_RENDER_BATCH_COMMAND_H__
#define __DELETE_RENDER_BATCH_COMMAND_H__

#include "Commands2/Base/RECommand.h"

#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/RenderObject.h"

class DeleteRenderBatchCommand : public RECommand
{
public:
    DeleteRenderBatchCommand(DAVA::Entity* entity, DAVA::RenderObject* renderObject, DAVA::uint32 renderBatchIndex);
    virtual ~DeleteRenderBatchCommand();

    void Undo() override;
    void Redo() override;

    DAVA::Entity* GetEntity() const;

    DAVA::RenderBatch* GetRenderBatch() const;

protected:
    DAVA::Entity* entity;
    DAVA::RenderObject* renderObject;
    DAVA::RenderBatch* renderBatch;

    DAVA::int32 lodIndex;
    DAVA::int32 switchIndex;
};


#endif // __DELETE_RENDER_BATCH_COMMAND_H__

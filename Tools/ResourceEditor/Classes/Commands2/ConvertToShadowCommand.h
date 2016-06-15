#ifndef __CONVERT_TO_SHADOW_COMMAND_H__
#define __CONVERT_TO_SHADOW_COMMAND_H__

#include "Commands2/Base/Command2.h"
#include "DAVAEngine.h"

class ConvertToShadowCommand : public Command2
{
public:
    ConvertToShadowCommand(DAVA::Entity* entity, DAVA::RenderBatch* batch);
    ~ConvertToShadowCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const;

    static bool CanConvertBatchToShadow(DAVA::RenderBatch* renderBatch);

    DAVA::Entity* entity;
    DAVA::RenderObject* renderObject;
    DAVA::RenderBatch* oldBatch;
    DAVA::RenderBatch* newBatch;
};


#endif // #ifndef __CONVERT_TO_SHADOW_COMMAND_H__
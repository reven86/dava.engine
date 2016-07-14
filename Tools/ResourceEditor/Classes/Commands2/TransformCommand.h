#ifndef __TRANSFORM_COMMAND_H__
#define __TRANSFORM_COMMAND_H__

#include "Qt/Scene/Selectable.h"
#include "QtTools/Commands/CommandWithoutExecute.h"

class TransformCommand : public CommandWithoutExecute
{
public:
    TransformCommand(Selectable object, const DAVA::Matrix4& origTransform, const DAVA::Matrix4& newTransform);

    void Undo() override;
    void Redo() override;

    DAVA::Entity* GetEntity() const;
    const Selectable& GetTransformedObject() const;

protected:
    Selectable object;
    DAVA::Matrix4 undoTransform;
    DAVA::Matrix4 redoTransform;
};

#endif // __COMMAND_BATCH_H__

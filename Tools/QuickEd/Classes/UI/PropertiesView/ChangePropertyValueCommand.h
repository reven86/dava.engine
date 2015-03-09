#ifndef __CHANGEPROPERTYVALUECOMMAND_H__
#define __CHANGEPROPERTYVALUECOMMAND_H__
#include <QUndoStack>
#include "FileSystem/VariantType.h"

class BaseProperty;
class ChangePropertyValueCommand: public QUndoCommand
{
public:
    explicit ChangePropertyValueCommand(BaseProperty *property, const DAVA::VariantType &newValue, QUndoCommand *parent = 0);
    explicit ChangePropertyValueCommand(BaseProperty *property, QUndoCommand *parent = 0);
    virtual ~ChangePropertyValueCommand();

    virtual void undo();
    virtual void redo();
private:
    BaseProperty *property;
    DAVA::VariantType oldValue;
    DAVA::VariantType newValue;
    bool revertToDefault;
};

#endif // __CHANGEPROPERTYVALUECOMMAND_H__

#ifndef __QUICKED_CHANGE_DEFAULT_VALUE_COMMAND_H__
#define __QUICKED_CHANGE_DEFAULT_VALUE_COMMAND_H__

#include <QUndoStack>
#include "FileSystem/VariantType.h"

class BaseProperty;

class ChangeDefaultValueCommand: public QUndoCommand
{
    
public:
    ChangeDefaultValueCommand(BaseProperty *property, const DAVA::VariantType &newValue, QUndoCommand *parent = 0);
    virtual ~ChangeDefaultValueCommand();
    
    virtual void undo();
    virtual void redo();
private:
    BaseProperty *property;
    DAVA::VariantType oldValue;
    DAVA::VariantType newValue;
    bool revertToDefault;
};


#endif // __QUICKED_CHANGE_DEFAULT_VALUE_COMMAND_H__

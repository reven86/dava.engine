#ifndef __QUICKED_CHANGE_DEFAULT_VALUE_COMMAND_H__
#define __QUICKED_CHANGE_DEFAULT_VALUE_COMMAND_H__

#include <QUndoCommand>
#include "FileSystem/VariantType.h"

class PackageNode;
class ControlNode;
class BaseProperty;

class ChangeDefaultValueCommand : public QUndoCommand
{
    
public:
    ChangeDefaultValueCommand(PackageNode *_root, ControlNode *_node, BaseProperty *_property, const DAVA::VariantType &_newValue, QUndoCommand *parent = 0);
    virtual ~ChangeDefaultValueCommand();
    
    virtual void redo();
    virtual void undo();
    
private:
    PackageNode *root;
    ControlNode *node;
    BaseProperty *property;
    DAVA::VariantType oldValue;
    DAVA::VariantType newValue;
};


#endif // __QUICKED_CHANGE_DEFAULT_VALUE_COMMAND_H__

#ifndef __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__
#define __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__

#include <QUndoCommand>
#include "FileSystem/VariantType.h"

class PackageNode;
class ControlNode;
class BaseProperty;

class ChangePropertyValueCommand: public QUndoCommand
{
public:
    ChangePropertyValueCommand(PackageNode *_root, ControlNode *_node, BaseProperty *_property, const DAVA::VariantType &newValue, QUndoCommand *parent = 0);
    ChangePropertyValueCommand(PackageNode *_root, ControlNode *_node, BaseProperty *_property, QUndoCommand *parent = 0);
    virtual ~ChangePropertyValueCommand();

    virtual void redo();
    virtual void undo();
    
private:
    PackageNode *root;
    ControlNode *node;
    BaseProperty *property;
    DAVA::VariantType oldValue;
    DAVA::VariantType newValue;
};

#endif // __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__

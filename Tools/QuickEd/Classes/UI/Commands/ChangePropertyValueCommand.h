#ifndef __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__
#define __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__

#include <QUndoStack>
#include "FileSystem/VariantType.h"

class BaseProperty;
class Document;
class ControlNode;

class ChangePropertyValueCommand: public QUndoCommand
{
public:
    explicit ChangePropertyValueCommand(Document *document, ControlNode *node, BaseProperty *property, const DAVA::VariantType &newValue, QUndoCommand *parent = 0);
    explicit ChangePropertyValueCommand(Document *document, ControlNode *node, BaseProperty *property, QUndoCommand *parent = 0);
    virtual ~ChangePropertyValueCommand();

    virtual void undo();
    virtual void redo();
    
private:
    Document *document;
    ControlNode *node;
    BaseProperty *property;
    DAVA::VariantType oldValue;
    DAVA::VariantType newValue;
};

#endif // __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__

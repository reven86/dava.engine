#ifndef __QUICKED_CHANGE_DEFAULT_VALUE_COMMAND_H__
#define __QUICKED_CHANGE_DEFAULT_VALUE_COMMAND_H__

#include <QUndoCommand>
#include "FileSystem/VariantType.h"

class Document;
class ControlNode;
class BaseProperty;

class ChangeDefaultValueCommand : public QUndoCommand
{
    
public:
    ChangeDefaultValueCommand(Document *_document, ControlNode *_node, BaseProperty *property, const DAVA::VariantType &newValue, QUndoCommand *parent = 0);
    virtual ~ChangeDefaultValueCommand();
    
    virtual void undo();
    virtual void redo();
private:
    Document *document;
    ControlNode *node;
    BaseProperty *property;
    DAVA::VariantType oldValue;
    DAVA::VariantType newValue;
};


#endif // __QUICKED_CHANGE_DEFAULT_VALUE_COMMAND_H__

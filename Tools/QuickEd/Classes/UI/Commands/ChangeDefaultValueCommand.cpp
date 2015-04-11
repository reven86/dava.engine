#include "ChangeDefaultValueCommand.h"

#include "Model/ControlProperties/BaseProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"

ChangeDefaultValueCommand::ChangeDefaultValueCommand(PackageNode *_root, ControlNode *_node, BaseProperty *_property, const DAVA::VariantType &_newValue, QUndoCommand *_parent)
    : QUndoCommand(_parent)
    , root(SafeRetain(_root))
    , node(SafeRetain(_node))
    , property(SafeRetain(_property))
    , newValue(_newValue)
{
    oldValue = property->GetDefaultValue();
}

ChangeDefaultValueCommand::~ChangeDefaultValueCommand()
{
    SafeRelease(root);
    SafeRelease(property);
    SafeRelease(node);
}

void ChangeDefaultValueCommand::redo()
{
    root->SetControlDefaultProperty(node, property, newValue);
}

void ChangeDefaultValueCommand::undo()
{
    root->SetControlDefaultProperty(node, property, oldValue);
}

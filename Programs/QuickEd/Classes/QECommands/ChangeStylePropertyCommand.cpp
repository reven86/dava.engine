#include "ChangeStylePropertyCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/ControlProperties/AbstractProperty.h"

ChangeStylePropertyCommand::ChangeStylePropertyCommand(PackageNode* _root, StyleSheetNode* _node, AbstractProperty* _property, const DAVA::Any& _newVal)
    : DAVA::Command(DAVA::String("change ") + _property->GetName().c_str())
    , root(SafeRetain(_root))
    , node(SafeRetain(_node))
    , property(SafeRetain(_property))
    , newValue(_newVal)
{
    oldValue = property->GetValue();
}

ChangeStylePropertyCommand::~ChangeStylePropertyCommand()
{
    SafeRelease(root);
    SafeRelease(node);
    SafeRelease(property);
}

void ChangeStylePropertyCommand::Redo()
{
    root->SetStyleProperty(node, property, newValue);
}

void ChangeStylePropertyCommand::Undo()
{
    root->SetStyleProperty(node, property, oldValue);
}

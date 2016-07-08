#include "ChangeStylePropertyCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/ControlProperties/AbstractProperty.h"

ChangeStylePropertyCommand::ChangeStylePropertyCommand(PackageNode* _root, StyleSheetNode* _node, AbstractProperty* prop, const DAVA::VariantType& newVal, QUndoCommand* parent /*= 0*/)
    : QUndoCommand(parent)
    , root(SafeRetain(_root))
    , node(SafeRetain(_node))
    , property(SafeRetain(prop))
    , newValue(newVal)
{
    oldValue = property->GetValue();
    setText(QString("change %1").arg(QString(property->GetName().c_str())));
}

ChangeStylePropertyCommand::~ChangeStylePropertyCommand()
{
    SafeRelease(root);
    SafeRelease(node);
    SafeRelease(property);
}

void ChangeStylePropertyCommand::redo()
{
    root->SetStyleProperty(node, property, newValue);
}

void ChangeStylePropertyCommand::undo()
{
    root->SetStyleProperty(node, property, oldValue);
}

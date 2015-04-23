#include "ChangePropertyValueCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/AbstractProperty.h"

ChangePropertyValueCommand::ChangePropertyValueCommand(PackageNode *_root, ControlNode *_node, AbstractProperty *prop, const DAVA::VariantType &newVal, QUndoCommand *parent /*= 0*/ )
    : QUndoCommand(parent)
    , root(SafeRetain(_root))
    , node(SafeRetain(_node))
    , property(SafeRetain(prop))
    , newValue(newVal)
{
    if (property->IsReplaced())
    {
        oldValue = property->GetValue();
    }
    setText( QString("change %1").arg(QString(property->GetName().c_str())));
}

ChangePropertyValueCommand::ChangePropertyValueCommand(PackageNode *_root, ControlNode *_node, AbstractProperty *prop, QUndoCommand *parent /*= 0*/ )
    : QUndoCommand(parent)
    , root(SafeRetain(_root))
    , node(SafeRetain(_node))
    , property(SafeRetain(prop))
{
    if (property->IsReplaced())
    {
        oldValue = property->GetValue();
    }
    setText( QString("reset %1").arg(QString(property->GetName().c_str())));
}

ChangePropertyValueCommand::~ChangePropertyValueCommand()
{
    SafeRelease(root);
    SafeRelease(node);
    SafeRelease(property);
}

void ChangePropertyValueCommand::redo()
{
    if (newValue.GetType() == DAVA::VariantType::TYPE_NONE)
        root->ResetControlProperty(node, property);
    else
        root->SetControlProperty(node, property, newValue);
}

void ChangePropertyValueCommand::undo()
{
    if (oldValue.GetType() == DAVA::VariantType::TYPE_NONE)
        root->ResetControlProperty(node, property);
    else
        root->SetControlProperty(node, property, oldValue);
}

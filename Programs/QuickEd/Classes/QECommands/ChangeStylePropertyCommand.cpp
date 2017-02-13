#include "ChangeStylePropertyCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/ControlProperties/AbstractProperty.h"

ChangeStylePropertyCommand::ChangeStylePropertyCommand(PackageNode* root_, StyleSheetNode* node_, AbstractProperty* property_, const DAVA::Any& newVal_)
    : DAVA::Command(DAVA::String("change ") + property_->GetName().c_str())
    , root(SafeRetain(root_))
    , node(SafeRetain(node_))
    , property(SafeRetain(property_))
    , newValue(newVal_)
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

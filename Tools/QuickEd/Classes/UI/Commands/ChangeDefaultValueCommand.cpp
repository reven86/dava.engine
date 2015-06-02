#include "ChangeDefaultValueCommand.h"

#include "Model/ControlProperties/BaseProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Document.h"
#include "UI/Properties/PropertiesModel.h"
#include "UI/Package/PackageModel.h"

ChangeDefaultValueCommand::ChangeDefaultValueCommand(Document *_document, ControlNode *_node, BaseProperty *_property, const DAVA::VariantType &_newValue, QUndoCommand *_parent)
    : QUndoCommand(_parent)
    , document(_document)
    , node(SafeRetain(_node))
    , property(SafeRetain(_property))
    , newValue(_newValue)
{
    oldValue = property->GetDefaultValue();
}

ChangeDefaultValueCommand::~ChangeDefaultValueCommand()
{
    SafeRelease(property);
    SafeRelease(node);
    document = nullptr;
}

void ChangeDefaultValueCommand::undo()
{
    property->SetDefaultValue(oldValue);

    PropertiesModel *model = document->GetPropertiesModel();
    if (model && model->GetControlNode()->GetPropertiesRoot() == property->GetRootProperty())
        model->emitPropertyChanged(property);

    PackageModel *packageModel = document->GetPackageModel();
    packageModel->emitNodeChanged(node);

}

void ChangeDefaultValueCommand::redo()
{
    property->SetDefaultValue(newValue);

    PropertiesModel *model = document->GetPropertiesModel();
    if (model && model->GetControlNode()->GetPropertiesRoot() == property->GetRootProperty())
        model->emitPropertyChanged(property);
    
    PackageModel *packageModel = document->GetPackageModel();
    packageModel->emitNodeChanged(node);

}

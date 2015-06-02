#include "ChangePropertyValueCommand.h"

#include "Model/ControlProperties/BaseProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Document.h"
#include "UI/Properties/PropertiesModel.h"
#include "UI/Package/PackageModel.h"


ChangePropertyValueCommand::ChangePropertyValueCommand(Document *_document, ControlNode *_node, BaseProperty *prop, const DAVA::VariantType &newVal, QUndoCommand *parent /*= 0*/ )
    : QUndoCommand(parent)
    , document(_document)
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

ChangePropertyValueCommand::ChangePropertyValueCommand(Document *_document, ControlNode *_node, BaseProperty *prop, QUndoCommand *parent /*= 0*/ )
    : QUndoCommand(parent)
    , document(_document)
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
    SafeRelease(node);
    SafeRelease(property);
    document = nullptr;
}

void ChangePropertyValueCommand::undo()
{
    if (oldValue.GetType() == DAVA::VariantType::TYPE_NONE)
        property->ResetValue();
    else
        property->SetValue(oldValue);

    PropertiesModel *model = document->GetPropertiesModel();
    if (model && model->GetControlNode()->GetPropertiesRoot() == property->GetRootProperty())
        model->emitPropertyChanged(property);

    PackageModel *packageModel = document->GetPackageModel();
    packageModel->emitNodeChanged(node);
}

void ChangePropertyValueCommand::redo()
{
    if (newValue.GetType() == DAVA::VariantType::TYPE_NONE)
        property->ResetValue();
    else
        property->SetValue(newValue);
    
    PropertiesModel *propertiesModel = document->GetPropertiesModel();
    if (propertiesModel && propertiesModel->GetControlNode()->GetPropertiesRoot() == property->GetRootProperty())
        propertiesModel->emitPropertyChanged(property);
    
    PackageModel *packageModel = document->GetPackageModel();
    packageModel->emitNodeChanged(node);
}

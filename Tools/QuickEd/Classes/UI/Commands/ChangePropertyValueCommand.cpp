#include "ChangePropertyValueCommand.h"

#include "Model/ControlProperties/BaseProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "UI/Document.h"
#include "UI/PropertiesContext.h"
#include "UI/Properties/PropertiesModel.h"


ChangePropertyValueCommand::ChangePropertyValueCommand(Document *_document, BaseProperty *prop, const DAVA::VariantType &newVal, QUndoCommand *parent /*= 0*/ )
    : QUndoCommand(parent)
    , document(_document)
    , property(SafeRetain(prop))
    , newValue(newVal)
{
    if (property->IsReplaced())
    {
        oldValue = property->GetValue();
    }
    setText( QString("change %1").arg(QString(property->GetName().c_str())));
}

ChangePropertyValueCommand::ChangePropertyValueCommand(Document *_document, BaseProperty *prop, QUndoCommand *parent /*= 0*/ )
    : QUndoCommand(parent)
    , document(_document)
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
    SafeRelease(property);
    document = nullptr;
}

void ChangePropertyValueCommand::undo()
{
    if (oldValue.GetType() == DAVA::VariantType::TYPE_NONE)
        property->ResetValue();
    else
        property->SetValue(oldValue);

    PropertiesModel *model = document->GetPropertiesContext()->GetModel();
    if (model && model->GetControlNode()->GetPropertiesRoot() == property->GetRootProperty())
        model->emityPropertyChanged(property);
}

void ChangePropertyValueCommand::redo()
{
    if (newValue.GetType() == DAVA::VariantType::TYPE_NONE)
        property->ResetValue();
    else
        property->SetValue(newValue);
    
    PropertiesModel *model = document->GetPropertiesContext()->GetModel();
    if (model && model->GetControlNode()->GetPropertiesRoot() == property->GetRootProperty())
        model->emityPropertyChanged(property);
}

#include "ChangePropertyValueCommand.h"
#include "UIControls/ControlProperties/BaseProperty.h"

ChangePropertyValueCommand::ChangePropertyValueCommand( BaseProperty *prop, const DAVA::VariantType &newVal, QUndoCommand *parent /*= 0*/ )
    : QUndoCommand(parent)
    , property(SafeRetain(prop))
    , newValue(newVal)
{
    if (property->IsReplaced())
    {
        oldValue = property->GetValue();
    }
    setText( QString("change %1").arg(QString(property->GetName().c_str())));
}

ChangePropertyValueCommand::ChangePropertyValueCommand( BaseProperty *prop, QUndoCommand *parent /*= 0*/ )
    : QUndoCommand(parent)
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
}

void ChangePropertyValueCommand::undo()
{
    if (oldValue.GetType() == DAVA::VariantType::TYPE_NONE)
        property->ResetValue();
    else
        property->SetValue(oldValue);
}

void ChangePropertyValueCommand::redo()
{
    if (newValue.GetType() == DAVA::VariantType::TYPE_NONE)
        property->ResetValue();
    else
        property->SetValue(newValue);
}

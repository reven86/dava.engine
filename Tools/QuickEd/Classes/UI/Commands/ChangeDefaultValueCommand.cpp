#include "ChangeDefaultValueCommand.h"

#include "Model/ControlProperties/BaseProperty.h"

ChangeDefaultValueCommand::ChangeDefaultValueCommand(BaseProperty *_property, const DAVA::VariantType &_newValue, QUndoCommand *_parent)
    : QUndoCommand(_parent)
    , property(SafeRetain(_property))
    , newValue(_newValue)
{
    oldValue = property->GetDefaultValue();
}

ChangeDefaultValueCommand::~ChangeDefaultValueCommand()
{
    SafeRelease(property);
}

void ChangeDefaultValueCommand::undo()
{
    property->SetDefaultValue(oldValue);
}

void ChangeDefaultValueCommand::redo()
{
    property->SetDefaultValue(newValue);
}

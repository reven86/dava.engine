#include "SetFieldValueCommand.h"

#include "Classes/Commands2/RECommandIDs.h"

SetFieldValueCommand::SetFieldValueCommand(const DAVA::Reflection::Field& field, const DAVA::Any& newValue_)
    : RECommand(CMDID_REFLECTED_FIELD_MODIFY, "")
    , newValue(newValue_)
    , reflection(field.ref)
{
    oldValue = reflection.GetValue();
}

void SetFieldValueCommand::Redo()
{
    reflection.SetValueWithCast(newValue);
}

void SetFieldValueCommand::Undo()
{
    reflection.SetValueWithCast(oldValue);
}
#include "SetFieldValueCommand.h"

#include "Classes/Commands2/RECommandIDs.h"

SetFieldValueCommand::SetFieldValueCommand(const DAVA::Reflection::Field& field_, const DAVA::Any& newValue_)
    : RECommand(CMDID_REFLECTED_FIELD_MODIFY, "")
    , newValue(newValue_)
    , field(field_)
{
    oldValue = field.ref.GetValue();
}

void SetFieldValueCommand::Redo()
{
    field.ref.SetValueWithCast(newValue);
}

void SetFieldValueCommand::Undo()
{
    field.ref.SetValueWithCast(oldValue);
}

const DAVA::Reflection::Field& SetFieldValueCommand::GetField() const
{
    return field;
}

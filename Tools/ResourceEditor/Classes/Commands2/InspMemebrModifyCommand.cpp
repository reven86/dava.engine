#include "Commands2/InspMemberModifyCommand.h"

InspMemberModifyCommand::InspMemberModifyCommand(const DAVA::InspMember* _member, void* _object, const DAVA::VariantType& _newValue)
    : Command2(CMDID_INSP_MEMBER_MODIFY, "Modify value")
    , member(_member)
    , object(_object)
    , newValue(_newValue)
{
    if (NULL != member && NULL != object)
    {
        oldValue = member->Value(object);
    }
}

InspMemberModifyCommand::~InspMemberModifyCommand()
{
}

void InspMemberModifyCommand::Undo()
{
    if (NULL != member && NULL != object)
    {
        member->SetValue(object, oldValue);
    }
}

void InspMemberModifyCommand::Redo()
{
    if (NULL != member && NULL != object)
    {
        member->SetValue(object, newValue);
    }
}

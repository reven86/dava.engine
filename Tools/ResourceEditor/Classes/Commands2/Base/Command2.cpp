#include "Commands2/Base/Command2.h"

Command2::Command2(DAVA::int32 _id, const DAVA::String& _text)
    : text(_text)
    , id(_id)
{
}

Command2::~Command2() = default;

void Command2::UndoInternalCommand(Command2* command)
{
    command->Undo();
    EmitNotify(command, false);
}

void Command2::RedoInternalCommand(Command2* command)
{
    command->Redo();
    EmitNotify(command, true);
}

bool Command2::MatchCommandID(DAVA::int32 commandID) const
{
    return (id == commandID);
}

bool Command2::MatchCommandIDs(const DAVA::Vector<DAVA::int32>& commandIDVector) const
{
    for (auto commandID : commandIDVector)
    {
        if (id == commandID)
        {
            return true;
        }
    }

    return false;
}

void Command2::Execute()
{
    Redo();
}

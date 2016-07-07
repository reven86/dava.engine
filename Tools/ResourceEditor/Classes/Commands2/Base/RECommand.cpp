#include "Commands2/Base/RECommand.h"

RECommand::RECommand(DAVA::CommandID_t id, const DAVA::String& text)
    : Command(id, text)
{
}

RECommand::~RECommand() = default;

DAVA::Command::Pointer RECommand::CreateEmptyCommand()
{
    return Pointer();
}

DAVA::Entity* RECommand::GetEntity() const
{
    return nullptr;
}

bool RECommand::MatchCommandID(DAVA::int32 commandID) const
{
    return (id == commandID);
}

bool RECommand::MatchCommandIDs(const DAVA::Vector<DAVA::int32>& commandIDVector) const
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

void RECommand::Execute()
{
    Redo();
}

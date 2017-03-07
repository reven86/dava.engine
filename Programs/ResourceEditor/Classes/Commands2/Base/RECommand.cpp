#include "Commands2/Base/RECommand.h"

RECommand::RECommand(DAVA::CommandID id, const DAVA::String& description_)
    : Command(id, description_)
{
}

bool RECommand::MatchCommandID(DAVA::CommandID commandID) const
{
    return GetID() == commandID;
}

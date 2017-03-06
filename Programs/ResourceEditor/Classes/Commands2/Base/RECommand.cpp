#include "Commands2/Base/RECommand.h"

RECommand::RECommand(DAVA::int32 id, const DAVA::String& description_)
    : Command(id, description_)
{
}

bool RECommand::MatchCommandID(DAVA::int32 commandID) const
{
    return GetID() == commandID;
}

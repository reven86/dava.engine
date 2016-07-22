#include "Commands2/Base/RECommand.h"

RECommand::RECommand(DAVA::uint32 id_, const DAVA::String& description_)
    : Command(description_)
    , id(id_)
{
}

bool RECommand::MatchCommandIDs(const DAVA::Vector<DAVA::uint32>& commandIDVector) const
{
    auto functor = [this](const DAVA::uint32& id) { return MatchCommandID(id); };
    return std::find_if(commandIDVector.begin(), commandIDVector.end(), functor) != commandIDVector.end();
}

bool IsRECommand(const DAVA::Command* command)
{
    return dynamic_cast<const RECommand*>(command) != nullptr;
}

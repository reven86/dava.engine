#include "Command/Command.h"
#include <functional>

namespace DAVA
{
Command::Command(CommandID_t id_, const String& text_)
    : id(id_)
    , text(text_)
{
}

bool Command::MatchCommandID(DAVA::CommandID_t commandID) const
{
    return (id == commandID);
}

bool Command::MatchCommandIDs(const DAVA::Vector<DAVA::CommandID_t>& commandIDVector) const
{
    auto functor = std::bind(&Command::MatchCommandID, this, std::placeholders::_1);
    return std::find_if(commandIDVector.begin(), commandIDVector.end(), functor) != commandIDVector.end();
}
}

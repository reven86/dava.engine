#include "Command/Command.h"
#include <functional>

namespace DAVA
{
Command::Command(CommandID_t id_, const String& description_)
    : id(id_)
    , description(description_)
{
}

bool Command::MatchCommandIDs(const DAVA::Vector<DAVA::CommandID_t>& commandIDVector) const
{
    auto functor = [this](const DAVA::CommandID_t& id) { return MatchCommandID(id); };
    return std::find_if(commandIDVector.begin(), commandIDVector.end(), functor) != commandIDVector.end();
}
}

#include "Commands2/Base/RECommandNotificationObject.h"

namespace RECommandNotificationObjectDetail
{
bool MatchID(const RECommandNotificationObject* object, DAVA::CommandID id)
{
    return (object->batch != nullptr) ? object->batch->MatchCommandID(id) : object->command->MatchCommandID(id);
}
}

bool RECommandNotificationObject::IsEmpty() const
{
    return (batch == nullptr) && (command == nullptr);
}

void RECommandNotificationObject::ExecuteForAllCommands(const DAVA::Function<void(const RECommand*, bool)>& fn) const
{
    if (batch != nullptr)
    {
        for (DAVA::uint32 i = 0, count = batch->Size(); i < count; ++i)
        {
            fn(batch->GetCommand(i), redo);
        }
    }
    else
    {
        fn(command, redo);
    }
}

bool RECommandNotificationObject::MatchCommandID(DAVA::CommandID commandID) const
{
    if (IsEmpty())
        return false;

    return RECommandNotificationObjectDetail::MatchID(this, commandID);
}

bool RECommandNotificationObject::MatchCommandIDs(const DAVA::Vector<DAVA::CommandID>& commandIDVector) const
{
    if (IsEmpty())
        return false;

    auto functor = [this](DAVA::uint32 id) { return MatchCommandID(id); };
    return std::find_if(commandIDVector.begin(), commandIDVector.end(), functor) != commandIDVector.end();
}

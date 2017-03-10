#include "Commands2/Base/RECommandNotificationObject.h"
#include "Commands2/Base/RECommandIDHandler.h"

namespace RECommandNotificationObjectDetail
{
const RECommandIDHandler* GetIDHandler(const RECommandNotificationObject* object)
{
    const RECommandIDHandler* batchHandler = object->batch;
    const RECommandIDHandler* commandHandler = object->command;

    return (batchHandler != nullptr) ? batchHandler : commandHandler;
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

bool RECommandNotificationObject::MatchCommandID(DAVA::uint32 commandID) const
{
    if (IsEmpty())
        return false;

    const RECommandIDHandler* idHandler = RECommandNotificationObjectDetail::GetIDHandler(this);
    return idHandler->MatchCommandID(commandID);
}

bool RECommandNotificationObject::MatchCommandIDs(const DAVA::Vector<DAVA::uint32>& commandIDVector) const
{
    if (IsEmpty())
        return false;

    const RECommandIDHandler* idHandler = RECommandNotificationObjectDetail::GetIDHandler(this);
    return idHandler->MatchCommandIDs(commandIDVector);
}

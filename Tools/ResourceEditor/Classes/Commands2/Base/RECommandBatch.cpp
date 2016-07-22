#include "Commands2/Base/RECommandBatch.h"

#include "Debug/DVAssert.h"
#include "Utils/StringFormat.h"
#include "Commands2/Base/RECommand.h"
#include "Commands2/RECommandIDs.h"

RECommandBatch::RECommandBatch(const DAVA::String& description, DAVA::uint32 commandsCount)
    : CommandBatch(description, commandsCount)
    , RECommand(CMDID_BATCH1, description)
{
}

RECommand* RECommandBatch::GetCommand(DAVA::uint32 index) const
{
    if (index < static_cast<DAVA::uint32>(commandList.size()))
    {
        DAVA::Command* command = commandList.at(index).get();
        return DAVA::DynamicTypeCheck<RECommand*>(command);
    }

    DVASSERT_MSG(false, DAVA::Format("command at index %u, in batch with size %u not found", index, static_cast<DAVA::uint32>(commandList.size())).c_str());
    return nullptr;
}

void RECommandBatch::RemoveCommands(DAVA::uint32 commandId)
{
    auto it = std::remove_if(commandList.begin(), commandList.end(), [commandId](const Pointer& cmd) {
        const DAVA::Command* commandPtr = cmd.get();
        if (IsRECommand(commandPtr))
        {
            return static_cast<const RECommand*>(commandPtr)->GetID() == commandId;
        }
        return false;
    });
    commandList.erase(it, commandList.end());

    for (const Pointer& command : commandList)
    {
        DAVA::Command* commandPtr = command.get();
        if (IsCommandBatch(commandPtr))
        {
            RECommandBatch* batch = static_cast<RECommandBatch*>(static_cast<DAVA::CommandBatch*>(commandPtr));
            batch->RemoveCommands(commandId);
        }
    }
}

bool RECommandBatch::MatchCommandID(DAVA::uint32 commandId) const
{
    DAVA::uint32 size = commandList.size();
    for (DAVA::uint32 index = 0; index < size; ++index)
    {
        const DAVA::Command* commandPtr = commandList.at(index).get();
        if (IsCommandBatch(commandPtr))
        {
            const RECommandBatch* reCommandBatch = static_cast<const RECommandBatch*>(static_cast<const DAVA::CommandBatch*>(commandPtr));
            if (reCommandBatch->MatchCommandID(commandId))
            {
                return true;
            }
        }
        else if (IsRECommand(commandPtr))
        {
            const RECommand* reCommandPtr = static_cast<const RECommand*>(commandPtr);
            if (reCommandPtr->GetID() == commandId)
            {
                return true;
            }
        }
    }
    return false;
}

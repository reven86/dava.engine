#include "Commands2/Base/RECommandBatch.h"

#include "Debug/DVAssert.h"
#include "Utils/StringFormat.h"
#include "Commands2/Base/RECommand.h"
#include "Commands2/RECommandIDs.h"

RECommandBatch::RECommandBatch(const DAVA::String& description, DAVA::uint32 commandsCount)
    : CommandBatch(description, commandsCount)
    , RECommand(description)
{
}

RECommand* RECommandBatch::GetCommand(DAVA::uint32 index) const
{
    if (index < static_cast<DAVA::uint32>(commandList.size()))
    {
        DAVA::Command* cmd = commandList.at(index).get();
        return DAVA::DynamicTypeCheck<RECommand*>(cmd);
    }

    DVASSERT_MSG(false, DAVA::Format("command at index %u, in batch with size %u not found", index, static_cast<DAVA::uint32>(commandList.size())).c_str());
    return nullptr;
}

void RECommandBatch::RemoveCommands(DAVA::uint32 commandId)
{
    auto it = std::remove_if(commandList.begin(), commandList.end(), [commandId](const Pointer& cmd) {
        return cmd->GetID() == commandId;
    });
    commandList.erase(it, commandList.end());

    for (const Pointer& command : commandList)
    {
        if (IsCommandBatch(command.get()))
        {
            RECommandBatch* batch = static_cast<RECommandBatch*>(command.get());
            batch->RemoveCommands(commandId);
        }
    }
}

bool RECommandBatch::IsMultiCommandBatch() const
{
    DAVA::uint32 size = commandList.size();

    if (size < 2)
    {
        return false;
    }

    DAVA::UnorderedSet<DAVA::uint32> commandIDs;
    for (DAVA::uint32 index = 0; index < size && commandIDs.size() < 2; ++index)
    {
        const RECommand* command = dynamic_cast<const RECommand*>(commandList.at(index).get());
        if (command != nullptr)
        {
            commandIDs.insert(command->GetID());
        }
    }
    return commandIDs.size() > 1;
}

bool RECommandBatch::MatchCommandID(DAVA::uint32 commandId) const
{
    DAVA::uint32 size = commandList.size();

    if (size < 2)
    {
        return false;
    }

    DAVA::UnorderedSet<DAVA::uint32> commandIDs;
    for (DAVA::uint32 index = 0; index < size && commandIDs.size() < 2; ++index)
    {
        if ()
            const RECommand* command = dynamic_cast<const RECommand*>(commandList.at(index).get());
        if (command != nullptr)
        {
            if (command->GetID() == commandId)
            {
                return true;
            }
        }
    }
    return false;
}

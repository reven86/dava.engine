#include "Commands2/Base/RECommandBatch.h"

#include "Debug/DVAssert.h"
#include "Utils/StringFormat.h"
#include "Commands2/Base/RECommand.h"
#include "Commands2/RECommandIDs.h"

RECommandBatch::RECommandBatch(const DAVA::String& description, DAVA::uint32 commandsCount)
    : CommandBatch(description, commandsCount)
    , RECommand(CMDID_BATCH, description)
{
}

void RECommandBatch::AddAndRedo(Pointer&& command)
{
    DAVA::Command* commandPtr = command.get();
    RECommand* reCommandPtr = dynamic_cast<RECommand*>(commandPtr);
    if (reCommandPtr != nullptr)
    {
        commandIDs.insert(reCommandPtr->GetID());
    }
    CommandBatch::AddAndRedo(command);
}

RECommand* RECommandBatch::GetCommand(DAVA::uint32 index) const
{
    if (index < static_cast<DAVA::uint32>(commandList.size()))
    {
        DAVA::Command* cmd = commandList.at(index).get();
        return DAVA::DynamicTypeCheck<RECommand*>(cmd);
    }

    DVASSERT_MSG(false, DAVA::Format("index %u, size %u", index, static_cast<DAVA::uint32>(commandList.size())).c_str());
    return nullptr;
}

void RECommandBatch::RemoveCommands(DAVA::uint32 commandId)
{
    auto it = std::remove_if(commandList.begin(), commandList.end(), [commandId](const Pointer& cmd) {
        return cmd->GetID() == commandId;
    });
    commandList.erase(it, commandList.end());

    if (commandId != CMDID_BATCH)
    {
        for (const Pointer& command : commandList)
        {
            if (command->GetID() == CMDID_BATCH)
            {
                RECommandBatch* batch = static_cast<RECommandBatch*>(command.get());
                batch->RemoveCommands(commandId);
            }
        }
    }
}

bool RECommandBatch::IsMultiCommandBatch() const
{
    return commandIDs.size() > 1;
}

bool RECommandBatch::MatchCommandID(DAVA::uint32 commandId) const
{
    return commandIDs.count(commandId) > 0;
}

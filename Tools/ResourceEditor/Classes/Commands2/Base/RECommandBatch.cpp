#include "Commands2/Base/RECommandBatch.h"

#include "Debug/DVAssert.h"
#include "Utils/StringFormat.h"
#include "QtTools/Commands/CommandWithoutExecute.h"

RECommandBatch::RECommandBatch(const DAVA::String& description, DAVA::uint32 commandsCount)
    : CommandBatch(description, commandsCount)
{
}

CommandWithoutExecute* RECommandBatch::GetCommand(DAVA::uint32 index) const
{
    if (index < static_cast<DAVA::uint32>(commandList.size()))
    {
        DAVA::Command* cmd = commandList.at(index).get();
        return DAVA::DynamicTypeCheck<CommandWithoutExecute*>(cmd);
    }

    DVASSERT_MSG(false, DAVA::Format("index %u, size %u", index, static_cast<DAVA::uint32>(commandList.size())).c_str());
    return nullptr;
}

void RECommandBatch::RemoveCommands(DAVA::CommandID_t commandId)
{
    auto it = std::remove_if(commandList.begin(), commandList.end(), [commandId](const Pointer& cmd) {
        return cmd->GetID() == commandId;
    });
    commandList.erase(it, commandList.end());
    commandIDs.erase(commandId);

    if (commandId != DAVA::CMDID_BATCH)
    {
        for (const Pointer& command : commandList)
        {
            if (command->GetID() == DAVA::CMDID_BATCH)
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

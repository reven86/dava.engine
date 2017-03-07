#include "Commands2/Base/RECommandBatch.h"

#include "Debug/DVAssert.h"
#include "Utils/StringFormat.h"
#include "Base/TemplateHelpers.h"
#include "Commands2/RECommandIDs.h"
#include "Commands2/Base/RECommand.h"

RECommandBatch::RECommandBatch(const DAVA::String& description, DAVA::uint32 commandsCount)
    : CommandBatch(description, commandsCount)
{
}

RECommand* RECommandBatch::GetCommand(DAVA::uint32 index) const
{
    if (index < static_cast<DAVA::uint32>(commands.size()))
    {
        DAVA::Command* command = commands.at(index).get();
        return DAVA::DynamicTypeCheck<RECommand*>(command);
    }

    DVASSERT(false, DAVA::Format("command at index %u, in batch with size %u not found", index, static_cast<DAVA::uint32>(commands.size())).c_str());
    return nullptr;
}

void RECommandBatch::RemoveCommands(DAVA::CommandID commandId)
{
    auto it = std::remove_if(commands.begin(), commands.end(), [commandId](const std::unique_ptr<DAVA::Command>& cmd)
                             {
                                 const DAVA::Command* commandPtr = cmd.get();
                                 if (IsCommandBatch(commandPtr))
                                 {
                                     return (commandId == DAVA::COMMAND_BATCH);
                                 }
                                 return static_cast<const RECommand*>(commandPtr)->GetID() == commandId;
                             });
    commands.erase(it, commands.end());

    for (const std::unique_ptr<DAVA::Command>& command : commands)
    {
        DAVA::Command* commandPtr = command.get();
        if (IsCommandBatch(commandPtr))
        {
            RECommandBatch* batch = static_cast<RECommandBatch*>(static_cast<DAVA::CommandBatch*>(commandPtr));
            batch->RemoveCommands(commandId);
        }
    }
}

bool RECommandBatch::MatchCommandID(DAVA::CommandID commandId) const
{
    DAVA::uint32 size = static_cast<DAVA::uint32>(commands.size());
    for (DAVA::uint32 index = 0; index < size; ++index)
    {
        const DAVA::Command* commandPtr = commands.at(index).get();
        if (IsCommandBatch(commandPtr))
        {
            const RECommandBatch* reCommandBatch = static_cast<const RECommandBatch*>(static_cast<const DAVA::CommandBatch*>(commandPtr));
            if (reCommandBatch->MatchCommandID(commandId))
            {
                return true;
            }
        }
        else
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

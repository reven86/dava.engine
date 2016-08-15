#include "Commands2/Base/CommandBatch.h"

#include "Debug/DVAssert.h"
#include "Utils/StringFormat.h"

CommandBatch::CommandBatch(const DAVA::String& text, DAVA::uint32 commandsCount)
    : Command2(CMDID_BATCH, text)
{
    DVASSERT(commandsCount > 0);
    commandList.reserve(commandsCount);
}

void CommandBatch::Redo()
{
    for (CommandsContainer::iterator i = commandList.begin(), end = commandList.end(); i != end; i++)
    {
        (*i)->Redo();
    }
}

void CommandBatch::Undo()
{
    for (CommandsContainer::reverse_iterator i = commandList.rbegin(), end = commandList.rend(); i != end; i++)
    {
        (*i)->Undo();
    }
}

DAVA::Entity* CommandBatch::GetEntity() const
{
    return nullptr;
}

void CommandBatch::AddAndExec(Command2::Pointer&& command)
{
    DVASSERT(command);

    Command2* actualCommand = command.get();
    commandList.emplace_back(std::move(command));
    commandIDs.insert(actualCommand->GetId());
    actualCommand->Redo();
}

Command2* CommandBatch::GetCommand(DAVA::uint32 index) const
{
    if (index < static_cast<DAVA::uint32>(commandList.size()))
    {
        return commandList[index].get();
    }

    DVASSERT_MSG(false, DAVA::Format("index %u, size %u", index, static_cast<DAVA::uint32>(commandList.size())).c_str());
    return nullptr;
}

void CommandBatch::RemoveCommands(DAVA::int32 commandId)
{
    auto it = std::remove_if(commandList.begin(), commandList.end(), [commandId](const Command2::Pointer& cmd) {
        return cmd->GetId() == commandId;
    });

    commandList.erase(it, commandList.end());
    commandIDs.erase(commandId);
}

bool CommandBatch::MatchCommandID(DAVA::int32 commandId) const
{
    return commandIDs.count(commandId) > 0;
}

bool CommandBatch::MatchCommandIDs(const DAVA::Vector<DAVA::int32>& commandIdVector) const
{
    for (auto id : commandIdVector)
    {
        if (MatchCommandID(id))
        {
            return true;
        }
    }

    return false;
}

void CommandBatch::Execute()
{
    // empty because we execute commands immediatly after we push it info batch.
    // No need to execute it one more time.
}

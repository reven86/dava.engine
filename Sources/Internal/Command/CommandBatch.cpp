#include "Command/CommandBatch.h"

#include "Debug/DVAssert.h"

namespace DAVA
{
CommandBatch::CommandBatch(const String& text, uint32 commandsCount)
    : Command(CMDID_BATCH, text)
{
    commandList.reserve(commandsCount);
}

void CommandBatch::Execute()
{
    for (CommandsContainer::iterator it = commandList.begin(), end = commandList.end(); it != end; ++it)
    {
        (*it)->Execute();
    }
}

void CommandBatch::Redo()
{
    for (CommandsContainer::iterator it = commandList.begin(), end = commandList.end(); it != end; ++it)
    {
        (*it)->Redo();
    }
}

void CommandBatch::Undo()
{
    for (CommandsContainer::reverse_iterator it = commandList.rbegin(), end = commandList.rend(); it != end; ++it)
    {
        (*it)->Undo();
    }
}

void CommandBatch::AddAndExec(Pointer&& command)
{
    DVASSERT(command);

    Command* actualCommand = command.get();
    commandList.emplace_back(std::move(command));
    commandIDs.insert(actualCommand->GetID());
    actualCommand->Execute();
}

bool CommandBatch::MatchCommandID(DAVA::CommandID_t commandId) const
{
    return commandIDs.count(commandId) > 0;
}
}

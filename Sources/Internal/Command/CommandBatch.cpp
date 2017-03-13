#include "Command/CommandBatch.h"

#include "Debug/DVAssert.h"

namespace DAVA
{
CommandBatch::CommandBatch(const String& description, uint32 commandsCount)
    : Command(description)
{
    commandList.reserve(commandsCount);
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

void CommandBatch::AddAndRedo(std::unique_ptr<Command>&& command)
{
    DVASSERT(command);

    Command* actualCommand = command.get();
    commandList.emplace_back(std::move(command));
    actualCommand->Redo();
}

void CommandBatch::Add(std::unique_ptr<Command>&& command)
{
    DVASSERT(command);
    commandList.emplace_back(std::move(command));
}

bool CommandBatch::IsClean() const
{
    return std::find_if(commandList.begin(), commandList.end(), [](const std::unique_ptr<Command>& command) {
               return command->IsClean() == false;
           }) == commandList.end();
}

bool IsCommandBatch(const Command* command)
{
    return dynamic_cast<const CommandBatch*>(command) != nullptr;
}
} //namespace DAVA

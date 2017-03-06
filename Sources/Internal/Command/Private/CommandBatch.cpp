#include "Command/CommandBatch.h"

#include "Debug/DVAssert.h"

namespace DAVA
{
CommandBatch::CommandBatch(const String& description, uint32 commandsCount)
    : Command(COMMAND_BATCH, description)
{
    commands.reserve(commandsCount);
}

void CommandBatch::Redo()
{
    for (CommandsContainer::iterator it = commands.begin(), end = commands.end(); it != end; ++it)
    {
        (*it)->Redo();
    }
}

void CommandBatch::Undo()
{
    for (CommandsContainer::reverse_iterator it = commands.rbegin(), end = commands.rend(); it != end; ++it)
    {
        (*it)->Undo();
    }
}

void CommandBatch::AddAndRedo(std::unique_ptr<Command>&& command)
{
    DVASSERT(command);

    Command* actualCommand = command.get();
    Add(std::move(command));
    actualCommand->Redo();
}

bool CommandBatch::IsClean() const
{
    return std::find_if(commands.begin(), commands.end(), [](const std::unique_ptr<Command>& command) {
               return command->IsClean() == false;
           }) == commands.end();
}

bool IsCommandBatch(const Command* command)
{
    return dynamic_cast<const CommandBatch*>(command) != nullptr;
}
} //namespace DAVA

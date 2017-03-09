#include "Command/CommandBatch.h"

#include "Debug/DVAssert.h"

namespace DAVA
{
CommandBatch::CommandBatch(const String& description, uint32 commandsCount)
    : Command(BATCH_COMMAND, description)
{
    commands.reserve(commandsCount);
}

void CommandBatch::Redo()
{
    for (auto it = commands.begin(), end = commands.end(); it != end; ++it)
    {
        (*it)->Redo();
    }
}

void CommandBatch::Undo()
{
    for (auto it = commands.rbegin(), end = commands.rend(); it != end; ++it)
    {
        (*it)->Undo();
    }
}

void CommandBatch::Add(std::unique_ptr<Command>&& command)
{
    DVASSERT(command);
    if (commands.empty() == false)
    {
        DAVA::Command* lastCommand = commands.back().get();
        const CommandID id = lastCommand->GetID();
        DVASSERT(id != BATCH_COMMAND, "we can not store batch inside another batch");
        if (id == command->GetID())
        {
            if (lastCommand->MergeWith(command.get()))
            {
                return;
            }
        }
    }
    commands.push_back(std::move(command));
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

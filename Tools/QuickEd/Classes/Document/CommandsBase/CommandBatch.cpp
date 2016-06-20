#include "Document/CommandsBase/CommandBatch.h"
#include "Debug/DVAssert.h"

CommandBatch::CommandBatch(const DAVA::String& text)
    : Command(text)
{
}

void CommandBatch::Execute()
{
}

void CommandBatch::Undo()
{
    for (auto iter = commands.rbegin(); iter != commands.rend(); ++iter)
    {
        (*iter)->Undo();
    }
}

void CommandBatch::Redo()
{
    for (CommandPtr& commandPtr : commands)
    {
        commandPtr->Redo();
    }
}

void CommandBatch::AddAndRedo(CommandPtr&& command)
{
    DVASSERT(command);
    DAVA::ICommand* actualCommand = command.get();
    commands.emplace_back(std::move(command));
    actualCommand->Redo();
}

bool CommandBatch::IsEmpty() const
{
    return commands.empty();
}

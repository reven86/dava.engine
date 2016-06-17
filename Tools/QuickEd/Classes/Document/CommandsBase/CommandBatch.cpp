#include "Document/CommandsBase/CommandBatch.h"
#include "Debug/DVAssert.h"

CommandBatch::CommandBatch(const DAVA::String& text)
    : QECommand(text)
{
}

void CommandBatch::Execute()
{
}

void CommandBatch::Undo()
{
    for (CommandPtr& commandPtr : commands)
    {
        commandPtr->Undo();
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

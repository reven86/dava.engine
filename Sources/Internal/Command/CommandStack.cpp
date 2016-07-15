#include "Command/CommandStack.h"
#include "Command/CommandBatch.h"

namespace DAVA
{
CommandStack::CommandStack()
{
}

CommandStack::~CommandStack()
{
}

void CommandStack::Exec(Command::Pointer&& command)
{
    DVASSERT(command != nullptr);
    if (command == nullptr)
    {
        return;
    }
    if (!batchesInWork.empty())
    {
        //when the macro started command without undo will cause undefined state of application
        DVASSERT_MSG(command->CanUndo(),
                     Format("Command %s, which can not make undo passed to CommandStack within macro %s",
                            command->GetText().c_str(),
                            batchesInWork.top()->GetText().c_str())
                     .c_str()
                     );
        if (command->CanUndo())
        {
            batchesInWork.top()->AddAndExec(std::move(command));
            return;
        }
    }
    commands.push_back(std::move(command));
}

void CommandStack::BeginBatch(const String& name, uint32 commandsCount)
{
    //we call BeginMacro first time
    if (batchesInWork.empty())
    {
        //own first item before we move it to the command manager
        batchesInWork.push(new CommandBatch(name, commandsCount));
    }
    //we already create one or more batches
    else
    {
        Command::Pointer newCommandBatch = Command::Create<CommandBatch>(name, commandsCount);
        CommandBatch* newCommandBatchPtr = static_cast<CommandBatch*>(newCommandBatch.get());
        batchesInWork.top()->AddAndExec(std::move(newCommandBatch));
        batchesInWork.push(newCommandBatchPtr);
    }
}

void CommandStack::EndBatch()
{
    DVASSERT(!batchesInWork.empty() && "CommandStack::EndMacro called without BeginMacro");
    if (batchesInWork.size() == 1)
    {
        CommandBatch* topLevelBatch = batchesInWork.top();
        Command::Pointer commandPtr(topLevelBatch);
        if (!topLevelBatch->IsEmpty())
        {
            commands.push_back(std::move(commandPtr));
        }
    }
    if (!batchesInWork.empty())
    {
        batchesInWork.pop();
    }
}

bool CommandStack::IsClean() const
{
    return isClean;
}

void CommandStack::SetClean()
{
    cleanIndex = currentIndex;
    UpdateCleanState();
}

void CommandStack::Undo()
{
    DVASSERT(CanUndo());
    if (CanUndo())
    {
        commands.at(currentIndex)->Undo();
        SetCurrentIndex(currentIndex - 1);
    }
}

void CommandStack::Redo()
{
    DVASSERT(CanRedo());
    if (CanRedo())
    {
        commands.at(currentIndex)->Redo();
        SetCurrentIndex(currentIndex + 1);
    }
}

bool CommandStack::CanUndo() const
{
    return CanUndoImpl();
}

bool CommandStack::CanRedo() const
{
    return CanRedoImpl();
}

bool CommandStack::CanUndoImpl() const
{
    DVASSERT(currentIndex < commands.size());
    if (currentIndex >= 0)
    {
        return commands.at(currentIndex)->CanUndo();
    }
    return false;
}

bool CommandStack::CanRedoImpl() const
{
    return currentIndex < (commands.size() - 1);
}

void CommandStack::UpdateCleanState()
{
    int begin = std::min(cleanIndex, currentIndex);
    int end = std::max(cleanIndex, currentIndex);
    DVASSERT(end >= begin);
    bool containsModifiedCommands = false;
    for (int index = begin; index != end && !containsModifiedCommands; ++index)
    {
        const Command::Pointer& command = commands.at(index);
        containsModifiedCommands |= command->IsModifying();
    }
    SetClean(!containsModifiedCommands);
}

void CommandStack::SetCurrentIndex(int32 currentIndex_)
{
    currentIndex = currentIndex_;
    UpdateCleanState();
    SetCanUndo(CanUndoImpl());
    SetCanRedo(CanRedoImpl());
    currentIndexChanged.Emit(currentIndex);
}

void CommandStack::SetClean(bool isClean_)
{
    if (isClean != isClean_)
    {
        isClean = isClean_;
        cleanChanged.Emit(isClean);
    }
}

void CommandStack::SetCanUndo(bool canUndo_)
{
    if (canUndo != canUndo_)
    {
        canUndo = canUndo_;
        canUndoChanged.Emit(canUndo);
    }
}

void CommandStack::SetCanRedo(bool canRedo_)
{
    if (canRedo != canRedo_)
    {
        canRedo = canRedo_;
        canRedoChanged.Emit(canRedo);
    }
}
}

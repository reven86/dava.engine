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
    if (rootBatch != nullptr)
    {
        //when the macro started command without undo will cause undefined state of application
        DVASSERT_MSG(command->CanUndo(),
                     Format("Command %s, which can not make undo passed to CommandStack within macro %s",
                            command->GetDescription().c_str(),
                            batchesStack.top()->GetDescription().c_str())
                     .c_str()
                     );
        batchesStack.top()->AddAndRedo(std::move(command));
    }
    else
    {
        commands.push_back(std::move(command));
    }
}

void CommandStack::BeginBatch(const String& name, uint32 commandsCount)
{
    DAVA::Command::Pointer newCommandBatch = DAVA::Command::Create<DAVA::CommandBatch>(name, commandsCount);
    DAVA::CommandBatch* newCommandBatchPtr = static_cast<DAVA::CommandBatch*>(newCommandBatch.get());
    //we call BeginMacro first time
    if (rootBatch == nullptr)
    {
        DVASSERT(batchesStack.empty());
        rootBatch = std::move(newCommandBatch);
        batchesStack.push(newCommandBatchPtr);
    }
    //we already create one or more batches
    else
    {
        batchesStack.top()->AddAndExec(std::move(newCommandBatch));
        batchesStack.push(newCommandBatchPtr);
    }
}

void CommandStack::EndBatch()
{
    DVASSERT(rootBatch != nullptr && "CommandStack::EndMacro called without BeginMacro");
    if (batchesStack.size() == 1)
    {
        DVASSERT(rootBatch != nullptr);
        DAVA::CommandBatch* rootBatchPtr = static_cast<DAVA::CommandBatch*>(rootBatch.get());
        if (!rootBatchPtr->IsEmpty())
        {
            commands.push_back(std::move(rootBatch));
        }
    }
    if (!batchesStack.empty())
    {
        batchesStack.pop();
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
    DVASSERT(currentIndex < commands.size());
    if (currentIndex >= 0)
    {
        return commands.at(currentIndex)->CanUndo();
    }
    return false;
}

bool CommandStack::CanRedo() const
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
    SetCanUndo(CanUndo());
    SetCanRedo(CanRedo());
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
